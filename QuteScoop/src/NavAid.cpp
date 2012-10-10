/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "NavAid.h"

NavAid::NavAid(const QStringList& stringList) {
	if(stringList.size() < 8 || stringList.size() > 15)
		return;

	bool ok;

	type = (Type)stringList[0].toInt(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse waypointtype (int):" << stringList;
		return;
	}
	lat = stringList[1].toDouble(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse lat (double):" << stringList;
		return;
	}
	lon = stringList[2].toDouble(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse lon (double):" << stringList;
		return;
	}
	altitude = stringList[3].toInt(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse altitude (int):" << stringList;
		return;
	}
	frequency = stringList[4].toInt(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse frequency (int):" << stringList;
		return;
	}
	range = stringList[5].toInt(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse range (int):" << stringList;
		return;
	}
	heading = stringList[6].toFloat(&ok);
	if(!ok) {
		qWarning() << "NavAid::NavAid() unable to parse heading (float):" << stringList;
		return;
	}
	label = stringList[7];

	name = "";
	for(int i = 8; i < stringList.size(); i++)
		name += stringList[i] + (i > 8? " ": "");
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
	return hash.value(type, QString());
	if (type == NDB)
		return (QString("NDB"));
	else if (type == VOR)
		return (QString("VOR"));
	else if (type == DME)
		return (QString("DME"));
	else if (type == ILS_LOC)
		return (QString("ILS"));
	else if (type == LOC)
		return (QString("LOC"));
	else if (type == GS)
		return (QString("GS"));
	else if (type == OM)
		return (QString("OM"));
	else if (type == MM)
		return (QString("MM"));
	else if (type == IM)
		return (QString("IM"));
	else if (type == DME_NO_FREQ)
		return (QString("DME"));
	return QString();
}

QString NavAid::toolTip() const {
	QString ret = Waypoint::toolTip();
	if (type == NDB)
		ret.append(QString(" %1 kHz").arg(frequency));
	else if (type == VOR || type == DME || type == DME_NO_FREQ || type == ILS_LOC || type == LOC || type == GS)
		ret.append(QString(" %1 MHz").arg(frequency / 100., 0, 'f', 2));
	else if (frequency != 0)
		ret.append(QString(" %1?").arg(frequency));
	if ((type == ILS_LOC || type == LOC) && heading != 0)
		ret.append(QString(" %1").arg((double) heading, 0, 'f', 0));
	if (!NavAid::typeStr(type).isEmpty())
		ret.append(QString(" [%1]").arg(NavAid::typeStr(type)));
	return ret;
}

int NavAid::getTyp()
{
    return Type(type);
}
