#include "Waypoint.h"

#include "Airac.h"
#include "NavData.h"

Waypoint::Waypoint(const QStringList& fields)
    : MapObject() {
    if (fields.size() != 6) {
        QMessageLogger("earth_fix.dat", 0, QT_MESSAGELOG_FUNC).critical()
            << fields << ": Expected 6 fields";
        return;
    }

    bool ok;

    lat = fields[0].toDouble(&ok);
    if (!ok) {
        QMessageLogger("earth_fix.dat", 0, QT_MESSAGELOG_FUNC).critical()
            << fields << ": unable to parse lat";
        return;
    }
    lon = fields[1].toDouble(&ok);
    if (!ok) {
        QMessageLogger("earth_fix.dat", 0, QT_MESSAGELOG_FUNC).critical()
            << fields << ": unable to parse lon";
        return;
    }
    id = fields[2];

    regionCode = fields[4];
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
    return {
        // airwaysString().split("\n")
    };
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
