/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#include <math.h>
#include <QDebug>

#include "NavData.h"
#include "FileReader.h"
#include "FirReader.h"
#include "helpers.h"
#include "Settings.h"

NavData *instance = 0;

NavData::NavData() {
    loadAirports(Settings::dataDirectory() + "airports.dat");
    loadFirs();
    loadCountryCodes(Settings::dataDirectory() + "countrycodes.dat");
    loadDatabase(Settings::navdataDirectory());
}

void NavData::loadAirports(const QString& filename) {
    airportMap.clear();
    airportsListTrafficSorted.clear();
    FileReader fileReader(filename);
    while (!fileReader.atEnd()) {
        QString line = fileReader.nextLine();
        if (line.isNull())
            return;
        Airport *airport = new Airport(line.split(':'));
        if (airport != 0 && !airport->isNull())
            airportMap[airport->label] = airport;
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

void NavData::loadFirs() {
    FirReader firReader;
    firReader.loadFirs(firMap);
}

NavData* NavData::getInstance() {
    if (instance == 0) {
        instance = new NavData();
    }
    return instance;
}

const QHash<QString, Airport*>& NavData::airports() const {
    return airportMap;
}

const QList<Airport*>& NavData::airportsTrafficSorted() const {
    return airportsListTrafficSorted;
}

const QHash<QString, Fir*>& NavData::firs() const {
    return firMap;
}

#define IS_NAN(x) (x != x)

double NavData::distance(double lat1, double lon1, double lat2, double lon2) {
    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;

    double result = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1-lon2));
    if(IS_NAN(result))
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
    QList<Airport*> airports = airportMap.values();
    for(int i = 0; i < airports.size(); i++) {
        if(distance(airports[i]->lat, airports[i]->lon, lat, lon) <= maxDist) {
            result.append(airports[i]);
        }
    }

    return result;
}

void NavData::updateData(const WhazzupData& whazzupData) {
    QList<Airport*> airportList = airportMap.values();
    for(int i = 0; i < airportList.size(); i++) {
        if(airportList[i] != 0)
            airportList[i]->resetWhazzupStatus();
    }

    QList<Pilot*> allpilots = whazzupData.getAllPilots();
    for(int i = 0; i < allpilots.size(); i++) {
        Pilot *p = allpilots[i];
        if(p == 0) continue;
        if(airportMap.contains(p->planDep) && airportMap[p->planDep] != 0)
            airportMap[p->planDep]->addDeparture(p);
        else if(p->flightStatus() == Pilot::BUSH) { // no flightplan yet?
            QList<Airport*> aa = airportsAt(p->lat, p->lon, 3);
            if(aa.size() != 0) {
                if(aa[0] != 0) {
                    airportMap[aa[0]->label]->addDeparture(p);
                }
            }
        }

        if(airportMap.contains(p->planDest) && airportMap[p->planDest] != 0)
            airportMap[p->planDest]->addArrival(p);
    }

    for(int i = 0; i < whazzupData.getControllers().size(); i++) {
        Controller *c = dynamic_cast<Controller*>(whazzupData.getControllers()[i]);

        QString icao = c->getApproach();
        if(!icao.isNull() && airportMap.contains(icao) && airportMap[icao] != 0) {
            airportMap[icao]->addApproach(c);
        }

        icao = c->getTower();
        if(!icao.isNull() && airportMap.contains(icao) && airportMap[icao] != 0) {
            airportMap[icao]->addTower(c);
        }

        icao = c->getGround();
        if(!icao.isNull() && airportMap.contains(icao) && airportMap[icao] != 0) {
            airportMap[icao]->addGround(c);
        }

        if(c->label.right(4) == "_FSS" || c->label.right(4) == "_CTR") {
            // calculate covered airports (by a circle around the geometrical centre of the FIR)
            double coverLat, coverLon;
            int coverRange;
            if (c->fir != 0) {
                qDebug() << "checking equdistant Point for FIR" << c->label; //fixme
                QPair<double, double> center = c->fir->equidistantPoint();
                coverLat = center.first;
                coverLon = center.second;
                coverRange = c->fir->maxDistanceFromCenter();
            } else {
                coverLat = c->lat;
                coverLon = c->lon;
                coverRange = c->visualRange;
            }

            QList<Airport*> aps = NavData::getInstance()->airportsAt(coverLat, coverLon, coverRange);
            qDebug() << QString("%1 at %2/%3 (%4 NM) covering %5 airports")
                    .arg(c->label)
                    .arg(coverLat)
                    .arg(coverLon)
                    .arg(coverRange)
                    .arg(aps.size());
            for(int j = 0; j < aps.size(); j++) {
                aps[j]->addCenter(c);
            }
        }
    }

    airportsListTrafficSorted.clear(); // we gonna fill it again here
    for(int i = 0; i < airportList.size(); i++) {
        if(airportList[i] != 0)
            airportList[i]->refreshAfterUpdate();

        // fill them in the airportsTrafficSorted List - sorted descending
        int congestion = airportList[i]->numFilteredArrivals() + airportList[i]->numFilteredDepartures(); // sort key
        airportsListTrafficSorted.append(airportList[i]);
        if(congestion > 0) {
            for(int h=0; h < airportsListTrafficSorted.size() - 1; h++) { // find the point to insert
                if (airportsListTrafficSorted[h]->numFilteredArrivals()
                    + airportsListTrafficSorted[h]->numFilteredDepartures() < congestion) { // move inserted item before that one
                        airportsListTrafficSorted.move(airportsListTrafficSorted.size() - 1, h);
                        break;
                }
            }
        }
    }
}

void NavData::accept(MapObjectVisitor* visitor) {
    QList<Airport*> airports = airportMap.values();
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
