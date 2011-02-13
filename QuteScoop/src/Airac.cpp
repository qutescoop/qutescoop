/**************************************************************************
 This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Airac.h"

#include "FileReader.h"
#include "Waypoint.h"
#include "NavData.h"

Airac::Airac() {
    // TODO Auto-generated constructor stub

}

Airac::~Airac() {
    // TODO Auto-generated destructor stub
}

void Airac::load(const QString& directory) {
    readFixes(directory);
    readNavaids(directory);
    readAirways(directory);

    QHash<QString, QList<Waypoint*> >::const_iterator iter = waypointMap.begin();
    while(iter != waypointMap.end()) {
        allWaypoints += iter.value();
        ++iter;
    }
}

void Airac::addFix(Waypoint* fix) {
    QList<Waypoint*>& list = waypointMap[fix->label];
    list.append(fix);
}

void Airac::readFixes(const QString& directory) {
    waypointMap.clear();
    FileReader fr(directory + "/default data/earth_fix.dat");
    while(!fr.atEnd()) {
        QString line = fr.nextLine().trimmed();
        if(line.isEmpty())
            continue;

        Waypoint *wp = new Waypoint(line.split(' ', QString::SkipEmptyParts));
        if (wp == 0 || wp->isNull())
            continue;

        addFix(wp);
    }
    qDebug() << "Read fixes from\t" << (directory + "/default data/earth_fix.dat") << "-" << waypointMap.size() << "imported";
}

void Airac::readNavaids(const QString& directory) {
    navaidMap.clear();
    FileReader fr(directory + "/default data/earth_nav.dat");
    while(!fr.atEnd()) {
        QString line = fr.nextLine().trimmed();
        if(line.isEmpty())
            continue;

        NavAid *nav = new NavAid(line.split(' ', QString::SkipEmptyParts));
        if (nav == 0 || nav->isNull())
            continue;

        QList<NavAid*> list = navaidMap[nav->label];
        list.append(nav);
        navaidMap[nav->label] = list;
    }
    qDebug() << "Read navaids from\t" << (directory + "/default data/earth_nav.dat") << "-" << navaidMap.size() << "imported";
}

void Airac::readAirways(const QString& directory) {
    bool ok;
    int segments = 0;

    FileReader fr(directory + "/default data/earth_awy.dat");
    while(!fr.atEnd()) {
        QString line = fr.nextLine().trimmed();
        if(line.isEmpty())
            continue;

        QStringList list = line.split(' ', QString::SkipEmptyParts);
        if(list.size() < 10 || list.size() > 20) {
            continue;
        }

        QString id = list[0];
        double lat = list[1].toDouble(&ok);
        if(!ok) continue;
        double lon = list[2].toDouble(&ok);
        if(!ok) continue;

        Waypoint *start = getWaypoint(id, lat, lon, 1);
        if(start == 0) {
            start = new Waypoint(id, lat, lon);
            addFix(start);
        }

        id = list[3];
        lat = list[4].toDouble(&ok);
        if(!ok) continue;
        lon = list[5].toDouble(&ok);
        if(!ok) continue;

        Waypoint *end = getWaypoint(id, lat, lon, 1);
        if(end == 0) {
            end = new Waypoint(id, lat, lon);
            addFix(end);
        }

        Airway::Type type = (Airway::Type)list[6].toInt(&ok);
        if(!ok) continue;

        int base = list[7].toInt(&ok);
        if(!ok) continue;
        int top = list[8].toInt(&ok);
        if(!ok) continue;

        QStringList names;
        if(list.size() > 10) {
            //handle airways with spaces (!) in the name
            QString glue;
            for(int i = 9; i < list.size(); i++) {
                if(i > 9) glue += " ";
                glue += list[i];
            }
            names = glue.split('-', QString::SkipEmptyParts);
        } else {
            names = list[9].split('-', QString::SkipEmptyParts);
        }

        for(int i = 0; i < names.size(); i++) {
            addAirwaySegment(start, end, type, base, top, names[i]);
            segments++;
        }
    }

    QHash<QString, QList<Airway*> >::iterator iter;
    for(iter = airwayMap.begin(); iter != airwayMap.end(); ++iter) {
        QList<Airway*>& list = iter.value();
        QList<Airway*> sorted = list[0]->sort();

        delete list[0];
        list.clear();
        list += sorted;
    }
    qDebug() << "Read airways from\t" << (directory + "/default data/earth_awy.dat") << "-" << airwayMap.size() << "airways," << segments << "segments imported and sorted";
}

Waypoint* Airac::getWaypoint(const QString& id, double lat, double lon, double maxDist) const {
    Waypoint *result = 0;
    double minDist = 99999;

    QList<NavAid*> navs = navaidMap[id];
    for(int i = 0; i < navs.size() && minDist > 0; i++) {
        double d = NavData::distance(lat, lon, navs[i]->lat, navs[i]->lon);
        if(d > maxDist) {
            continue;
        }
        if(d <= minDist) {
            result = navs[i];
            minDist = d;
        }
    }

    QList<Waypoint*> points = waypointMap[id];
    for(int i = 0; i < points.size() && minDist > 0; i++) {
        double d = NavData::distance(lat, lon, points[i]->lat, points[i]->lon);
        if(d > maxDist) {
            continue;
        }
        if(d <= minDist) {
            result = points[i];
            minDist = d;
        }
    }
    return result;
}

Airway* Airac::getAirway(const QString& name, Airway::Type type, int base, int top) {
    QList<Airway*> list = airwayMap[name];
    for(int i = 0; i < list.size(); i++) {
        if(list[i]->type == type) {
            return list[i];
        }
    }
    Airway* awy = new Airway(name, type, base, top);
    list.append(awy);
    airwayMap[name] = list;
    return awy;
}

Airway* Airac::getAirway(const QString& name, double lat, double lon) const {
    QList<Airway*> list = airwayMap[name];
    if(list.isEmpty()) {
        return 0;
    }
    if(list.size() == 1) {
        return list[0];
    }

    double minDist = 9999;
    Airway* result = 0;
    for(int i = 0; i < list.size(); i++) {
        Waypoint* wp = list[i]->getClosestPointTo(lat, lon);
        if(wp == 0) {
            continue;
        }
        double d = NavData::distance(lat, lon, wp->lat, wp->lon);
        if(d == 0) {
            return list[i];
        }
        if(d < minDist) {
            minDist = d;
            result = list[i];
        }
    }
    return result;
}

void Airac::addAirwaySegment(Waypoint* from, Waypoint* to, Airway::Type type, int base, int top, const QString& name) {
    Airway* awy = getAirway(name, type, base, top);
    awy->addSegment(from, to);
}

Waypoint* Airac::getNextWaypoint(QStringList& workingList, double lat, double lon) const {
    Waypoint* result = 0;
    while(!workingList.isEmpty() && result == 0) {
        QString id = workingList.first();
        workingList.removeFirst();
        result = getWaypoint(id, lat, lon);
    }
    return result;
}

QList<Waypoint*> Airac::resolveFlightplan(const QStringList& plan, double lat, double lon) const {
    qDebug() << "Airac::resolveFlightPlan()" << plan;
    QList<Waypoint*> result;
    if(plan.isEmpty()) return result;

    QStringList workingList = plan;

    // find a starting point
    Waypoint* currPoint = getNextWaypoint(workingList, lat, lon);
    if(currPoint == 0) return result;

    result.append(currPoint);
    double myLat = currPoint->lat;
    double myLon = currPoint->lon;
    bool wantAirway = true;

    while(!workingList.isEmpty()) {
        QString id = workingList.first();
        workingList.removeFirst();

        Airway *awy = 0;
        if(wantAirway) awy = getAirway(id, lat, lon);
        if(awy != 0 && !workingList.isEmpty()) {
            wantAirway = false;
            // have airway - next should be a waypoint
            QString endId = workingList.first();
            Waypoint* wp = getWaypoint(endId, myLat, myLon);
            if(wp != 0) {
                result += awy->expand(currPoint->label, wp->label);
                currPoint = wp;
                myLat = wp->lat;
                myLon = wp->lon;
                workingList.removeFirst();
                wantAirway = true;
                continue;
            }
        } else if(awy == 0) {
            wantAirway = false;
            Waypoint* wp = getWaypoint(id, myLat, myLon);
            if(wp != 0) {
                result.append(wp);
                currPoint = wp;
                myLat = wp->lat;
                myLon = wp->lon;
                wantAirway = true;
                continue;
            }
        }
    }

    QString debugStr;
    for(int i = 0; i < result.size(); i++) {
        if(i>0) debugStr += "-";
        debugStr += result[i]->label;
    }
    qDebug() << "Airac::resolveFlightPlan() -- finished:" << debugStr;

    return result;
}
