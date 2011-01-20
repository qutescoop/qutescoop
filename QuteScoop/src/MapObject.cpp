/**************************************************************************
 *  This file is part of QuteScoop. See README for license
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
