/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "BookedController.h"
#include "ControllerDetails.h"
#include "Window.h"
#include "NavData.h"
#include "Settings.h"

#include <QJsonObject>

BookedController::BookedController(const QJsonObject& json, const WhazzupData* whazzup):
        Client(json, whazzup) {
    sector = 0;

    // extra booking values
    bookingType = json["bookingType"].toInt();
    timeTo = json["timeTo"].toString();
    date = json["date"].toString();
    //always some miracle "1" here: getField(stringList, 19); ??
    link = json["link"].toString();
    if(!link.isEmpty() && !link.contains("http://"))
        link = QString("http://%1").arg(link);
    timeFrom = json["timeFrom"].toString();

    // default
    lat = 0.0; lon = 0.0;
    visualRange = 0;

    // computed values
    switch (bookingType) {
        case 0:  bookingInfoStr = QString(); break;
        case 1:  bookingInfoStr = QString("Event"); break;
        case 10: bookingInfoStr = QString("Training"); break;
    }
    if (label.right(5) == "_ATIS") {
        facilityType = 2; // dont know who wants to book it, but well...
    } else if (label.right(4) == "_DEL") {
        facilityType = 3;
        if (NavData::instance()->airports.contains(this->getDelivery())) {
            lat = NavData::instance()->airports[this->getDelivery()]->lat;
            lon = NavData::instance()->airports[this->getDelivery()]->lon;
            visualRange = 2;
        }
    } else if (label.right(4) == "_GND") {
        facilityType = 3;
        if (NavData::instance()->airports.contains(this->getGround())) {
            lat = NavData::instance()->airports[this->getGround()]->lat;
            lon = NavData::instance()->airports[this->getGround()]->lon;
            visualRange = 5;
        }
    } else if (label.right(4) == "_TWR") {
        facilityType = 4;
        if (NavData::instance()->airports.contains(this->getTower())) {
            lat = NavData::instance()->airports[this->getTower()]->lat;
            lon = NavData::instance()->airports[this->getTower()]->lon;
            visualRange = 15;
        }
    } else if (label.right(4) == "_APP" || label.right(4) == "_DEP") {
        facilityType = 5;
        if (NavData::instance()->airports.contains(this->getApproach())) {
            lat = NavData::instance()->airports[this->getApproach()]->lat;
            lon = NavData::instance()->airports[this->getApproach()]->lon;
            visualRange = 35;
        }
    } else if (label.right(4) == "_CTR") {
        facilityType = 6;
        QString ctr = this->getCenter();
        if (NavData::instance()->sectors.contains(ctr)) {
            QPair<double, double> sectorCenter = NavData::instance()->sectors[ctr]->getCenter();
            lat = sectorCenter.first;
            lon = sectorCenter.second;
            visualRange = 150;
        }
    } else if (label.right(4) == "_FSS") {
        facilityType = 7;
        QString ctr = this->getCenter();
        if (NavData::instance()->sectors.contains(ctr)) {
            QPair<double, double> sectorCenter = NavData::instance()->sectors[ctr]->getCenter();
            lat = sectorCenter.first;
            lon = sectorCenter.second;
            visualRange = 500;
        }
    }

    timeConnected = starts();

    //some unapplicable data for a booked controller, but we might need it once to cast BookedController -> Controller
    frequency = "";
    atisMessage = "";
    server = "";
}

QString BookedController::facilityString() const {
    switch(facilityType) {
    case 0: return "Observer";
    case 1: return "Staff";
    case 2: return "ATIS";
    case 3: return "Ground";
    case 4: return "Tower";
    case 5: return "App/Dep";
    case 6: return "Center";
    case 7: return "FSS";
    }
    return QString();
}

QString BookedController::getCenter() {
    if(!isATC())
        return QString();
    QStringList list = label.split('_');

    // allow only _FSS* and _CTR*
    if(list.last().startsWith("CTR") || list.last().startsWith("FSS")) {
        list.removeLast();
        QString result = list.join("_");
        if(NavData::instance()->sectors.contains(result)) {
            QPair<double, double> sectorCenter = NavData::instance()->sectors[result]->getCenter();
            lat = sectorCenter.first;
            lon = sectorCenter.second;
        }
        return result;
    }
    return QString();
}

QString BookedController::getApproach() const {
    if(!isATC())
        return QString();
    QStringList list = label.split('_');
    if(list.last().startsWith("APP") || list.last().startsWith("DEP")) {
        // map special callsigns to airports. Still not perfect, because only 1 airport gets matched this way...
        if(list.first() == "EDBB")
            return "EDDB"; // map EDBB -> EDDB (no other active airfields covered by this sector)
        if(list.first() == "NY")
            return "KLGA"; // map NY -> KLGA
        if(list.first() == "MSK")
            return "UUWW"; // map MSK -> UUWW

        // VATSIMmers don't think ICAO codes are cool
        if(list.first().length() == 3)
            return "K" + list.first();
        return list.first();
    }
    return QString();
}

QString BookedController::getTower() const {
    if(!isATC())
        return QString();
    QStringList list = label.split('_');
    if(list.last().startsWith("TWR")) {
        if(list.first().length() == 3)
            return "K" + list.first(); // VATSIMmers don't think ICAO codes are cool
        return list.first();
    }
    return QString();
}

QString BookedController::getGround() const {
    if(!isATC())
        return QString();
    QStringList list = label.split('_');
    if(list.last().startsWith("GND")) {
        if(list.first().length() == 3)
            return "K" + list.first(); // VATSIMmers don't think ICAO codes are cool
        return list.first();
    }
    return QString();
}

QString BookedController::getDelivery() const {
    if(!isATC())
        return QString();
    QStringList list = label.split('_');
    if(list.last().startsWith("DEL")) {
        if(list.first().length() == 3)
            return "K" + list.first(); // VATSIMmers don't think ICAO codes are cool
        return list.first();
    }

    return QString();
}

QString BookedController::rank() const {
    switch(rating) {
        case 0: return QString();
        case 1: return QString("OBS");
        case 2: return QString("S1");
        case 3: return QString("S2");
        case 4: return QString("S3");
        case 5: return QString("C1");
        case 6: return QString("C2");
        case 7: return QString("C3");
        case 8: return QString("I1");
        case 9: return QString("I2");
        case 10: return QString("I3");
        case 11: return QString("SUP");
        case 12: return QString("ADM");
        default: return QString("unknown:%1").arg(rating);
    }
}

QString BookedController::toolTip() const {
    QString result = label;
    if (sector != 0)
        result += " [" + sector->name + "]";
    result += " (";
    if(!isObserver() && !frequency.isEmpty())
        result += frequency + ", ";
    result += realName();
    if(!rank().isEmpty())
        result += ", " + rank();
    result += ")";
    return result;
}

QString BookedController::mapLabel() const {
    if(label.endsWith("_CTR") || label.endsWith("_FSS"))
        return label.left(label.length() - 4);
    return label;
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
