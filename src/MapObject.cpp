/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "MapObject.h"

MapObject::MapObject() :
    QObject(),
    lat(0.),
    lon(0.),
    drawLabel(true) {
}

MapObject::~MapObject() {
}

MapObject::MapObject(const MapObject& obj):
        QObject() {
	if(this == &obj)
		return;

	lat = obj.lat;
	lon = obj.lon;
	label = obj.label;
        drawLabel = obj.drawLabel;
}

MapObject& MapObject::operator=(const MapObject& obj) {
	if(this == &obj)
		return *this;

	lat = obj.lat;
	lon = obj.lon;
	label = obj.label;
        drawLabel = obj.drawLabel;
	return *this;
}
