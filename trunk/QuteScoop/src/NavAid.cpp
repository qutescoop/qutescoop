/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "NavAid.h"

NavAid::NavAid(const QStringList& stringList) {
	if(stringList.size() < 9)
		return;

	bool ok;

	type = (Type)stringList[0].toInt(&ok);
	if(!ok) return;
	lat = stringList[1].toDouble(&ok);
	if(!ok) return;
	lon = stringList[2].toDouble(&ok);
	if(!ok) return;
	altitude = stringList[3].toInt(&ok);
	if(!ok) return;
	frequency = stringList[4].toInt(&ok);
	if(!ok) return;
	range = stringList[5].toInt(&ok);
	if(!ok) return;
	heading = stringList[6].toFloat(&ok);
	if(!ok) return;
	label = stringList[7];

	for(int i = 8; i < stringList.size(); i++) {
		name += stringList[i];
		if(i > 8) name += " ";
	}
}

QString NavAid::toolTip() const {
	return Waypoint::toolTip() + QString(" - %6").
			arg(type == NDB?
				QString("%1kHz [NDB]").arg(frequency): (
						type == VOR?
						QString("%1MHz [VOR]").arg(frequency / 100., 0, 'f', 2): (
								type == DME?
								QString("%1MHz [DME]").arg(frequency / 100., 0, 'f', 2):
								QString("%1").arg(frequency))
						)
				);

}
