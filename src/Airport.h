/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef AIRPORT_H_
#define AIRPORT_H_

#include "_pch.h"

#include "MapObject.h"
#include "Controller.h"
#include "Pilot.h"
#include "Metar.h"

class Pilot;

class Airport: public MapObject {
    public:
        Airport();
        Airport(const QStringList &list);
        ~Airport();

        virtual void showDetailsDialog();

        virtual QString toolTip() const;
        virtual QString mapLabel() const;
        virtual bool matches(const QRegExp &regex) const;

        void resetWhazzupStatus();

        bool active;

        QSet<Controller*> approaches, towers, grounds, deliveries;
        QSet<Pilot*> arrivals, departures;
        QSet<Controller*> allControllers() const;

        void addArrival(Pilot *client);
        void addDeparture(Pilot *client);
        int numFilteredArrivals, numFilteredDepartures;

        //void addCenter(Controller *client);
        void addApproach(Controller *client);
        void addTower(Controller *client);
        void addGround(Controller *client);
        void addDelivery(Controller *client);

        QString name, city, countryCode;

        bool showFlightLines;

        const GLuint &appDisplayList();
        const GLuint &appBorderDisplayList();
        const GLuint &twrDisplayList();
        const GLuint &gndDisplayList();
        const GLuint &delDisplayList();

        Metar metar;

    private:
        GLuint _appDisplayList, _appBorderDisplayList, _twrDisplayList,
        _gndDisplayList, _delDisplayList;
};

#endif /*AIRPORT_H_*/
