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
	Waypoint(const QString& id, double lat, double lon);

	virtual QString mapLabel() const { return label; }
	virtual QString toolTip() const;

	virtual void showDetailsDialog() {};
};

#endif /*WAYPOINT_H_*/
