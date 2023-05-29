/**************************************************************************
 This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Airac.h"

#include "Airport.h"
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
    foreach (const QSet<Waypoint*> &wl, fixes.values())
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
        readFixes(Settings::navdataDirectory());
        readNavaids(Settings::navdataDirectory());
        readAirways(Settings::navdataDirectory());
    }

    allPoints.clear();
    allPoints.reserve(fixes.size() + navaids.size());
    foreach (const QSet<Waypoint*> &wl, fixes.values())
        foreach(Waypoint *w, wl)
            allPoints.insert(w);
    foreach (const QSet<NavAid*> &nl, navaids.values())
        foreach(NavAid *n , nl)
            allPoints.insert(n);

    GuiMessages::remove("airacload");
    emit loaded();
    qDebug() << "Airac::load() -- finished";
}

void Airac::readFixes(const QString& directory) {
    fixes.clear();

    const QString file(directory + "/earth_fix.dat");
    FileReader fr(file);

    // 1st line: just an "I"
    fr.nextLine();
    // 2nd line: navdata format version, build information and data source
    const QString version = fr.nextLine();
    if (version.left(2) != "11") {
        qCritical() << file << "is not in X-Plane version 11 data format";
    }

    while(!fr.atEnd()) {
        // file format:
        //  49.862241667    9.348325000  SPESA ENRT ED 4530243
        // https://developer.x-plane.com/article/navdata-in-x-plane-11/

        QString line = fr.nextLine().trimmed();
        if(line.isEmpty())
            continue;

        // 99 denotes EOF
        if(line == "99")
            break;

        Waypoint *wp = new Waypoint(line.split(' ', Qt::SkipEmptyParts));
        if (wp == 0 || wp->isNull())
            continue;

        fixes[wp->label].insert(wp);
    }
    qDebug() << "Read fixes from" << (directory + "/earth_fix.dat")
             << "-" << fixes.size() << "imported";
}

void Airac::readNavaids(const QString& directory) {
    navaids.clear();

    const QString file(directory + "/earth_nav.dat");
    FileReader fr(file);

    // 1st line: just an "I"
    fr.nextLine();
    // 2nd line: navdata format version, build information and data source
    const QString version = fr.nextLine();
    if (version.left(2) != "11") {
        qCritical() << file << "is not in X-Plane version 11 data format";
    }

    while(!fr.atEnd()) {
        // file format:
        //  3  52.721000000   -8.885222222      200    11330   130     -4.000  SHA ENRT EI SHANNON VOR/DME
        // https://developer.x-plane.com/article/navdata-in-x-plane-11/

        QString line = fr.nextLine().trimmed();
        if(line.isEmpty())
            continue;

        // 99 denotes EOF
        if(line == "99")
            break;

        NavAid *nav = new NavAid(line.split(' ', Qt::SkipEmptyParts));
        if (nav == 0 || nav->isNull())
            continue;

        navaids[nav->label].insert(nav);
    }
    qDebug() << "Read navaids from" << (directory + "/earth_nav.dat")
            << "-" << navaids.size() << "imported";
}

void Airac::readAirways(const QString& directory) {
    // @todo: bring this in line with the other navdata source -> object converters

    airways.clear();

    const QString file(directory + "/earth_awy.dat");
    FileReader fr(file);

    // 1st line: just an "I"
    fr.nextLine();
    // 2nd line: navdata format version, build information and data source
    const QString version = fr.nextLine();
    if (version.left(2) != "11") {
        qCritical() << file << "is not in X-Plane version 11 data format";
    }

    bool ok;
    int segments = 0;
    while(!fr.atEnd()) {
        QString line = fr.nextLine().trimmed();
        // file format:
        // EXOLU VA 11 TAXUN VA 11 N 2  75 460 B342-N519-W14
        // https://developer.x-plane.com/article/navdata-in-x-plane-11/

        if(line.isEmpty())
            continue;

        // 99 denotes EOF
        if(line == "99")
            break;

        QStringList list = line.split(' ', Qt::SkipEmptyParts);
        if(list.size() != 11) {
            qCritical() << "Airac::readAirways() not exactly 11 fields:" << list;
            continue;
        }

        QString id = list[0];
        QString regionCode = list[1];
        int fixType = list[2].toInt(&ok);
        if(!ok) {
            qCritical() << "Airac::readAirways() unable to parse fix type (int):" << list;
            continue;
        }
        Waypoint *start = waypoint(id, regionCode, fixType);
        if(start == 0){
            qCritical() << "Airac::readAirways() unable to find start waypoint:" << list;
            continue;
        }

        id = list[3];
        regionCode = list[4];
        fixType = list[5].toInt(&ok);
        if(!ok) {
            qCritical() << "Airac::readAirways() unable to parse fix type (int):" << list;
            continue;
        }
        Waypoint *end = waypoint(id, regionCode, fixType);
        if(end == 0){
            qCritical() << "Airac::readAirways() unable to find end waypoint:" << list;
            continue;
        }

        QStringList names;
        names = list[10].split('-', Qt::SkipEmptyParts);
        for(int i = 0; i < names.size(); i++) {
            addAirwaySegment(start, end, names[i]);
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

    qDebug() << "Read airways from" << (directory + "/earth_awy.dat")
            << "-" << airways.size() << "airways," << segments << "segments imported and sorted";
}

Waypoint* Airac::waypoint(const QString &id, const QString &regionCode, const int &type) const{
    Waypoint *result = 0;
    if (type == 11){
        foreach(Waypoint *w, fixes[id]){
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
  find a waypoint that is near the given location with the given maximum distance.
  @returns 0 if none found
**/
Waypoint* Airac::waypointNearby(const QString& input, double lat, double lon, double maxDist = 20000.) {
    // @todo clean this up
    // @todo add "virtual" fixes (ARINC424) to our nav database upfront instead of returning them
    // here dynamically (without adding them), which leads to duplicates

    Waypoint *result = 0;
    double minDist = 99999;

    foreach (NavAid *n, navaids[input]) {
        double d = NavData::distance(lat, lon, n->lat, n->lon);
        if ((d < minDist) && (d < maxDist)) {
            result = n;
            minDist = d;
        }
    }
    foreach (Waypoint *w, fixes[input]) {
        double d = NavData::distance(lat, lon, w->lat, w->lon);
        if ((d < minDist) && (d < maxDist)) {
            result = w;
            minDist = d;
        }
    }
    if (NavData::instance()->airports.contains(input)) { // trying aerodromes
        double d = NavData::distance(lat, lon,
                                     NavData::instance()->airports[input]->lat,
                                     NavData::instance()->airports[input]->lon);
        if ((d < minDist) && (d < maxDist)) {
            result = new Waypoint(input, NavData::instance()->airports[input]->lat,
                                  NavData::instance()->airports[input]->lon);
            minDist = d;
        }
    }

    if (result == 0) { // trying generic formats
        QRegExp eurocontrol("(\\d{2})"
                            "((\\d{2})?)"
                            "((\\d{2})?)"
                            "([NS])"
                            "(\\d{2,3})"
                            "((\\d{2})?)"
                            "((\\d{2})?)"
                            "([EW])"); // things that are valid for the Eurocontrol route validator:
                    // 63N005W or 6330N00530W (minutes) or 633000N0053000W (minutes and seconds)
                    // we are not strict and also allow 2-char longitudes like 63N05W
        QRegExp slash("([\\-]?\\d{2})/([\\-]?\\d{2,3})"); // some pilots
                                                // ..like to use non-standard: -53/170

        double foundLat = -180., foundLon = -360.;

        const QPair<double, double> *arincP = NavData::fromArinc(input);
        if (arincP != 0) { // ARINC424
            double d = NavData::distance(lat, lon, arincP->first, arincP->second);
            if ((d < minDist) && (d < maxDist)) {
                foundLat = arincP->first;
                foundLon = arincP->second;
            }
            delete arincP;
        } else if (eurocontrol.exactMatch(input)) {
            auto capturedTexts = eurocontrol.capturedTexts();

            double wLat = capturedTexts[1].toDouble() + capturedTexts[2].toDouble() / 60. + capturedTexts[4].toDouble() / 3600.;
            double wLon = capturedTexts[7].toDouble() + capturedTexts[8].toDouble() / 60. + capturedTexts[10].toDouble() / 3600.;
            if (capturedTexts[6] == "S") {
                wLat = -wLat;
            }
            if (capturedTexts[12] == "W") {
                wLon = -wLon;
            }
            double d = NavData::distance(lat, lon, wLat, wLon);
            if ((d < minDist) && (d < maxDist)) {
                foundLat = wLat;
                foundLon = wLon;
            }
        } else if (slash.exactMatch(input)) { // slash-style: 35/30
            auto capturedTexts = slash.capturedTexts();
            double wLat = capturedTexts[1].toDouble();
            double wLon = capturedTexts[2].toDouble();
            double d = NavData::distance(lat, lon, wLat, wLon);
            if ((d < minDist) && (d < maxDist)) {
                foundLat = wLat;
                foundLon = wLon;
            }
        }

        if (foundLat > -180.) {
            // try ARINC because some northern Atlantic waypoints are already in the AIRAC
            QString foundId = NavData::toArinc(foundLat, foundLon);
            if (foundId == "") {
                foundId = NavData::toEurocontrol(foundLat, foundLon);
            }

            if (fixes.contains(foundId)) {
                auto founds = fixes.value(foundId).values();
                if (founds.size() > 0) {
                    return founds.at(0);
                }
            }

            result = new Waypoint(foundId, foundLat, foundLon);
            // we add it to our database
            // @todo consider if this is a good idea here
            fixes[foundId].insert(result);
        }
    }

    return result;
}

Airway* Airac::airway(const QString& name) {
    foreach(Airway *a, airways[name])
        return a;

    Airway* awy = new Airway(name);
    airways[name].append(awy);
    return awy;
}

Airway* Airac::airwayNearby(const QString& name, double lat, double lon) const {
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

void Airac::addAirwaySegment(Waypoint *from, Waypoint *to, const QString& name) {
    Airway *awy = airway(name);
    awy->addSegment(from, to);
}

/**
* Returns a list of waypoints for the given planned route, starting at lat/lon.
* Airways along the route will be replaced by the appropriate fixes along that
* airway. lat/lon is being used as a hint and should be the position of the
* departure airport.
*
* Unknown fixes and/or airways will be ignored.
**/
QList<Waypoint*> Airac::resolveFlightplan(QStringList plan, double lat, double lon) {
    //qDebug() << "Airac::resolveFlightPlan()" << plan;
    QList<Waypoint*> result;
    Waypoint* currPoint = 0;
    Airway* awy = 0;
    bool wantAirway = false;
    while (!plan.isEmpty()) {
        QString id = fpTokenToWaypoint(plan.takeFirst());

        if (wantAirway) {
            awy = airwayNearby(id, lat, lon);
        }
        if (wantAirway && (awy != 0) && !plan.isEmpty()) {
            wantAirway = false;
            // have airway - next should be a waypoint
            QString endId = fpTokenToWaypoint(plan.first());
            // 5500NM is the longest legitimate route part (PACOT entry-exit) that we can't resolve
            Waypoint* wp = waypointNearby(endId, lat, lon, 5500.);
            if(wp != 0) {
                if (currPoint != 0) {
                    auto _expand = awy->expand(currPoint->label, wp->label);
                    result.append(_expand);
                }
                currPoint = wp;
                lat = wp->lat;
                lon = wp->lon;
                plan.removeFirst();
                wantAirway = true;
            }
        } else if (awy == 0) {
            if (!plan.isEmpty()) {// joining with the next point for idiot style..
                if (
                    QRegExp("\\d{2,4}[NS]").exactMatch(id) //.. 30(00)N (0)50(00)W
                    && QRegExp("(\\d{2,3}|\\d{5})[EW]").exactMatch(plan.first())
                          // but preserving correct ARINC style (\\d{4}[EW])
                ) {

                    id += fpTokenToWaypoint(plan.takeFirst());
                }
            }

            Waypoint* wp = waypointNearby(id, lat, lon, 5500.);
            if(wp != 0) {
                result.append(wp);
                currPoint = wp;
                lat = wp->lat;
                lon = wp->lon;
            }
            wantAirway = (wp != 0);
        }
    }

    return result;
}

QString Airac::fpTokenToWaypoint(QString token) const
{
  // remove everything following an invalid character (e.g. "/N320F240", "/S1130K400")
  return token.replace(QRegExp("[^A-Za-z0-9].*$"), "");
}
