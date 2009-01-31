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

#include "MapObject.h"

MapObject::MapObject()
{
}

MapObject::~MapObject() {
}

MapObject::MapObject(const MapObject& obj):
	QObject()
{
	if(this == &obj)
		return;

	lat = obj.lat;
	lon = obj.lon;
	label = obj.label;
}

MapObject& MapObject::operator=(const MapObject& obj) {
	if(this == &obj)
		return *this;

	lat = obj.lat;
	lon = obj.lon;
	label = obj.label;
	return *this;
}

bool MapObject::matches(const QRegExp& regex) const {
	return label.contains(regex);
}
