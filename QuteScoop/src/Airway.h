/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef AIRWAY_H_
#define AIRWAY_H_

#include <QString>
#include <QList>
#include "Waypoint.h"

class Airway {
public:
	enum Type {
		LOW = 1,
		HIGH = 2
	};
	Type type;
	int base;
	int top;
	QString name;

	Airway(const QString& name, Type type, int base, int top);
	virtual ~Airway();

	void addSegment(Waypoint* from, Waypoint* to);

	/**
	 * Returns a list of all fixes along this airway from start
	 * to end. The expanded list will not include the given start
	 * point, but will include the given end point.
	 */
	QList<Waypoint*> expand(const QString& startId, const QString& endId) const;

	Waypoint* getClosestPointTo(double lat, double lon) const;

	QList<Airway*> sort();
	void dumpSegments() const;
	void dumpWaypoints() const;

private:
	int getIndex(const QString& id) const;

	class Segment {
	public:
		Segment(Waypoint *from, Waypoint *to);
		bool operator==(const Segment& other) const;
		Waypoint *from;
		Waypoint *to;
	};

	QList<Segment> segments;
	QList<Waypoint*> waypoints;

	Airway* createFromSegments();
};

#endif /* AIRWAY_H_ */
