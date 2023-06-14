#include "Route.h"

#include "Airac.h"
#include "Airport.h"
#include "NavData.h"

Route::Route() {}
Route::~Route() {}

void Route::calculateWaypointsAndDistance() {
    Airport* depAirport;
    if (NavData::instance()->airports.contains(dep)) {
        depAirport = NavData::instance()->airports[dep];
    } else {
        return;
    }

    Airport* destAirport;
    if (NavData::instance()->airports.contains(dest)) {
        destAirport = NavData::instance()->airports[dest];
    } else {
        return;
    }

    QStringList list = route.split(' ', Qt::SkipEmptyParts);
    waypoints = Airac::instance()->resolveFlightplan(list, depAirport->lat, depAirport->lon, Airac::ifrMaxWaypointInterval);

    Waypoint* depWp = new Waypoint(depAirport->id, depAirport->lat, depAirport->lon);
    waypoints.prepend(depWp);
    Waypoint* destWp = new Waypoint(destAirport->id, destAirport->lat, destAirport->lon);
    waypoints.append(destWp);

    waypointsStr = QString();
    double dist = 0;
    for (int i = 0; i < waypoints.size(); i++) {
        if (routeDistance.isEmpty() && i > 0) {
            dist += NavData::distance(
                waypoints[i - 1]->lat, waypoints[i - 1]->lon,
                waypoints[i]->lat, waypoints[i]->lon
            );
        }
        waypointsStr += waypoints[i]->id;
        if (i < waypoints.size() - 1) {
            waypointsStr += " ";
        }
    }
    // setting distance if it was not set before
    if (routeDistance.isEmpty()) {
        routeDistance = QString("%1 NM (calculated)").arg(qRound(dist));
    }
}
