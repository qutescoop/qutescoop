/**************************************************************************
*  This file is part of QuteScoop. See README for license
**************************************************************************/

#include "BookedController.h"

#include <QJsonObject>

BookedController::BookedController(const QJsonObject& json, const WhazzupData* whazzup) :
    Client(json, whazzup)
{
    bookingType = json["type"].toString();
    timeTo = json["end"].toString();
    timeFrom = json["start"].toString();

    if(bookingType == "booking") {
        bookingInfoStr = QString();
    } else if (bookingType == "event") {
        bookingInfoStr = QString("Event");
    } else if (bookingType == "training") {
        bookingInfoStr = QString("Training");
    } else {
        bookingInfoStr = QString(bookingType);
    }

    if (label.right(5) == "_ATIS") {
        facilityType = 2;
    } else if (label.right(4) == "_DEL") {
        facilityType = 2;
    } else if (label.right(4) == "_GND") {
        facilityType = 3;
    } else if (label.right(4) == "_TWR") {
        facilityType = 4;
    } else if (label.right(4) == "_APP" || label.right(4) == "_DEP") {
        facilityType = 5;
    } else if (label.right(4) == "_CTR") {
        facilityType = 6;
    } else if (label.right(4) == "_FSS") {
        facilityType = 7;
    }

    timeConnected = starts();
}

QString BookedController::facilityString() const {
    switch (facilityType) {
        case 0: return "OBS";
        case 1: return "Staff";
        case 2: return "DEL";
        case 3: return "GND";
        case 4: return "TWR";
        case 5: return "APP";
        case 6: return "CTR";
        case 7: return "FSS";
    }
    return QString();
}

QDateTime BookedController::starts() const {
    return QDateTime::fromString(timeFrom, "yyyy-MM-dd HH:mm:ss");
}

QDateTime BookedController::ends() const {
    return QDateTime::fromString(timeTo, "yyyy-MM-dd HH:mm:ss");
}
