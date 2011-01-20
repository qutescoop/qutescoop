/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef AIRAC_H_
#define AIRAC_H_

#include <QHash>
#include <QString>
#include <QList>

#include "Waypoint.h"
#include "NavAid.h"
#include "Airway.h"

class Airac {
public:
	Airac();
	virtual ~Airac();

	void load(const QString& directory);

	bool isEmpty() const { return waypointMap.isEmpty(); }

	/**
	 * Returns the waypoint with the given id closest to the given lat/lon.
	 */
	Waypoint* getWaypoint(const QString& id, double lat, double lon, double maxDist = 2000.0) const;

	/**
	 * Returns a list of waypoints for the given planned route, starting at lat/lon.
	 * Airways along the route will be replaced by the appropriate fixes along that
	 * airway. lat/lon is being used as a hint and should be the position of the
	 * departure airport.
	 *
	 * Input format can be:
	 * 1. FIX - FIX - FIX
	 * 2. FIX - Airway - FIX - Airway - FIX
	 *
	 * Unknown fixes and/or airways will be ignored.
	 */
	QList<Waypoint*> resolveFlightplan(const QStringList& flightplan, double lat, double lon) const;

	Airway* getAirway(const QString& name, double lat, double lon) const;

	const QList<Waypoint*>& getAllWaypoints() const { return allWaypoints; }

private:
	QHash<QString, QList<Waypoint*> > waypointMap;
	QHash<QString, QList<NavAid*> > navaidMap;
	QHash<QString, QList<Airway*> > airwayMap;
	QList<Waypoint*> allWaypoints;

	void readFixes(const QString& directory);
	void addFix(Waypoint* fix);

	void readNavaids(const QString& directory);
	void readAirways(const QString& directory);
	void addAirwaySegment(Waypoint* from, Waypoint* to, Airway::Type type, int base, int top, const QString& name);

	Airway* getAirway(const QString& name, Airway::Type type, int base, int top);
	Waypoint* getNextWaypoint(QStringList& workingList, double lat, double lon) const;
};

#endif /* AIRAC_H_ */
