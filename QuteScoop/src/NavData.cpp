/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "NavData.h"

#include "FileReader.h"
#include "SectorReader.h"
#include "helpers.h"
#include "Settings.h"

NavData *instance = 0;

NavData::NavData() {
    loadAirports(Settings::applicationDataDirectory("data/airports.dat"));
    loadSectors();
    loadCountryCodes(Settings::applicationDataDirectory("data/countrycodes.dat"));
    loadDatabase(Settings::navdataDirectory());
}

void NavData::loadAirports(const QString& filename) {
    airportHash.clear();
    activeAirportsCongestionMap.clear();
    FileReader fileReader(filename);
    while (!fileReader.atEnd()) {
        QString line = fileReader.nextLine();
        if (line.isNull())
            return;
        Airport *airport = new Airport(line.split(':'));
        if (airport != 0 && !airport->isNull())
            airportHash[airport->label] = airport;
    }
}

void NavData::loadCountryCodes(const QString& filename) {
    countryCodes.clear();
    FileReader fileReader(filename);
    while (!fileReader.atEnd()) {
        QString line = fileReader.nextLine();
        if (line.isNull())
            return;
        QStringList list = line.split(':');
        if(!list.size() == 2)
            continue;
        countryCodes[list.first()] = list.last();
    }
}

void NavData::loadSectors() {
    SectorReader sectorReader;
    sectorReader.loadSectors(sectorHash);
}

NavData* NavData::getInstance() {
    if (instance == 0) {
        instance = new NavData();
    }
    return instance;
}

#define IS_NAN(x) (x != x)

double NavData::distance(double lat1, double lon1, double lat2, double lon2) {
    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;

    double result = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1-lon2));
    if(qIsNaN(result))
        return 0;
    return result * 60.0 / Pi180;
}

void NavData::distanceTo(double lat, double lon, double dist, double heading, double *latTo, double *lonTo) {
    if(latTo == 0 || lonTo == 0)
        return;

    lat *= Pi180;
    lon *= Pi180;
    heading = (360 - heading) * Pi180;
    dist = dist / 60.0 * Pi180;

    double rlat = asin(sin(lat) * cos(dist) + cos(lat) * sin(dist) * cos(heading));
    double dlon = atan2(sin(heading) * sin(dist) * cos(lat), cos(dist) - sin(lat) * sin(lat));
    double rlon = fmod(lon - dlon + Pi, 2 * Pi ) - Pi;

    *latTo = rlat / Pi180;
    *lonTo = rlon / Pi180;
}

QList<Airport*> NavData::airportsAt(double lat, double lon, double maxDist) {
    QList<Airport*> result;
    QList<Airport*> airports = airportHash.values();
    for(int i = 0; i < airports.size(); i++) {
        if(distance(airports[i]->lat, airports[i]->lon, lat, lon) <= maxDist) {
            result.append(airports[i]);
        }
    }

    return result;
}

void NavData::updateData(const WhazzupData& whazzupData) {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    qDebug() << "NavData::updateData() on" << airportHash.size() << "airports";
    foreach (Airport *a, activeAirportsCongestionMap.values()) {
        a->resetWhazzupStatus();
    }

    QSet<Airport*> newActiveAirportsSet;
    QList<Pilot*> allpilots = whazzupData.getAllPilots();
    for(int i = 0; i < allpilots.size(); i++) {
        Pilot *p = allpilots[i];
        if(p == 0) continue;

        Airport *dep = airportHash.value(p->planDep, 0);
        if(dep != 0) {
            dep->addDeparture(p);
            newActiveAirportsSet.insert(dep);
            if(Settings::filterTraffic()) { // Airport traffic filtered
                if(p->distanceFromDeparture() < Settings::filterDistance())
                    dep->numFilteredDepartures++;
            }
        }
        else if(p->flightStatus() == Pilot::BUSH) { // no flightplan yet?
            QList<Airport*> aA = airportsAt(p->lat, p->lon, 3);
            if(!aA.isEmpty()) {
                aA.first()->addDeparture(p);
                newActiveAirportsSet.insert(aA.first());
            }
        }

        Airport *dest = airportHash.value(p->planDest, 0);
        if(dest != 0) {
            dest->addArrival(p);
            newActiveAirportsSet.insert(dest);
            if(Settings::filterTraffic()) { // Airport traffic filtered
                if((p->distanceToDestination() < Settings::filterDistance())
                    || (p->distanceToDestination() / p->groundspeed < Settings::filterArriving()))
                    dest->numFilteredArrivals++;
            }
        }
    }

    for(int i = 0; i < whazzupData.getControllers().size(); i++) {
        Controller *c = dynamic_cast<Controller*>(whazzupData.getControllers()[i]);

        QString icao = c->getApproach();
        if(!icao.isNull() && airportHash.contains(icao)) {
            airportHash[icao]->addApproach(c);
            newActiveAirportsSet.insert(airportHash[icao]);
        }

        icao = c->getTower();
        if(!icao.isNull() && airportHash.contains(icao)) {
            airportHash[icao]->addTower(c);
            newActiveAirportsSet.insert(airportHash[icao]);
        }

        icao = c->getGround();
        if(!icao.isNull() && airportHash.contains(icao)) {
            airportHash[icao]->addGround(c);
            newActiveAirportsSet.insert(airportHash[icao]);
        }

        icao = c->getDelivery();
        if(!icao.isNull() && airportHash.contains(icao)) {
            airportHash[icao]->addDelivery(c);
            newActiveAirportsSet.insert(airportHash[icao]);
        }
    }

    // call refreshAfterUpdate() on airports that were, but are not any more, active
    foreach(Airport *a, activeAirportsCongestionMap) {
        if (!newActiveAirportsSet.contains(a)) // not yet the newly active ones
            a->refreshAfterUpdate();
    }

    // new method with MultiMap. Tests show: 450ms vs. 3800ms for 800 pilots :)
    activeAirportsCongestionMap.clear();
    foreach(Airport *a, newActiveAirportsSet) {
        a->refreshAfterUpdate();
        int congestion = a->numFilteredArrivals + a->numFilteredDepartures; // sort key
        activeAirportsCongestionMap.insert(congestion, a);
    }

    qDebug() << "NavData::updateData() -- finished";
    qApp->restoreOverrideCursor();
}

void NavData::accept(MapObjectVisitor* visitor) {
    QList<Airport*> airports = airportHash.values();
    for(int i = 0; i < airports.size(); i++) {
        if(airports[i] != 0)
            visitor->visit(airports[i]);
    }
}

double NavData::courseTo(double lat1, double lon1, double lat2, double lon2) {
    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;

    double d = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1-lon2));

    double tc1;
    if(sin(lon2 - lon1) < 0) {
        tc1 = acos((sin(lat2) - sin(lat1) * cos(d)) / (sin(d) * cos(lat1)));
    } else {
        tc1 = 2 * Pi - acos((sin(lat2) - sin(lat1) * cos(d)) / (sin(d) * cos(lat1)));
    }
    return 360 - (tc1 / Pi180);
}

void NavData::greatCirclePlotTo(double lat1, double lon1,
                                    double lat2, double lon2,
                                    double f,
                                    double *lat, double *lon)
{
    if(lat == 0 || lon == 0) return;

    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;

    double d = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1-lon2));

    double A = sin((1-f) * d) / sin(d);
    double B = sin(f*d) / sin(d);
    double x = A * cos(lat1) * cos(lon1) + B * cos(lat2) * cos(lon2);
    double y = A * cos(lat1) * sin(lon1) + B * cos(lat2) * sin(lon2);
    double z = A * sin(lat1)             + B * sin(lat2);
    double rLat = atan2(z, sqrt(x*x + y*y));
    double rLon = atan2(y, x);

    *lat = rLat / Pi180;
    *lon = rLon / Pi180;
}

void NavData::loadDatabase(const QString& directory) {
    if(directory.isEmpty()) {
        return;
    }

    if(Settings::useNavdata()) {
        airac.load(directory);
    }
}

void NavData::plotPath(double lat1, double lon1, double lat2, double lon2){

    // always start plotting at the origin
    VERTEX(lat1, lon1);

    double d = NavData::distance(lat1, lon1, lat2, lon2);
    if(d < 1) return; // less than 1 mile - not worth plotting

    double fractionIncrement = 30 / d; // one dot every 30nm
    if(fractionIncrement > 1) fractionIncrement = 1;
    double currentFraction = 0;

    double myLat = lat1;
    double myLon = lon1;
    do {
        currentFraction += fractionIncrement;
        if(currentFraction > 1) currentFraction = 1;

        if(currentFraction < 1) {
            // don't plot last dot - we do that in the caller. Avoids plotting vertices twice
            greatCirclePlotTo(lat1, lon1, lat2, lon2, currentFraction, &myLat, &myLon);
            VERTEX(myLat, myLon);
        }

    } while(currentFraction < 1);
}
