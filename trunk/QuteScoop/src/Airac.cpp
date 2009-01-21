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

#include <QDebug>

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
}

void Airac::readFixes(const QString& directory) {
	waypointMap.clear();
	qDebug() << "reading fixes from file" << (directory + "/default data/earth_fix.dat") << "...";
	FileReader fr(directory + "/default data/earth_fix.dat");
	while(!fr.atEnd()) {
		QString line = fr.nextLine().trimmed();
		if(line.isEmpty())
			continue;

		Waypoint *wp = new Waypoint(line.split(' ', QString::SkipEmptyParts));
		if (wp == 0 || wp->isNull())
			continue;

		QList<Waypoint*> list = waypointMap[wp->id];
		list.append(wp);
		waypointMap[wp->id] = list;
	}
	qDebug() << "done loading waypoints:" << waypointMap.size() << "names";
}

void Airac::readNavaids(const QString& directory) {
	navaidMap.clear();
	qDebug() << "reading fixes from file" << (directory + "/default data/earth_nav.dat") << "...";
	FileReader fr(directory + "/default data/earth_nav.dat");
	while(!fr.atEnd()) {
		QString line = fr.nextLine().trimmed();
		if(line.isEmpty())
			continue;

		NavAid *nav = new NavAid(line.split(' ', QString::SkipEmptyParts));
		if (nav == 0 || nav->isNull())
			continue;

		QList<NavAid*> list = navaidMap[nav->id];
		list.append(nav);
		navaidMap[nav->id] = list;
	}
	qDebug() << "done loading navaids:" << navaidMap.size() << "names";
}

void Airac::readAirways(const QString& directory) {
	bool ok;

	qDebug() << "reading airways from file" << (directory + "/default data/earth_awy.dat") << "...";
	FileReader fr(directory + "/default data/earth_awy.dat");
	while(!fr.atEnd()) {
		QString line = fr.nextLine().trimmed();
		if(line.isEmpty())
			continue;

		QStringList list = line.split(' ', QString::SkipEmptyParts);
		if(list.size() != 10) continue;

		QString id = list[0];
		float lat = list[1].toFloat(&ok);
		if(!ok) continue;
		float lon = list[2].toFloat(&ok);
		if(!ok) continue;

		Waypoint *start = getWaypoint(id, lat, lon);
		if(start == 0) continue;

		id = list[3];
		lat = list[4].toFloat(&ok);
		if(!ok) continue;
		lon = list[5].toFloat(&ok);
		if(!ok) continue;

		Waypoint *end = getWaypoint(id, lat, lon);
		if(end == 0) continue;

		Airway::Type type = (Airway::Type)list[6].toInt(&ok);
		if(!ok) continue;

		int base = list[7].toInt(&ok);
		if(!ok) continue;
		int top = list[8].toInt(&ok);
		if(!ok) continue;

		QStringList names = list[9].split('-', QString::SkipEmptyParts);
		for(int i = 0; i < names.size(); i++) {
			addAirwaySegment(start, end, type, base, top, names[i]);
		}
	}
	qDebug() << "done loading airways:" << airwayMap.size() << "names";
}

Waypoint* Airac::getWaypoint(const QString& id, float lat, float lon) const {
	Waypoint *result = 0;
	double minDist = 99999;

	QList<Waypoint*> points = waypointMap[id];
	for(int i = 0; i < points.size() && minDist > 0; i++) {
		double d = NavData::distance(lat, lon, points[i]->lat, points[i]->lon);
		if(d < minDist) {
			result = points[i];
			minDist = d;
		}
	}

	QList<NavAid*> navs = navaidMap[id];
	for(int i = 0; i < navs.size() && minDist > 0; i++) {
		double d = NavData::distance(lat, lon, navs[i]->lat, navs[i]->lon);
		if(d < minDist) {
			result = navs[i];
			minDist = d;
		}
	}

	return result;
}

void Airac::addAirwaySegment(Waypoint* from, Waypoint* to, Airway::Type type, int base, int top, const QString& name) {
	Airway* awy = airwayMap[name];
	if(awy == 0) {
		awy = new Airway(name, type, base, top);
		airwayMap[name] = awy;
	}
	awy->addSegment(from, to);
}
