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

#include "Waypoint.h"

Waypoint::Waypoint(const QStringList& stringList)
{
	if(stringList.size() != 3)
		return;

	bool ok;
	lat = stringList[0].toDouble(&ok);
	if(!ok) return;
	lon = stringList[1].toDouble(&ok);
	if(!ok) return;
	label = stringList[2];
}

Waypoint::Waypoint(const QString& id, double lat, double lon) {
	this->label = id;
	this->lat = lat;
	this->lon = lon;
}
