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

#ifndef AIRAC_H_
#define AIRAC_H_

#include <QHash>
#include <QString>
#include <QList>

#include "Waypoint.h"
#include "NavAid.h"
#include "Airway.h"

class Airac {
public:
	Airac();
	virtual ~Airac();

	void load(const QString& directory);

	/**
	 * Returns the waypoint with the given id closest to the given lat/lon.
	 */
	Waypoint* getWaypoint(const QString& id, float lat, float lon) const;

private:
	QHash<QString, QList<Waypoint*> > waypointMap;
	QHash<QString, QList<NavAid*> > navaidMap;
	QHash<QString, Airway*> airwayMap;

	void readFixes(const QString& directory);
	void readNavaids(const QString& directory);
	void readAirways(const QString& directory);
	void addAirwaySegment(Waypoint* from, Waypoint* to, Airway::Type type, int base, int top, const QString& name);
};

#endif /* AIRAC_H_ */
