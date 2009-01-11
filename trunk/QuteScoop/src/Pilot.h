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

#include "Airport.h"
#include "Client.h"

class Airport;

class Pilot: public Client
{	
public:
	enum FlightStatus { BOARDING, GROUND_DEP, DEPARTING, EN_ROUTE, ARRIVING, GROUND_ARR, BLOCKED, CRASHED, BUSH };
	
	Pilot(const QStringList& stringList, const WhazzupData* whazzup);

	virtual QString rank() const;
	
	void showDetailsDialog();	
	QString flightStatusString() const;
	FlightStatus flightStatus() const;
	
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
	
	int trueHeading;
	bool onGround; // IVAO only
	
	QString qnhInHg; // VATSIM only
	QString qnhMb; // VATSIM only
	
	QString aircraftType() const;
	Airport* depAirport() const;
	Airport* destAirport() const;
	QStringList waypoints() const;
	double distanceToDestination() const;
	double distanceFromDeparture() const;
	QString eta() const;
	void positionInFuture(double *lat, double *lon, int seconds) const;
	
	void toggleDisplayPath();
	void plotFlightPath() const;
	void plotPathToDest() const;
	void plotPathFromDep() const;
	
	bool displayLineFromDep, displayLineToDest;
	
	QList<QPair<double, double> > oldPositions;
	
private:
	void plotPath(double lat1, double lon1, double lat2, double lon2) const;
};

#endif /*PILOT_H_*/
