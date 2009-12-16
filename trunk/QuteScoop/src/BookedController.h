/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
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
