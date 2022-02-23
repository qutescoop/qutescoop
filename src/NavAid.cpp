/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "NavAid.h"

NavAid::NavAid(const QStringList& stringList) {
	if(stringList.size() < 8 || stringList.size() > 15)
		return;

	bool ok;

    _type = (Type)stringList[0].toInt(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse waypointtype (int):" << stringList << 0;
		return;
	}
	lat = stringList[1].toDouble(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse lat (double):" << stringList << 1;
		return;
	}
	lon = stringList[2].toDouble(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse lon (double):" << stringList << 2;
		return;
	}

	_freq = stringList[4].toInt(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse frequency (int):" << stringList << 4;
		return;
	}

	label = stringList[7];

    regionCode = stringList[9];

	_name = "";
	for(int i = 8; i < stringList.size(); i++)
		_name += stringList[i] + (i > 8? " ": "");
}

QString NavAid::typeStr(Type type) {
	QHash<Type, QString> hash;
	hash.reserve(10);
	hash.insert(NDB, "NDB");
	hash.insert(VOR, "VOR");
	hash.insert(DME, "DME");
	hash.insert(ILS_LOC, "ILS");
	hash.insert(LOC, "LOC");
	hash.insert(GS, "GS");
	hash.insert(OM, "OM");
	hash.insert(MM, "OM");
	hash.insert(IM, "IM");
	hash.insert(DME_NO_FREQ, "DME (no freq)");
    hash.insert(DME, "DME");
    hash.insert(FAP_GBAS, "FAP alignment point");
    hash.insert(GBAS_GND, "GBAS Ground station");
    hash.insert(GBAS_THR, "GBAS Threshold point");
	return hash.value(type, QString());
}

QString NavAid::toolTip() const {
	QString ret = Waypoint::toolTip();
    if (_type == NDB)
        ret.append(QString(" %1 kHz").arg(_freq));
    else if (_type == VOR || _type == DME || _type == DME_NO_FREQ || _type == ILS_LOC || _type == LOC || _type == GS)
        ret.append(QString(" %1 MHz").arg(_freq / 100., 0, 'f', 2));
    else if (_freq != 0)
        ret.append(QString(" %1?").arg(_freq));
    if ((_type == ILS_LOC || _type == LOC) && _hdg != 0)
        ret.append(QString(" %1").arg((double) _hdg, 0, 'f', 0));
    if (!NavAid::typeStr(_type).isEmpty())
        ret.append(QString(" [%1]").arg(NavAid::typeStr(_type)));
    return ret;
}
