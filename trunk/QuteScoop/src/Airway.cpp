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

#include "Airway.h"

Airway::Segment::Segment(Waypoint* from, Waypoint* to) {
	this->from = from;
	this->to = to;
}

bool Airway::Segment::operator==(const Airway::Segment& other) const {
	return (from == other.from || from == other.to) && (to == other.from || to == other.to);
}

Airway::Airway(const QString& name, Type type, int base, int top) {
	this->name = name;
	this->type = type;
	this->base = base;
	this->top = top;
	segments = new QList<Segment>();
}

Airway::~Airway() {
	if(segments != 0) {
		delete segments;
	}
}

void Airway::addSegment(Waypoint* from, Waypoint* to) {
	if(segments == 0) {
		qDebug() << "cannot add segments on already sorted airway!";
		return;
	}

	Segment newSegment(from, to);

	// check if we already have this segment
	for(int i = 0; i < segments->size(); i++) {
		if((*segments)[i] == newSegment) {
			return;
		}
	}

	segments->append(newSegment);
}

void dump(const QList<Waypoint*> points) {
	QString line;
	for(int i = 0; i < points.size(); i++) {
		line = line + points[i]->id + " - ";
	}
	qDebug() << line << "*";
}

void Airway::sort() {
	if(segments == 0) {
		qDebug() << "cannot sort already sorted airway!";
		return;
	}

	if(segments->isEmpty()) {
		delete segments;
		segments = 0;
		return;
	}

	Segment seg = segments->first();
	segments->removeFirst();
	waypoints.append(seg.from);
	waypoints.append(seg.to);

	bool nothingRemoved = false;
	while(!segments->isEmpty() && !nothingRemoved) {
		nothingRemoved = true;
		for(int i = 0; i < segments->size() && nothingRemoved; i++) {
			Waypoint *p = waypoints.last();
			Segment s = (*segments)[i];

			if(s.from == p) {
				waypoints.append(s.to);
				segments->removeAt(i);
				nothingRemoved = false;
				continue;
			}
			if(s.to == p) {
				waypoints.append(s.from);
				segments->removeAt(i);
				nothingRemoved = false;
				continue;
			}

			p = waypoints.first();
			if(s.from == p) {
				waypoints.prepend(s.to);
				segments->removeAt(i);
				nothingRemoved = false;
				continue;
			}
			if(s.to == p) {
				waypoints.prepend(s.from);
				segments->removeAt(i);
				nothingRemoved = false;
				continue;
			}
		}
	}

	if(!segments->isEmpty()) {
		qDebug() << "there are still" << segments->size() << "segments left in airway" << name << "!";
	}

	if(name == "UL603")
		dump(waypoints);

	delete segments;
	segments = 0;
}

int Airway::getIndex(const QString& id) const {
	for(int i = 0; i < waypoints.size(); i++) {
		if(waypoints[i]->id == id) {
			return i;
		}
	}
	return -1;
}

QList<Waypoint*> Airway::expand(const QString& startId, const QString& endId) const {
	QList<Waypoint*> result;

	int startIndex = getIndex(startId);
	int endIndex = getIndex(endId);

	if(startIndex < 0 || endIndex < 0) return result;

	int direction = 1;
	if(startIndex > endIndex) {
		direction = -1;
	}

	for(int i = startIndex; i != endIndex; i += direction) {
		if(i != startIndex) {
			// don't append first waypoint in list
			result.append(waypoints[i]);
		}
	}
	result.append(waypoints[endIndex]);

	qDebug() << "expanded" << startId << name << endId << "to";
	dump(result);

	return result;
}