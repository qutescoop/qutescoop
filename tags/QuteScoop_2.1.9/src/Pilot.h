/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef PILOT_H_
#define PILOT_H_

#include "_pch.h"

#include "Airport.h"
#include "Client.h"
#include "Waypoint.h"

class Airport;

class Pilot: public Client
{
public:
    enum FlightStatus {
        BOARDING, GROUND_DEP, DEPARTING, EN_ROUTE, ARRIVING,
        GROUND_ARR, BLOCKED, CRASHED, BUSH, PREFILED
    };

    Pilot(const QStringList& stringList, const WhazzupData* whazzup);

    virtual QString toolTip() const;
    virtual QString rank() const;
    virtual void showDetailsDialog();

    FlightStatus flightStatus() const;
    QString flightStatusString() const;
    QString flightStatusShortString() const;
    QString planFlighttypeString() const;
    QString aircraftType() const;
    Airport *depAirport() const;
    Airport *destAirport() const;
    Airport *altAirport() const;
    QStringList waypoints() const;
    double distanceToDestination() const;
    double distanceFromDeparture() const;
    QDateTime etd() const; // Estimated Time of Departure
    QDateTime eta() const; // Estimated Time of Arrival
    //QDateTime fixedEta; // ETA, written after creation as a workaround for Prediction (Warp) Mode
    QTime eet() const; // Estimated Enroute Time as remaining time to destination
    QDateTime etaPlan() const; // Estimated Time of Arrival as flightplanned
    QString delayStr() const;
    int planTasInt() const; // defuck TAS for Mach numbers
    int defuckPlanAlt(QString alt) const; // returns an altitude from various flightplan strings
    QPair<double, double> positionInFuture(int seconds) const;
    int nextPointOnRoute(const QList<Waypoint*> &waypoints) const;
    bool showDepLine() const,
        showDestLine() const;
    QString routeWaypointsStr();
    QList<Waypoint*> routeWaypoints();
    QList<Waypoint*> routeWaypointsWithDepDest();
    void checkStatus(); // adjust label visibility from flight status

    QString planAircraft, planTAS, planDep, planAlt, planDest,
        planAltAirport, planRevision, planFlighttype, planDeptime,
        transponder, planRemarks, planRoute, planActtime, airline,
        planAltAirport2, planTypeOfFlight, // IVAO only
        qnhInHg, qnhMb, // VATSIM only
        routeWaypointsPlanDepCache, routeWaypointsPlanDestCache,
        routeWaypointsPlanRouteCache;
    QDate dayOfFlight;
    int altitude, groundspeed, planHrsEnroute, planMinEnroute,
        planHrsFuel, planMinFuel,
        pob; // IVAO only
    double trueHeading;
    bool onGround, // IVAO only
        showDepDestLine;
    QDateTime whazzupTime; // need some local reference to that
    QList<Waypoint*> routeWaypointsCache; // caching calculated routeWaypoints
};

#endif /*PILOT_H_*/
