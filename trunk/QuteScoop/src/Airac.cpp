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

void Airac::addFix(Waypoint* fix) {
	QList<Waypoint*> list = waypointMap[fix->id];
	list.append(fix);
	waypointMap[fix->id] = list;
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

		addFix(wp);
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
	int segments = 0;

	qDebug() << "reading airways from file" << (directory + "/default data/earth_awy.dat") << "...";
	FileReader fr(directory + "/default data/earth_awy.dat");
	while(!fr.atEnd()) {
		QString line = fr.nextLine().trimmed();
		if(line.isEmpty())
			continue;

		QStringList list = line.split(' ', QString::SkipEmptyParts);
		if(list.size() != 10) {
			qDebug() << "too short: skipping line" << line;
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
			qDebug() << "added fix (start):" << start->id << start->lat << start->lon;
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
			qDebug() << "added fix (end):" << end->id << end->lat << end->lon;
		}

		Airway::Type type = (Airway::Type)list[6].toInt(&ok);
		if(!ok) continue;

		int base = list[7].toInt(&ok);
		if(!ok) continue;
		int top = list[8].toInt(&ok);
		if(!ok) continue;

		QStringList names = list[9].split('-', QString::SkipEmptyParts);
		for(int i = 0; i < names.size(); i++) {
			addAirwaySegment(start, end, type, base, top, names[i]);
			segments++;
		}
	}

	qDebug() << "sorting airways...";

	QHash<QString, Airway*>::iterator iter;
	for(iter = airwayMap.begin(); iter != airwayMap.end(); ++iter) {
		iter.value()->sort();
	}

	qDebug() << "done loading airways:" << airwayMap.size() << "names," << segments << "segments";
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

void Airac::addAirwaySegment(Waypoint* from, Waypoint* to, Airway::Type type, int base, int top, const QString& name) {
	Airway* awy = airwayMap[name];
	if(awy == 0) {
		awy = new Airway(name, type, base, top);
		airwayMap[name] = awy;
	}
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

void dump(QList<Waypoint*> list);

QList<Waypoint*> Airac::getWaypoints(const QStringList& plan, double lat, double lon) const {
	QList<Waypoint*> result;
	if(plan.isEmpty()) return result;

	QStringList workingList = plan;

	// find a starting point
	Waypoint* currPoint = getNextWaypoint(workingList, lat, lon);
	if(currPoint == 0) return result;

	result.append(currPoint);
	double myLat = currPoint->lat;
	double myLon = currPoint->lon;

	while(!workingList.isEmpty()) {
		QString id = workingList.first();
		workingList.removeFirst();

		Airway *awy = airwayMap[id];
		if(awy != 0 && !workingList.isEmpty()) {
			// have airway - next should be a waypoint
			QString endId = workingList.first();
			Waypoint* wp = getWaypoint(endId, myLat, myLon);
			if(wp != 0) {
				// next is a waypoint - expand airway
				result += awy->expand(currPoint->id, wp->id);
				currPoint = wp;
				myLat = wp->lat;
				myLon = wp->lon;
				workingList.removeFirst();
				continue;
			}
		} else if(awy == 0) {
			Waypoint* wp = getWaypoint(id, myLat, myLon);
			if(wp != 0) {
				// next is a waypoint - expand airway
				result.append(wp);
				currPoint = wp;
				myLat = wp->lat;
				myLon = wp->lon;
				continue;
			}
		}
	}

	qDebug() << "expanded" << plan << "to:";
	dump(result);

	return result;
}
