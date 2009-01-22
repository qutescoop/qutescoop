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

#include "Airway.h"

Airway::Airway(const QString& name, Type type, int base, int top) {
	this->name = name;
	this->type = type;
	this->base = base;
	this->top = top;
}

Airway::~Airway() {
	// TODO Auto-generated destructor stub
}

void Airway::addSegment(Waypoint* from, Waypoint* to) {
	if(waypoints.isEmpty()) {
		waypoints.append(from);
		waypoints.append(to);
		return;
	}

	for(int i = 0; i < waypoints.size(); i++) {
		if(waypoints[i] == from) {
			waypoints.insert(i+1, to);
			return;
		}
	}
}

Waypoint* Airway::getPoint(const QString& id) {
	for(int i = 0; i < waypoints.size(); i++) {
		if(waypoints[i]->id == id) {
			return waypoints[i];
		}
	}
	return 0;
}


QList<Waypoint*> Airway::expand(Waypoint* start, Waypoint* end) const {
	QList<Waypoint*> result;

	int startIndex = waypoints.indexOf(start);
	int endIndex = waypoints.indexOf(end);
	if(start < 0 || end < 0) return result;

	if(startIndex > endIndex) {
		int x = startIndex;
		startIndex = endIndex;
		endIndex = x;
	}

	for(int i = startIndex + 1; i <= endIndex; i++) {
		result.append(waypoints[i]);
	}

	return result;
}
