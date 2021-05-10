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
        QString mapLabel() const;
        bool matches(const QRegExp& regex) const;

        bool isObserver() const { return facilityType == 0; }
        bool isATC() const { return facilityType > 0; } // facilityType = 1 is reported for FSS stations (at least from VATSIM)
        QString rank() const;

        QString getCenter() const;
        QString getApproach() const;
        QString getTower() const;
        QString getGround() const;
        QString getDelivery() const;

        Airport *airport() const;

        QString frequency, atisMessage;
        int facilityType, visualRange;
        QDateTime timeLastAtisReceived, assumeOnlineUntil;

        QString voiceChannel;
        QString voiceLink() const;

        Sector *sector;

    private:
};

#endif /*CONTROLLER_H_*/
