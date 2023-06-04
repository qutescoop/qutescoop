#include "Waypoint.h"

#include "Airac.h"
#include "NavData.h"

Waypoint::Waypoint(const QStringList& stringList)
    : MapObject() {
    if (stringList.size() != 6) {
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
    id = stringList[2];

    regionCode = stringList[4];
}

Waypoint::Waypoint(const QString& id, const double lat, const double lon)
    : MapObject() {
    this->id = id;
    this->lat = lat;
    this->lon = lon;
}

Waypoint::~Waypoint() {}

QString Waypoint::mapLabel() const {
    return id;
}

QStringList Waypoint::mapLabelSecondaryLinesHovered() const {
    return {}; //airwaysString().split("\n");
}

QString Waypoint::toolTip() const {
    return QString("%1 (%2)")
        .arg(id, NavData::toEurocontrol(lat, lon, LatLngPrecission::Secs));
}

int Waypoint::type() {
    return 0;
}

const QString Waypoint::airwaysString() const {
    QStringList ret;

    auto airac = Airac::instance(false);

    if (airac == 0) {
        return { };
    }

    foreach (const auto &awyList, airac->airways) {
        foreach (const auto &a, awyList) {
            if (a->waypoints().contains((Waypoint*) this)) {
                ret << a->name;
            }
        }
    }

    return ret.join("\n");
}
