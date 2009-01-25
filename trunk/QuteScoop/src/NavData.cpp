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

	for(int i = 0; i < whazzupData.getPilots().size(); i++) {
		Pilot *p = dynamic_cast<Pilot*>(whazzupData.getPilots()[i]);
		if(p == 0) continue;
		if(airportMap.contains(p->planDep) && airportMap[p->planDep] != 0)
			airportMap[p->planDep]->addDeparture(p);
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
	}

	for(int i = 0; i < airportList.size(); i++) {
		if(airportList[i] != 0)
			airportList[i]->refreshAfterUpdate();
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
