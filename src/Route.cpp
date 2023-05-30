/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Route.h"

#include "Airac.h"
#include "Airport.h"
#include "NavData.h"

Route::Route() {
}
Route::~Route() {
}

void Route::calculateWaypointsAndDistance() {
    Airport* depAirport;
    if(NavData::instance()->airports.contains(dep))
        depAirport = NavData::instance()->airports[dep];
    else
        return;

    Airport* destAirport;
    if(NavData::instance()->airports.contains(dest))
        destAirport = NavData::instance()->airports[dest];
    else
        return;

    QStringList list = route.split(' ', Qt::SkipEmptyParts);
    waypoints = Airac::instance()->resolveFlightplan(list, depAirport->lat, depAirport->lon);

    Waypoint* depWp = new Waypoint(depAirport->label, depAirport->lat, depAirport->lon);
    waypoints.prepend(depWp);
    Waypoint* destWp = new Waypoint(destAirport->label, destAirport->lat, destAirport->lon);
    waypoints.append(destWp);

    waypointsStr = QString();
    double dist = 0;
    for(int i = 0; i < waypoints.size(); i++) {
        if (routeDistance.isEmpty() && i > 0) {
            dist += NavData::distance(waypoints[i-1]->lat, waypoints[i-1]->lon,
                                      waypoints[i]->lat, waypoints[i]->lon);
        }
        waypointsStr += waypoints[i]->label;
        if (i < waypoints.size()-1) waypointsStr += " ";
    }
    // setting distance if it was not set before
    if (routeDistance.isEmpty())
        routeDistance = QString("%1 NM (calculated)").arg(qRound(dist));
}
