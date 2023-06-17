#ifndef PILOT_H_
#define PILOT_H_

#include "Airline.h"
#include "Client.h"
#include "MapObject.h"
#include "Waypoint.h"

#include <QJsonDocument>

class Airport;

class Pilot
    : public MapObject, public Client {
    Q_OBJECT
    public:
        static const int taxiTimeOutbound = 240;
        static int altToFl(int alt_ft, int qnh_mb);
        static const QHash<QString, std::function<QString(Pilot*)> > placeholders;

        enum FlightStatus {
            BOARDING, GROUND_DEP, DEPARTING, EN_ROUTE, ARRIVING,
            GROUND_ARR, BLOCKED, CRASHED, BUSH, PREFILED
        };

        Pilot(const QJsonObject& json, const WhazzupData* whazzup);
        virtual ~Pilot();

        virtual QString toolTip() const override;
        virtual QString rank() const override;
        virtual QString mapLabel() const override;
        virtual QString mapLabelHovered() const override;
        virtual QStringList mapLabelSecondaryLines() const override;
        virtual QStringList mapLabelSecondaryLinesHovered() const override;
        virtual QString livestreamString(bool shortened = false) const override;
        virtual bool hasPrimaryAction() const override;
        virtual void primaryAction() override;

        void showDetailsDialog();

        FlightStatus flightStatus() const;
        QString flightStatusString() const;
        QString flightStatusShortString() const;
        QString planFlighttypeString() const;
        QString aircraftType() const;
        Airport* depAirport() const;
        Airport* destAirport() const;
        Airport* altAirport() const;
        QStringList waypoints() const;
        double distanceToDestination() const;
        double distanceFromDeparture() const;
        QDateTime etd() const; // Estimated Time of Departure
        QDateTime eta() const; // Estimated Time of Arrival
        //QDateTime fixedEta; // ETA, written after creation as a workaround for Prediction (Warp) Mode
        QTime eet() const; // Estimated Enroute Time as remaining time to destination
        QDateTime etaPlan() const; // Estimated Time of Arrival as flightplanned
        QString delayString() const;
        int planTasInt() const; // defuck TAS for Mach numbers
        int defuckPlanAlt(QString alt) const; // returns an altitude from various flightplan strings
        QString humanAlt() const; // altitude as string, prefixed with FL if applicable
        QString flOrEmpty() const; // altitude prefixed with F
        QPair<double, double> positionInFuture(int seconds) const;
        int nextPointOnRoute(const QList<Waypoint*> &waypoints) const;
        QString routeWaypointsString();
        QList<Waypoint*> routeWaypoints();
        QList<Waypoint*> routeWaypointsWithDepDest();
        void checkStatus(); // adjust label visibility from flight status

        QString planAircraft, planAircraftFaa, planAircraftFull,
            planTAS, planDep, planAlt, planDest,
            planAltAirport, planRevision, planFlighttype, planDeptime,
            transponder, transponderAssigned, planRemarks, planRoute, planActtime,
            routeWaypointsPlanDepCache, routeWaypointsPlanDestCache,
            routeWaypointsPlanRouteCache;
        QDate dayOfFlight;
        int altitude, groundspeed, planEnroute_hrs, planEnroute_mins,
            planFuel_hrs, planFuel_mins,
            qnh_mb;
        int pilotRating = -99, militaryRating = -99;
        double trueHeading, qnh_inHg;
        bool showRoute = false;
        QDateTime whazzupTime; // need some local reference to that
        QList<Waypoint*> routeWaypointsCache; // caching calculated routeWaypoints
        Airline* airline;
    private:
};

#endif /*PILOT_H_*/
