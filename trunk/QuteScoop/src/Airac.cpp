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

Airac::Airac() {
	// TODO Auto-generated constructor stub

}

Airac::~Airac() {
	// TODO Auto-generated destructor stub
}

void Airac::load(const QString& directory) {
	// directory should contain navigation database files

	// default data/earth_fix.dat

	waypointMap.clear();
	qDebug() << "Reading fixes from file" << (directory + "/default data/earth_fix.dat");
	FileReader fr(directory + "/default data/earth_fix.dat");
	while(!fr.atEnd()) {
		QString line = fr.nextLine().trimmed();
		if(line.isEmpty())
			continue;

		Waypoint *wp = new Waypoint(line.split(' ', QString::SkipEmptyParts));
		if (wp == 0 || wp->isNull())
			continue;

		QList<Waypoint*> list = waypointMap[wp->name];
		list.append(wp);
		waypointMap[wp->name] = list;
	}
	qDebug() << "done loading waypoints:" << waypointMap.size() << "names";
}
