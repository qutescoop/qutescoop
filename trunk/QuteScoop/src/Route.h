/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef ROUTE_H
#define ROUTE_H

#include <QObject>
#include <QStringList>

class Route: public QObject 
{
    Q_OBJECT
            
public:
    //enum RouteProvider { ME, VROUTE };
    Route(const QStringList& sl);
	virtual ~Route();

    QString provider, dep, dest, flightPlan, minFl, maxFl, airacCycle, lastChange, comments, routeDistance;
    //RouteProvider provider;
};

#endif // ROUTE_H
