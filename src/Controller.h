/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "Client.h"
#include "WhazzupData.h"
#include "ClientDetails.h"
#include "Sector.h"
#include "Airport.h"

#include <QJsonObject>

class Airport;

class Controller: public Client {
    public:
        Controller(const QJsonObject& json, const WhazzupData* whazzup);

        virtual void showDetailsDialog();

        QString facilityString() const;
        QString toolTip() const;
        QString toolTipShort() const;
        QString mapLabel() const;
        bool matches(const QRegExp& regex) const;

        bool isObserver() const;
        bool isATC() const; // facilityType = 1 is reported for FSS stations (at least from VATSIM)
        QString rank() const;

        QString getSectorName() const;
        QString getApproach() const;
        QString getTower() const;
        QString getGround() const;
        QString getDelivery() const;
        QString getAtis() const;

        Airport *airport() const;

        QString frequency, atisMessage, atisCode;
        int facilityType, visualRange;
        QDateTime assumeOnlineUntil;

        Sector *sector;

    private:
};

#endif /*CONTROLLER_H_*/
