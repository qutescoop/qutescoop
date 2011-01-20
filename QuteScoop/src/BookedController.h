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

class BookedController: public Client
{
public:
	BookedController(const QStringList& stringList, const WhazzupData* whazzup);

	void showDetailsDialog();
	bool isObserver() const { return facilityType == 0; }
    bool isATC() const { return facilityType > 0; } // facilityType = 1 is reported for FSS stations and staff (at least from VATSIM)

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

	QString voiceServer;
	QString voiceLink() const { return QString(); }
    
    // Booking values
    QString countryCode;
    QString link;
    int bookingType;
    QString bookingInfoStr;

    QString timeFrom;
    QString timeTo;
    QString date;
    QString eventLink;
    
    QDateTime starts() const;
    QDateTime ends() const;
    
    Sector *sector;

private:
	bool couldBeAtcCallsign() const;
};

#endif /*BOOKEDCONTROLLER_H_*/
