/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Route.h"

Route::Route(const QStringList& sl)
{
    provider = sl[0];
    routeDistance = sl[1];
    dep = sl[2];
    dest = sl[3];
    minFl = sl[4];
    maxFl = sl[5];
    flightPlan = sl[6];
    lastChange = sl[7];
    comments = sl[8];
    airacCycle = QString();
}

Route::~Route()
{
}
