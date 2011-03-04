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

class Airport: public MapObject
{
public:
    Airport();
    Airport(const QStringList& list);
    ~Airport();

    virtual void showDetailsDialog();

    virtual QString toolTip() const;
    virtual QString mapLabel() const;
    virtual bool matches(const QRegExp& regex) const;

    void resetWhazzupStatus();

    bool isActive() { return active; }

    const QList<Controller*>& getApproaches() const { return approaches; }
    const QList<Controller*>& getTowers() const { return towers; }
    const QList<Controller*>& getGrounds() const { return grounds; }
    const QList<Controller*>& getDeliveries() const { return deliveries; }

    const QList<Pilot*>& getArrivals() const { return arrivals; }
    const QList<Pilot*>& getDepartures() const { return departures; }

    QList<Controller*> getAllControllers() const;

    void addArrival(Pilot* client);
    void addDeparture(Pilot* client);
    int numFilteredArrivals, numFilteredDepartures;

    //void addCenter(Controller* client);
    void addApproach(Controller* client);
    void addTower(Controller* client);
    void addGround(Controller* client);
    void addDelivery(Controller* client);

    QString name, city, countryCode;

    bool showFlightLines;

    const GLuint& getAppDisplayList();
    const GLuint& getAppBorderDisplayList();
    const GLuint& getTwrDisplayList();
    const GLuint& getGndDisplayList();
    const GLuint& getDelDisplayList();

    Metar metar;

private:
    bool active;
    //QList<Controller*> centers;
    QList<Controller*> approaches, towers, grounds, deliveries;

    QList<Pilot*> arrivals, departures;

    GLuint appDisplayList, appBorderDisplayList, twrDisplayList,
        gndDisplayList, delDisplayList;
};

#endif /*AIRPORT_H_*/
