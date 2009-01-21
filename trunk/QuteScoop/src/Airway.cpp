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
