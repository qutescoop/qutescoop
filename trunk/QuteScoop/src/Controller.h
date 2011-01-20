/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "Client.h"
#include "WhazzupData.h"
#include "ClientDetails.h"

class WhazzupData;
class Sector;

class Controller: public Client
{
public:
    Controller(const QStringList& stringList, const WhazzupData* whazzup);

    void showDetailsDialog();
    bool isObserver() const { return facilityType == 0; }
    bool isATC() const { return facilityType > 0; } // facilityType = 1 is reported for FSS stations (at least from VATSIM)

    QString toolTip() const;

    QString facilityString() const;

    virtual QString mapLabel() const;

    virtual QString rank() const;

    QString getCenter();
    QString getApproach() const;
    QString getTower() const;
    QString getGround() const;
    QString getDelivery() const;

    QString frequency;
    int facilityType;
    int visualRange;
    QString atisMessage;
    QDateTime timeLastAtisReceived;
    QDateTime assumeOnlineUntil;

    QString voiceServer;
    QString voiceLink() const;

    Sector *sector;

private:
    bool couldBeAtcCallsign() const;
};

#endif /*CONTROLLER_H_*/
