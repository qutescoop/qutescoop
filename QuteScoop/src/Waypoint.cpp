/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Waypoint.h"

Waypoint::Waypoint(const QStringList& stringList) {
	if(stringList.size() != 3)
		return;

	bool ok;
	lat = stringList[0].toDouble(&ok);
	if(!ok)
		return;
	lon = stringList[1].toDouble(&ok);
	if(!ok)
		return;
	label = stringList[2];
}

Waypoint::Waypoint(const QString& id, double lat, double lon) {
	this->label = id;
	this->lat = lat;
	this->lon = lon;
}

QString Waypoint::toolTip() const {
	return QString("%1 (%2%3 %4%5)").arg(label).
			arg(lat > 0? "N": "S").
			arg(qAbs(lat), 6, 'f', 3, '0').
			arg(lon > 0? "E": "W").
			arg(qAbs(lon), 7, 'f', 3, '0');
}
