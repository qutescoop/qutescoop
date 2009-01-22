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

#ifndef AIRWAY_H_
#define AIRWAY_H_

#include <QString>
#include <QList>
#include "Waypoint.h"

class Airway {
public:
	enum Type {
		LOW = 1,
		HIGH = 2
	};

	Airway(const QString& name, Type type, int base, int top);
	virtual ~Airway();

	void addSegment(Waypoint* from, Waypoint* to);

	/**
	 * Returns a list of all fixes along this airway from start
	 * to end. The expanded list will not include the given start
	 * point, but will include the given end point.
	 */
	QList<Waypoint*> expand(Waypoint* start, Waypoint* end) const;

	/**
	 * Returns the first waypoint on this airway with the given id
	 */
	Waypoint* getPoint(const QString& id);

private:
	Type type;
	int base;
	int top;
	QString name;
	QList<Waypoint*> waypoints;
};

#endif /* AIRWAY_H_ */
