/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "BookedController.h"
#include "ControllerDetails.h"
#include "Window.h"
#include "NavData.h"
#include "Settings.h"

BookedController::BookedController(const QStringList& stringList, const WhazzupData* whazzup):
        Client(stringList, whazzup) {
    sector = 0;

    // extra booking values
    bookingType = field(stringList, 4).toInt();
    timeTo = field(stringList, 14);
    date = field(stringList, 16);
    //always some miracle "1" here: getField(stringList, 19); ??
    link = field(stringList, 35);
    if(!link.isEmpty() && !link.contains("http://"))
        link = QString("http://%1").arg(link);
    timeFrom = field(stringList, 37);

    // default
    countryCode = "";
    lat = 0.0; lon = 0.0;
    visualRange = 0;

    // computed values
    switch (bookingType) {
        case 0:  bookingInfoStr = QString(); break;
        case 1:  bookingInfoStr = QString("Event"); break;
        case 10: bookingInfoStr = QString("Training"); break;
    }
    if (label.right(5) == "_ATIS") facilityType = 2; // dont know who wants to book it, but well...

    else if (label.right(4) == "_DEL") {
        if (NavData::instance()->airports.contains(this->getDelivery())) {
            countryCode = NavData::instance()->airports[this->getDelivery()]->countryCode;
            lat = NavData::instance()->airports[this->getDelivery()]->lat;
            lon = NavData::instance()->airports[this->getDelivery()]->lon;
            visualRange = 2;
        }
        facilityType = 3;
    }
    else if (label.right(4) == "_GND") {
        if (NavData::instance()->airports.contains(this->getGround())) {
            countryCode = NavData::instance()->airports[this->getGround()]->countryCode;
            lat = NavData::instance()->airports[this->getGround()]->lat;
            lon = NavData::instance()->airports[this->getGround()]->lon;
            visualRange = 5;
        }
        facilityType = 3;
    }
    else if (label.right(4) == "_TWR") {
        facilityType = 4;
        if (NavData::instance()->airports.contains(this->getTower())) {
            countryCode = NavData::instance()->airports[this->getTower()]->countryCode;
            lat = NavData::instance()->airports[this->getTower()]->lat;
            lon = NavData::instance()->airports[this->getTower()]->lon;
            visualRange = 15;
        }
    }
    else if (label.right(4) == "_APP" || label.right(4) == "_DEP") {
        facilityType = 5;
        if (NavData::instance()->airports.contains(this->getApproach())) {
            countryCode = NavData::instance()->airports[this->getApproach()]->countryCode;
            lat = NavData::instance()->airports[this->getApproach()]->lat;
            lon = NavData::instance()->airports[this->getApproach()]->lon;
            visualRange = 35;
        }
    }
    else if (label.right(4) == "_CTR") {
        facilityType = 6;
        QString ctr = this->getCenter();
        if (NavData::instance()->sectors.contains(ctr)) {
            countryCode = NavData::instance()->sectors[ctr]->countryCode;
            lat = NavData::instance()->sectors[ctr]->lat;
            lon = NavData::instance()->sectors[ctr]->lon;
            visualRange = 150; // NavData::instance()->sectors[ctr]->maxDistanceFromCenter();
        }
    }
    else if (label.right(4) == "_FSS") {
        facilityType = 7;
        QString ctr = this->getCenter();
        if (NavData::instance()->sectors.contains(ctr)) {
            countryCode = NavData::instance()->sectors[ctr]->countryCode;
            lat = NavData::instance()->sectors[ctr]->lat;
            lon = NavData::instance()->sectors[ctr]->lon;
            visualRange = 500; // NavData::instance()->sectors[ctr]->maxDistanceFromCenter();
        }
    }

    timeConnected = starts();

    //some unapplicable data for a booked controller, but we might need it once to cast BookedController -> Controller
    frequency = "";
    atisMessage = "";
    timeLastAtisReceived = QDateTime();
    voiceServer = "";
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
            lat = NavData::instance()->sectors[result]->lat; // fix my coordinates
            lon = NavData::instance()->sectors[result]->lon;
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
            return "EDDI"; // map EDBB -> EDDI
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
    if(network == VATSIM) {
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
    } else {
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
        default: return QString("unknown:%1").arg(rating);
        }
    }
}

QString BookedController::toolTip() const {
    QString result = label;
    if (sector != 0)
        result += " [" + sector->name + "]";
    result += " (";
    if(!isObserver() && !frequency.isEmpty())
        result += frequency + ", ";
    result += realName;
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
