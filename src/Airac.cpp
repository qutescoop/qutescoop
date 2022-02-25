/**************************************************************************
 This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Airac.h"

#include "FileReader.h"
#include "Waypoint.h"
#include "NavData.h"
#include "Settings.h"
#include "GuiMessage.h"

Airac *airacInstance = 0;
Airac *Airac::instance(bool createIfNoInstance) {
    if(airacInstance == 0)
        if (createIfNoInstance)
            airacInstance = new Airac();
    return airacInstance;
}

Airac::Airac() {
}

Airac::~Airac() {
    foreach (const QSet<Waypoint*> &wl, waypoints.values())
        foreach(Waypoint *w, wl)
            delete w;
    foreach (const QSet<NavAid*> &nl, navaids.values())
        foreach(NavAid *n , nl)
            delete n;
    foreach (const QList<Airway*> &al, airways.values())
        foreach(Airway *a , al)
            delete a;
}

void Airac::load() {
    qDebug() << "Airac::load()" << Settings::navdataDirectory();
    GuiMessages::status("Loading navigation database...", "airacload");
    if (Settings::useNavdata()) {
        readWaypoints(Settings::navdataDirectory());
        readNavaids(Settings::navdataDirectory());
        readAirways(Settings::navdataDirectory());
    }

    allPoints.clear();
    allPoints.reserve(waypoints.size() + navaids.size());
    foreach (const QSet<Waypoint*> &wl, waypoints.values())
        foreach(Waypoint *w, wl)
            allPoints.insert(w);
    foreach (const QSet<NavAid*> &nl, navaids.values())
        foreach(NavAid *n , nl)
            allPoints.insert(n);

    GuiMessages::remove("airacload");
    emit loaded();
    qDebug() << "Airac::load() -- finished";
}

void Airac::readWaypoints(const QString& directory) {
    waypoints.clear();
    FileReader fr(directory + "/earth_fix.dat");
    fr.skipLines(3);
    while(!fr.atEnd()) {
        QString line = fr.nextLine().trimmed();
        if(line.isEmpty() || line.startsWith("99")) //99 denotes EOF
            continue;

        Waypoint *wp = new Waypoint(line.split(' ', Qt::SkipEmptyParts));
        if (wp == 0 || wp->isNull())
            continue;

        waypoints[wp->label].insert(wp);
    }
    qDebug() << "Read fixes from\t" << (directory + "/earth_fix.dat")
             << "-" << waypoints.size() << "imported";
}

void Airac::readNavaids(const QString& directory) {
    navaids.clear();
    FileReader fr(directory + "/earth_nav.dat");
    fr.skipLines(3);
    while(!fr.atEnd()) {
        QString line = fr.nextLine().trimmed();
        if(line.isEmpty() || line.startsWith("99"))  //99 denotes EOF
            continue;

        NavAid *nav = new NavAid(line.split(' ', Qt::SkipEmptyParts));
        if (nav == 0 || nav->isNull())
            continue;

        navaids[nav->label].insert(nav);
    }
    qDebug() << "Read navaids from\t" << (directory + "/earth_nav.dat")
            << "-" << navaids.size() << "imported";
}

void Airac::readAirways(const QString& directory) {
    bool ok;
    int segments = 0;

    FileReader fr(directory + "/earth_awy.dat");
    fr.skipLines(3);
    while(!fr.atEnd()) {
        QString line = fr.nextLine().trimmed();
        if(line.isEmpty() || line.startsWith("99"))  //99 denotes EOF
            continue;

        // ABDOR GM 11 VALBA GM 11 N 2 195 460 UZ801
        // https://developer.x-plane.com/article/navdata-in-x-plane-11/

        QStringList list = line.split(' ', Qt::SkipEmptyParts);
        if(list.size() < 10 || list.size() > 20)
            continue;

        QString id = list[0];
        QString regionCode = list[1];
        int fixType = list[2].toInt(&ok);
        if(!ok) {
            qWarning() << "Airac::readAirways() unable to parse fix type (int):" << list;
            continue;
        }
        Waypoint *start = waypoint(id, regionCode, fixType);
        if(start == 0){
            qWarning() << "Airac::readAirways() unable to find start waypoint:" << list;
            continue;
        }

        id = list[3];
        regionCode = list[4];
        fixType = list[5].toInt(&ok);
        if(!ok) {
            qWarning() << "Airac::readAirways() unable to parse fix type (int):" << list;
            continue;
        }
        Waypoint *end = waypoint(id, regionCode, fixType);
        if(end == 0){
            qWarning() << "Airac::readAirways() unable to find end waypoint:" << list;
            continue;
        }


        Airway::Type type = (Airway::Type)list[7].toInt(&ok);
        if(!ok) {
            qWarning() << "Airac::readAirways() unable to parse airwaytype (int):" << list;
            continue;
        }
        int base = list[8].toInt(&ok);
        if(!ok) {
            qWarning() << "Airac::readAirways() unable to parse base (int):" << list;
            continue;
        }
        int top = list[9].toInt(&ok);
        if(!ok) {
            qWarning() << "Airac::readAirways() unable to parse top (int):" << list;
            continue;
        }

        QStringList names;
        if(list.size() > 11) {
            //handle airways with spaces (!) in the name
            QString glue;
            for(int i = 10; i < list.size(); i++) {
                if(i > 10)
                    glue += " ";
                glue += list[i];
            }
            names = glue.split('-', Qt::SkipEmptyParts);
        } else
            names = list[10].split('-', Qt::SkipEmptyParts);

        for(int i = 0; i < names.size(); i++) {
            addAirwaySegment(start, end, type, base, top, names[i]);
            segments++;
        }
    }

    QHash<QString, QList<Airway*> >::iterator iter;
    for(iter = airways.begin(); iter != airways.end(); ++iter) {
        QList<Airway*>& list = iter.value();
        const QList<Airway*> sorted = list[0]->sort();
        delete list[0];
        list = sorted;
    }

    qDebug() << "Read airways from\t" << (directory + "/earth_awy.dat")
            << "-" << airways.size() << "airways," << segments << "segments imported and sorted";
}

Waypoint* Airac::waypoint(const QString &id, const QString &regionCode, const int &type) const{
    Waypoint *result = 0;
    if (type == 11){
        foreach(Waypoint *w, waypoints[id]){
            if(w->regionCode == regionCode)
               return w;
        }
    } else {
        foreach(NavAid *n, navaids[id]){
            if (n->regionCode == regionCode)
                return n;
        }
    }
    return result;
}

/**
  find a waypoint that is near the given location @param lat, @param lon with
  the given maximum distance @maxDist.
  @returns 0 if none found
**/
Waypoint* Airac::waypoint(const QString& id, double lat, double lon, double maxDist) const {
    Waypoint *result = 0;
    double minDist = 99999;

    foreach (NavAid *n, navaids[id]) {
        double d = NavData::distance(lat, lon, n->lat, n->lon);
        if ((d < minDist) && (d < maxDist)) {
            result = n;
            minDist = d;
        }
    }
    foreach (Waypoint *w, waypoints[id]) {
        double d = NavData::distance(lat, lon, w->lat, w->lon);
        if ((d < minDist) && (d < maxDist)) {
            result = w;
            minDist = d;
        }
    }
    if (NavData::instance()->airports.contains(id)) { // trying aerodromes
        double d = NavData::distance(lat, lon,
                                     NavData::instance()->airports[id]->lat,
                                     NavData::instance()->airports[id]->lon);
        if ((d < minDist) && (d < maxDist)) {
            result = new Waypoint(id, NavData::instance()->airports[id]->lat,
                                  NavData::instance()->airports[id]->lon);
            minDist = d;
        }
    }

    if (result == 0) { // trying generic formats
        QRegExp slash("([\\-]?\\d{2})/([\\-]?\\d{2,3})"); // some pilots
                                                // ..like to use non-standard: -53/170
        QRegExp wildGuess("([NS]?)(\\d{2})([NSEW])(\\d{2,3})([EW]?)"); // or even
                                                                        // .. N53W170
        QRegExp idiotFormat("(\\d{2,4})([NS])(\\d{2,5})([EW])"); // complete ignorants
                                                             // ..write: 3000N 02000W
        const QPair<double, double> *arincP = NavData::fromArinc(id);
        if (arincP != 0) { // ARINC424
            double d = NavData::distance(lat, lon, arincP->first, arincP->second);
            if ((d < minDist) && (d < maxDist)) {
                result = new Waypoint(id, arincP->first, arincP->second);
                minDist = d;
            }
        } else if (slash.exactMatch(id)) { // slash-style: 35/30
            double wLat = slash.capturedTexts()[1].toDouble();
            double wLon = slash.capturedTexts()[2].toDouble();
            double d = NavData::distance(lat, lon, wLat, wLon);
            if ((d < minDist) && (d < maxDist)) {
                result = new Waypoint(NavData::toArinc(wLat, wLon), wLat, wLon);
                minDist = d;
            }
        } else if (wildGuess.exactMatch(id)) { // N53W170
            double wLat = wildGuess.capturedTexts()[2].toDouble();
            double wLon = wildGuess.capturedTexts()[4].toDouble();
            if ((wildGuess.capturedTexts()[1] == "S") ||
                (wildGuess.capturedTexts()[3] == "S"))
                wLat = -wLat;
            if ((wildGuess.capturedTexts()[3] == "W") ||
                (wildGuess.capturedTexts()[5] == "W"))
                wLon = -wLon;
            double d = NavData::distance(lat, lon, wLat, wLon);
            if ((d < minDist) && (d < maxDist)) {
                result = new Waypoint(NavData::toArinc(wLat, wLon), wLat, wLon);
                minDist = d;
            }
        } else if (idiotFormat.exactMatch(id)) { // 3000N 02000W (space removed by resolveFlightplan)
            double wLat = idiotFormat.capturedTexts()[1].toDouble();
            double wLon = idiotFormat.capturedTexts()[3].toDouble();
            while (wLat > 90)
                wLat /= 10.;
            while (wLon > 180)
                wLon /= 10.;
            if (idiotFormat.capturedTexts()[2] == "S")
                wLat = -wLat;
            if (idiotFormat.capturedTexts()[4] == "W")
                wLon = -wLon;
            double d = NavData::distance(lat, lon, wLat, wLon);
            if ((d < minDist) && (d < maxDist)) {
                result = new Waypoint(NavData::toArinc(wLat, wLon), wLat, wLon);
                minDist = d;
            }
        }
    }
    return result;
}

Airway* Airac::airway(const QString& name, Airway::Type type, int base, int top) {
    foreach(Airway *a, airways[name])
        if(a->type == type)
            return a;
    Airway* awy = new Airway(name, type, base, top);
    airways[name].append(awy);
    return awy;
}

Airway* Airac::airway(const QString& name, double lat, double lon) const {
    const QList<Airway*> list = airways[name];
    if(list.isEmpty())
        return 0;
    if(list.size() == 1)
        return list[0];

    double minDist = 9999;
    Airway* result = 0;
    foreach(Airway *aw, list) {
        Waypoint* wp = aw->closestPointTo(lat, lon);
        if(wp == 0)
            continue;
        double d = NavData::distance(lat, lon, wp->lat, wp->lon);
        if(qFuzzyIsNull(d))
            return aw;
        if(d < minDist) {
            minDist = d;
            result = aw;
        }
    }
    return result;
}

void Airac::addAirwaySegment(Waypoint *from, Waypoint *to, Airway::Type type, int base, int top, const QString& name) {
    Airway *awy = airway(name, type, base, top);
    awy->addSegment(from, to);
}

/**
* Returns a list of waypoints for the given planned route, starting at lat/lon.
* Airways along the route will be replaced by the appropriate fixes along that
* airway. lat/lon is being used as a hint and should be the position of the
* departure airport.
*
* Input format can be:
* 1. FIX - FIX - FIX
* 2. FIX - Airway - FIX - Airway - FIX
*
* Unknown fixes and/or airways will be ignored.
**/
QList<Waypoint*> Airac::resolveFlightplan(QStringList plan, double lat, double lon) const {
    //qDebug() << "Airac::resolveFlightPlan()" << plan;
    QList<Waypoint*> result;
    if(plan.isEmpty()) return result;
    Waypoint* currPoint = 0;
    bool wantAirway = false;
    while (!plan.isEmpty()) {
        QString id = plan.takeFirst();
        Airway *awy = 0;
        if (wantAirway)
            awy = airway(id, lat, lon);
        if (awy != 0 && !plan.isEmpty()) {
            wantAirway = false;
            // have airway - next should be a waypoint
            QString endId = plan.first();
            Waypoint* wp = waypoint(endId, lat, lon);
            if(wp != 0) {
                if (currPoint != 0)
                    result += awy->expand(currPoint->label, wp->label);
                currPoint = wp;
                lat = wp->lat;
                lon = wp->lon;
                plan.removeFirst();
                wantAirway = true;
                continue;
            }
        } else if (awy == 0) {
            if (!plan.isEmpty()) // joining with the next point for idiot style..
                if (QRegExp("\\d{2,4}[NS]").exactMatch(id) && //.. 30(00)N (0)50(00)W
                    QRegExp("(\\d{2,3}|\\d{5})[EW]").exactMatch(plan.first()))
                                    // but preserving correct ARINC style (\\d{4}[EW])
                    id += plan.takeFirst();
            Waypoint* wp = waypoint(id, lat, lon);
            if(wp != 0) {
                result.append(wp);
                currPoint = wp;
                lat = wp->lat;
                lon = wp->lon;
            }
            wantAirway = (wp != 0);
        }
    }
//    QString debugStr;
//    foreach (Waypoint *p, result)
//        debugStr += p->label + " ";
//    qDebug() << "Airac::resolveFlightPlan() -- finished:" << debugStr;
    return result;
}
