/**************************************************************************
*  This file is part of QuteScoop. See README for license
**************************************************************************/

#include "BookedController.h"

#include <QJsonObject>

BookedController::BookedController(const QJsonObject& json, const WhazzupData* whazzup) :
    Client(json, whazzup)
{
    bookingType = json["bookingType"].toInt();
    timeTo = json["timeTo"].toString();
    date = json["date"].toString();

    link = json["link"].toString();
    if (!link.isEmpty() && !link.contains("http://")) {
        link = QString("http://%1").arg(link);
    }
    timeFrom = json["timeFrom"].toString();

    // computed values
    switch (bookingType) {
        case 0:  bookingInfoStr = QString(); break;
        case 1:  bookingInfoStr = QString("Event"); break;
        case 10: bookingInfoStr = QString("Training"); break;
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
    return QDateTime(
        QDate::fromString(date, QString("yyyyMMdd")),
        QTime::fromString(timeFrom, QString("HHmm")),
        Qt::UTC
    );
}

QDateTime BookedController::ends() const {
    return QDateTime(
        QDate::fromString(date, QString("yyyyMMdd")),
        QTime::fromString(timeTo, QString("HHmm")),
        Qt::UTC
    );
}
