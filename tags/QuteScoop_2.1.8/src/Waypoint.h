/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WAYPOINT_H_
#define WAYPOINT_H_

#include <QStringList>
#include "MapObject.h"

class Waypoint: public MapObject
{
    public:
        Waypoint() {}
        Waypoint(const QStringList& stringList);
        Waypoint(const QString& id, const double lat, const double lon);

        virtual QString mapLabel() const { return label; }
        virtual QString toolTip() const;
        virtual void showDetailsDialog() {} // not applicable
        virtual int getTyp() { return 0;}
};

#endif /*WAYPOINT_H_*/
