/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef ROUTE_H_
#define ROUTE_H_

#include "Waypoint.h"

class Route: public QObject {
        Q_OBJECT
    public:
        Route();
        virtual ~Route();

        QString provider, dep, dest, route, minFl, maxFl, airacCycle, lastChange,
        comments, routeDistance, waypointsStr;
        QList<Waypoint*> waypoints;
        void calculateWaypointsAndDistance();
};

#endif // ROUTE_H
