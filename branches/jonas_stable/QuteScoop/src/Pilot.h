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

#ifndef PILOT_H_
#define PILOT_H_

#include <QList>

#include "Airport.h"
#include "Client.h"
#include "Waypoint.h"

class Airport;

class Pilot: public Client
{
public:
	enum FlightStatus { BOARDING, GROUND_DEP, DEPARTING, EN_ROUTE, ARRIVING, GROUND_ARR, BLOCKED, CRASHED, BUSH, PREFILED };

	Pilot(const QStringList& stringList, const WhazzupData* whazzup);

	virtual QString rank() const;

	void showDetailsDialog();
	FlightStatus flightStatus() const;
    QString flightStatusString() const;
    QString flightStatusShortString() const;

	int altitude;
	int groundspeed;
	QString planAircraft;
	QString planTAS;
	QString planDep;
	QString planAlt;
	QString planDest;
	QString transponder;
	QString planRevision;
	QString planFlighttype;
	QString planDeptime;
    QDate dayOfFlight;

	QString planActtime;
	int planHrsEnroute;
	int planMinEnroute;
	int planHrsFuel;
	int planMinFuel;
	QString planAltAirport;
	QString planRemarks;
	QString planRoute;

	QString planAltAirport2; // IVAO only
	QString planTypeOfFlight; // IVAO only
	int pob; // IVAO only

    double trueHeading;
	bool onGround; // IVAO only

	QString qnhInHg; // VATSIM only
	QString qnhMb; // VATSIM only

	QString aircraftType() const;
	Airport* depAirport() const;
    Airport* destAirport() const;
    Airport* altAirport() const;
    QStringList waypoints() const;
	double distanceToDestination() const;
	double distanceFromDeparture() const;

    QDateTime etd() const; // Estimated Time of Departure
    QDateTime eta() const; // Estimated Time of Arrival
    QDateTime fixedEta; //ETA, written after creation as a workaround for Prediction (Warp) Mode
    QTime ete() const; // Estimated Remaining Time Enroute
    QDateTime etaPlan() const; // Estimated Time of Arrival as flightplanned
    QString delayStr() const;

    QDateTime whazzupTime; // need some local reference to that

    int planTasInt() const; // defuck TAS for Mach numbers

    int defuckPlanAlt(QString alt) const; // // returns an altitude from various flightplan strings

	void positionInFuture(double *lat, double *lon, int seconds) const;

	void toggleDisplayPath();
	void plotFlightPath() const;

	bool displayLineFromDep, displayLineToDest;

	QList<QPair<double, double> > oldPositions;

private:
    void plotPathToDest() const;
	void plotPathFromDep() const;
	void plotPlannedLine() const;
	void plotPath(double lat1, double lon1, double lat2, double lon2) const;
	QList<Waypoint*> resolveFlightplan() const;
};

#endif /*PILOT_H_*/
