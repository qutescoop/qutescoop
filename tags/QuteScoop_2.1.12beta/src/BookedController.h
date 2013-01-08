/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef BOOKEDCONTROLLER_H_
#define BOOKEDCONTROLLER_H_

#include "Client.h"
#include "WhazzupData.h"
#include "ClientDetails.h"

class WhazzupData;
class Sector;

class BookedController: public Client {
    public:
        BookedController(const QStringList& stringList, const WhazzupData* whazzup);

        bool isObserver() const { return facilityType == 0; }
        bool isATC() const { return facilityType > 0; } // facilityType = 1 is reported for FSS stations and staff (at least from VATSIM)

        virtual QString toolTip() const;
        virtual QString mapLabel() const;
        virtual QString rank() const;
        virtual void showDetailsDialog() {} // not applicable

        QString facilityString() const;
        QString getCenter(); // not const as we assign lat, lon
        QString getApproach() const;
        QString getTower() const;
        QString getGround() const;
        QString getDelivery() const;

        QString frequency, atisMessage;
        int facilityType, visualRange;
        QDateTime timeLastAtisReceived;

        QString voiceServer;
        QString voiceLink() const { return QString(); }

        // Booking values
        QString countryCode, link, bookingInfoStr, timeFrom, timeTo, date, eventLink;
        int bookingType;

        QDateTime starts() const;
        QDateTime ends() const;

        Sector *sector;

    private:
};

#endif /*BOOKEDCONTROLLER_H_*/
