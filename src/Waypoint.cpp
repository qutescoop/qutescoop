/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Waypoint.h"

Waypoint::Waypoint(const QStringList& stringList) {
    if(stringList.size() != 6){
        qCritical() << "Waypoint(): could not parse " << stringList << " as Waypoint. Expected 6 fields.";
        return;
    }

    bool ok;

    lat = stringList[0].toDouble(&ok);
    if (!ok) {
        qCritical() << "Waypoint::Waypoint() unable to parse lat:" << stringList;
        return;
    }
    lon = stringList[1].toDouble(&ok);
    if (!ok) {
        qCritical() << "Waypoint::Waypoint() unable to parse lon:" << stringList;
        return;
    }
    label = stringList[2];

    regionCode = stringList[4];
}

Waypoint::Waypoint(const QString& id, const double lat, const double lon) {
    this->label = id;
    this->lat = lat;
    this->lon = lon;
}

QString Waypoint::toolTip() const {
    return QString("%1 (%2%3 %4%5)")
        .arg(label).
         arg(lat > 0? "N": "S").
         arg(qAbs(lat), 6, 'f', 3, '0').
         arg(lon > 0? "E": "W").
         arg(qAbs(lon), 7, 'f', 3, '0');
}
