/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "BookedController.h"
#include "ControllerDetails.h"
#include "Window.h"
#include "NavData.h"
#include "Settings.h"

BookedController::BookedController(const QStringList& stringList, const WhazzupData* whazzup):
    Client(stringList, whazzup)
{
    sector = 0;

    // extra booking values
    bookingType = getField(stringList, 4).toInt();
    timeTo = getField(stringList, 14);
    date = getField(stringList, 16);
    //always some miracle "1" here: getField(stringList, 19); ??
    link = getField(stringList, 35);
    if(!link.isEmpty() && !link.contains("http://"))
        link = QString("http://%1").arg(link);
    timeFrom = getField(stringList, 37);

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

    // fixme: this does not seem to be the quickest method...
    else if (label.right(4) == "_DEL") {
        if (NavData::getInstance()->airports().contains(this->getDelivery())) {
            countryCode = NavData::getInstance()->airports()[this->getDelivery()]->countryCode;
            lat = NavData::getInstance()->airports()[this->getDelivery()]->lat;
            lon = NavData::getInstance()->airports()[this->getDelivery()]->lon;
            visualRange = 2;
        }
        facilityType = 3;
    }
    else if (label.right(4) == "_GND") {
        if (NavData::getInstance()->airports().contains(this->getGround())) {
            countryCode = NavData::getInstance()->airports()[this->getGround()]->countryCode;
            lat = NavData::getInstance()->airports()[this->getGround()]->lat;
            lon = NavData::getInstance()->airports()[this->getGround()]->lon;
            visualRange = 5;
        }
        facilityType = 3;
    }
    else if (label.right(4) == "_TWR") {
        facilityType = 4;
        if (NavData::getInstance()->airports().contains(this->getTower())) {
            countryCode = NavData::getInstance()->airports()[this->getTower()]->countryCode;
            lat = NavData::getInstance()->airports()[this->getTower()]->lat;
            lon = NavData::getInstance()->airports()[this->getTower()]->lon;
            visualRange = 15;
        }
    }
    else if (label.right(4) == "_APP" || label.right(4) == "_DEP") {
        facilityType = 5;
        if (NavData::getInstance()->airports().contains(this->getApproach())) {
            countryCode = NavData::getInstance()->airports()[this->getApproach()]->countryCode;
            lat = NavData::getInstance()->airports()[this->getApproach()]->lat;
            lon = NavData::getInstance()->airports()[this->getApproach()]->lon;
            visualRange = 35;
        }
    }
    else if (label.right(4) == "_CTR") {
        facilityType = 6;
        QString ctr = this->getCenter();
        if (NavData::getInstance()->sectors().contains(ctr)) {
            countryCode = NavData::getInstance()->sectors()[ctr]->countryCode();
            lat = NavData::getInstance()->sectors()[ctr]->lat();
            lon = NavData::getInstance()->sectors()[ctr]->lon();
            //visualRange = NavData::getInstance()->sectors()[ctr]->maxDistanceFromCenter();
        }
    }
    else if (label.right(4) == "_FSS") {
        facilityType = 7;
        QString ctr = this->getCenter();
        if (NavData::getInstance()->sectors().contains(ctr)) {
            countryCode = NavData::getInstance()->sectors()[ctr]->countryCode();
            lat = NavData::getInstance()->sectors()[ctr]->lat();
            lon = NavData::getInstance()->sectors()[ctr]->lon();
            //visualRange = NavData::getInstance()->sectors()[ctr]->maxDistanceFromCenter();
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

    QStringList segments = label.split('_');

    // allow only _FSS* and _CTR*
    if(!segments.last().startsWith("CTR") && !segments.last().startsWith("FSS"))
        return QString();
    segments.removeLast();

    // ignore _T* and _X* positions
    if(segments.last().startsWith("T_") || segments.last().startsWith("T1_")
            || segments.last().startsWith("T2_") || segments.last().startsWith("T3_") ||
            segments.last().startsWith("X"))
        return QString();

    // now create LOVV_N from LOVV and N, then return it
    QString result = segments.first();
    segments.removeFirst();
    while(!segments.isEmpty()) {
        result += "_" + segments.first();
        segments.removeFirst();
    }

    if(NavData::getInstance()->sectors().contains(result)) {
        Sector *f = NavData::getInstance()->sectors()[result];
        lat = f->lat(); // fix my coordinates so that user can find me on the map
        lon = f->lon();
    }
    return result;
}

QString BookedController::getApproach() const {
    if(!isATC())
        return QString();

    if(!couldBeAtcCallsign()) return QString();
    QStringList list = label.split('_');
    if(list.last().startsWith("APP") || list.last().startsWith("DEP")) {
        if(list.first().length() == 3)
            return "K" + list.first(); // VATSIMmers don't think ICAO codes are cool
        return list.first();
    }

    return QString();
}

QString BookedController::getTower() const {
    if(!isATC())
        return QString();

    if(!couldBeAtcCallsign()) return QString();
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
    if(list.size() > 3) return QString();
    if(list.size() == 3 &&
            (list[1].startsWith("X") || list[1].startsWith("T")))
        return QString();

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
    if(list.size() > 3) return QString();
    if(list.size() == 3 &&
            (list[1].startsWith("X") || list[1].startsWith("T")))
        return QString();

    if(list.last().startsWith("DEL")) {
        if(list.first().length() == 3)
            return "K" + list.first(); // VATSIMmers don't think ICAO codes are cool
        return list.first();
    }

    return QString();
}

bool BookedController::couldBeAtcCallsign() const {
    QStringList list = label.split('_');
    if(list.size() > 4 || list.size() <= 1) return false; // ignore XXXX_A_B_C_D_CTR and bogus
    if(list.size() == 3 && // ignore LOVV_T_CTR and LOVV_X_CTR
            (list[1].startsWith("X") || list[1].startsWith("T")))
        return false;
    if(list.size() == 4 && // ignore XXXX_X_N_CTR
            (list[2].startsWith("X") || list[2].startsWith("T")))
        return false;

    return true;
}

void BookedController::showDetailsDialog() {
    //not applicable
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
    QString r = rank();
    QString result = label + " (";
    if(!isObserver() && !frequency.isEmpty()) {
        result += frequency + ", ";
    }
    result += realName;
    if(!r.isEmpty()) result += ", " + r;
    result += ")";
    return result;
}

QString BookedController::mapLabel() const {
    if(label.endsWith("_CTR")) // hack to make _CTR labels smaller
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
