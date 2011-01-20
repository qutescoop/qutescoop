/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

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
