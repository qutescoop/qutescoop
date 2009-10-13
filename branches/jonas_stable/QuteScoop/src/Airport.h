/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
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

#ifndef AIRPORT_H_
#define AIRPORT_H_

#include <QStringList>
#include <QGLWidget>

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

    virtual QString toolTip() const;
    virtual void showDetailsDialog();
    virtual QString mapLabel() const;

    virtual bool matches(const QRegExp& regex) const;

    QString name;
    QString city;
    QString countryCode;

    void resetWhazzupStatus();

    bool isActive() { return active; };

    const QList<Controller*>& getApproaches() const { return approaches; };
    const QList<Controller*>& getTowers() const { return towers; };
    const QList<Controller*>& getGrounds() const { return grounds; };
    const QList<Controller*>& getDeliveries() const { return deliveries; };

    const QList<Pilot*>& getArrivals() const { return arrivals; };
    const QList<Pilot*>& getDepartures() const { return departures; };

    int numFilteredArrivals() const;
    int numFilteredDepartures() const;

    QList<Controller*> getAllControllers() const;

    void addArrival(Pilot* client);
    void addDeparture(Pilot* client);

    void addCenter(Controller* client);
    void addApproach(Controller* client);
    void addTower(Controller* client);
    void addGround(Controller* client);
    void addDelivery(Controller* client);

    void toggleFlightLines();
    void setDisplayFlightLines(bool show);
    bool showFlightLines;
    void refreshAfterUpdate();

    const GLuint& getAppDisplayList();
    const GLuint& getAppBorderDisplayList();
    const GLuint& getTwrDisplayList();
    const GLuint& getGndDisplayList();
    const GLuint& getDelDisplayList();

    Metar metar;

private:
    void createAppDisplayLists();

    bool active;
    QList<Controller*> centers;
    QList<Controller*> approaches;
    QList<Controller*> towers;
    QList<Controller*> grounds;
    QList<Controller*> deliveries;

    QList<Pilot*> arrivals;
    QList<Pilot*> departures;

    GLuint appDisplayList;
    GLuint appBorderDisplayList;
    GLuint twrDisplayList;
    GLuint gndDisplayList;
    GLuint delDisplayList;
};

#endif /*AIRPORT_H_*/
