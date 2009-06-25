/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
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
