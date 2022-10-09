/**************************************************************************
*  This file is part of QuteScoop. See README for license
**************************************************************************/

#ifndef BOOKEDCONTROLLER_H_
#define BOOKEDCONTROLLER_H_

#include "Client.h"
#include "WhazzupData.h"

#include <QJsonObject>

class WhazzupData;
class Sector;

/*
 * @todo:
 * This is not a "Client" in the sense of a VATSIM client. This is rather a Dto to initialize
 * actual "Controller"s from.
 */
class BookedController: public Client {
public:
    BookedController(const QJsonObject& json, const WhazzupData* whazzup);

    QString facilityString() const;

    int facilityType;

    // Booking values
    QString link, bookingInfoStr, timeFrom, timeTo, date, eventLink;
    int bookingType;

    QDateTime starts() const;
    QDateTime ends() const;
private:
};

#endif /*BOOKEDCONTROLLER_H_*/
