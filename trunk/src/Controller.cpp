/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Controller.h"

#include "_pch.h"

#include "Client.h"
#include "ControllerDetails.h"
#include "NavData.h"
#include "Settings.h"
#include "helpers.h"


Controller::Controller(const QStringList& stringList, const WhazzupData* whazzup):
        Client(stringList, whazzup),
        sector(0)
{
    frequency = field(stringList, 4);
    facilityType = field(stringList, 18).toInt();
    if(label.right(4) == "_FSS") facilityType = 7; // workaround as VATSIM reports 1 for _FSS

    visualRange = field(stringList, 19).toInt();
    atisMessage = field(stringList, 35);
    timeLastAtisReceived = QDateTime::fromString(field(stringList, 36), "yyyyMMddHHmmss");

    QStringList atisLines = atisMessage.split(QString::fromUtf8("^§")); // needed due to source encoded in UTF8 -
                                                                        // found after some headache...

    if(network== Client::IVAO) {
        voiceChannel = atisLines.first();
        atisLines.removeFirst();
    }


    atisMessage = "";
    while (!atisLines.isEmpty()) {
        QString line = atisLines.takeFirst();
        if (line.startsWith("$ "))
            voiceChannel = line.mid(2);
        else
            atisMessage += line + "<br>";
    }

    // do some magic for Controller Info like "online until"...
    QRegExp rxOnlineUntil = QRegExp(
            "(open|close|online|offline|till|until)(\\W*\\w*\\W*){0,4}\\b(\\d{1,2}):?(\\d{2})\\W?(z|utc)?", Qt::CaseInsensitive);
    if (rxOnlineUntil.indexIn(atisMessage) > 0) {
        //fixme
        QTime found = QTime::fromString(rxOnlineUntil.cap(3)+rxOnlineUntil.cap(4), "HHmm");
        if(found.isValid()) {
            if (qAbs(found.secsTo(whazzup->whazzupTime.time())) > 60*60 * 12) // e.g. now its 2200z, and he says
                                                                //"online until 0030z", allow for up to 12 hours
                assumeOnlineUntil = QDateTime(whazzup->whazzupTime.date().addDays(1), found, Qt::UTC);
            else
                assumeOnlineUntil = QDateTime(whazzup->whazzupTime.date(), found, Qt::UTC);
        }
    }

    QString icao = this->getCenter();
    if (!icao.isEmpty()) {
        while(!NavData::instance()->sectors.contains(icao) && !icao.isEmpty()) {
            int p = icao.lastIndexOf('_');
            if(p == -1) {
                qDebug() << "Unknown sector/FIR\t" << icao << "\tPlease provide sector information if you can";
                if (visualRange == 0 || (qFuzzyIsNull(lat) && qFuzzyIsNull(lon))) {
                    icao = "";
                    continue;
                }
                Sector *s = new Sector(); // creating a round sector with 0.5 * visualRange radius
                //s->lat = lat; s->lon = lon; // position label on primary visibility center
                s->icao = icao;
                s->name = "no sector data";
                for(float u = 0.; u < 2. * M_PI; u += M_PI / 24.) { // 48 segments
                    s->points.append(QPair<double, double>(lat + qCos(u) * visualRange / 2. / 60.,
                                                           lon + qSin(u) * visualRange / 2. / 60. /
                                                                qCos(qAbs(lat) * Pi180)));
                }
                NavData::instance()->sectors.insert(icao, s); // adding to the pool
            } else
                icao = icao.left(p);
        }
        if(NavData::instance()->sectors.contains(icao) && !icao.isEmpty())
            this->sector = NavData::instance()->sectors[icao];
    }
}

QString Controller::facilityString() const {
    switch(facilityType) {
    case 0: return "OBS";
    case 1: return "Staff";
    case 2: return  network == VATSIM? "DEL": "ATIS";
    case 3: return "GND";
    case 4: return "TWR";
    case 5: return "APP";
    case 6: return "CTR";
    case 7: return "FSS";
    }
    return QString();
}

QString Controller::getCenter() const{
    if(!isATC())
        return QString();
    QStringList list = label.split('_');

    // allow only _FSS* and _CTR*
    if(list.last().startsWith("CTR") || list.last().startsWith("FSS")) {
        list.removeLast();
        return list.join("_");
    }
    return QString();
}

QString Controller::getApproach() const {
    if(!isATC())
        return QString();
    QStringList list = label.split('_');
    if(list.last().startsWith("APP") || list.last().startsWith("DEP")) {
        if(list.first().length() == 3)
            return "K" + list.first(); // VATSIMmers don't think ICAO codes are cool

        // map special callsigns to airports. Still not perfect, because only 1 airport gets matched this way...
        if(list.first() == "EDBB")
            return "EDDI"; // map EDBB -> EDDI
        if(list.first() == "NY")
            return "KLGA"; // map NY -> KLGA
        return list.first();
    }
    return QString();
}

QString Controller::getTower() const {
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

QString Controller::getGround() const {
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

QString Controller::getDelivery() const {
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

Airport *Controller::airport() const {
    QString tryAirport;
    if (!label.split("_").isEmpty())
        tryAirport = label.split("_").first();
    if (tryAirport.size() == 3)
        tryAirport = "K" + tryAirport;
    if(NavData::instance()->airports.contains(tryAirport))
        return NavData::instance()->airports[tryAirport];
    else return 0;
}

void Controller::showDetailsDialog() {
    ControllerDetails *infoDialog = ControllerDetails::instance(true);
    infoDialog->refresh(this);
    infoDialog->show();
    infoDialog->raise();
    infoDialog->activateWindow();
    infoDialog->setFocus();
}

QString Controller::rank() const {
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
        case 2: return QString("AS1"); //ATC Applicant
        case 3: return QString("AS2"); //ATC Trainee
        case 4: return QString("AS3"); //Advanced ATC Trainee
        case 5: return QString("ADC"); //Aerodrome Controller
        case 6: return QString("APC"); //Approach Controller
        case 7: return QString("ACC"); //Center Controller
        case 8: return QString("SEC"); //Senior Controller (not available)
        case 9: return QString("SAI"); //Senior ATC Instructor
        case 10: return QString("CAI"); //Chief ATC Instructor
        default: return QString("unknown:%1").arg(rating);
        }
    }
}

QString Controller::toolTip() const { // LOVV_CTR [Vienna] (134.350, Name, C1)
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

QString Controller::mapLabel() const { // LOVV
    if(label.endsWith("_CTR") || label.endsWith("_FSS"))
        return label.left(label.length() - 4);
    return label;
}

bool Controller::matches(const QRegExp& regex) const {
    if (frequency.contains(regex)) return true;
    if (atisMessage.contains(regex)) return true;
    if(realName.contains(regex)) return true;
    if (sector != 0)
        if (sector->name.contains(regex)) return true;
    return MapObject::matches(regex);
}

QString Controller::voiceLink() const {
    if (Settings::voiceType() == Settings::TEAMSPEAK) {
        QStringList serverChannel = voiceChannel.split("/");
        return QString("teamspeak://%1?nickname=%2?loginname=%3?password=%4?channel=%5")
            .arg(serverChannel.first())
            .arg(Settings::voiceCallsign())
            .arg(Settings::voiceUser()).arg(Settings::voicePassword())
            .arg(serverChannel.last());
    } else
        return QString();
}
