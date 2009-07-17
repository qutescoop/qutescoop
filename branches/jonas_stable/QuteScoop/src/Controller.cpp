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

#include "Controller.h"
#include "ControllerDetails.h"
#include "Window.h"
#include "NavData.h"
#include "Settings.h"

Controller::Controller(const QStringList& stringList, const WhazzupData* whazzup):
    Client(stringList, whazzup)
{
    frequency = getField(stringList, 4);
    facilityType = getField(stringList, 18).toInt();
    if(label.right(4) == "_FSS") facilityType = 7; // workaround as VATSIM reports 1 for _FSS

    visualRange = getField(stringList, 19).toInt();
    atisMessage = getField(stringList, 35);
    timeLastAtisReceived = QDateTime::fromString(getField(stringList, 36), "yyyyMMddhhmmss");

    QStringList atisLines = atisMessage.split(QString::fromUtf8("^§")); // needed due to source encoded in UTF8 - found after some headache...
    if(atisLines.size() >= 1) {
        voiceServer = atisLines[0];
        QString atis = "";
        for(int i = 1; i < atisLines.size(); i++) {
            if(i > 1) atis += "<br>";
            atis += atisLines[i];
        }
        atisMessage = atis;
    }

    // do some magic for Controller Info like "online until"...
    QRegExp rxOnlineUntil = QRegExp("(open|close|online|offline)(\\W*\\w*\\W*){0,4}\\b(\\d{1,2}):?(\\d{2})\\W?(z|utc)", Qt::CaseInsensitive);
    if (rxOnlineUntil.indexIn(atisMessage) > 0) {
        //fixme
        QTime found = QTime::fromString(rxOnlineUntil.cap(3)+rxOnlineUntil.cap(4), "hhmm");
        if(found.isValid()) {
            if (qAbs(found.secsTo(whazzup->timestamp().time())) > 60*60 * 12) // e.g. now its 2200z, and he says "online until 0030z", allow for up to 12 hours
                assumeOnlineUntil = QDateTime(whazzup->timestamp().date().addDays(1), found, Qt::UTC);
            else
                assumeOnlineUntil = QDateTime(whazzup->timestamp().date(), found, Qt::UTC);
        }
        qDebug() << "Found" << label << "to be online until" << assumeOnlineUntil << "(Controller Info)";
    }


    QHash<QString, Fir*> firs = NavData::getInstance()->firs();
    QString icao = this->getCenter();
    fir = 0; // make this bulletproof as we get crashes around here
    if(icao.isNull() || icao.isEmpty()) {
        fir = 0;
    } else {
        while(!firs.contains(icao) && !icao.isEmpty()) {
            int p = icao.lastIndexOf('_');
            if(p == -1) {
                qDebug() << "Unknown FIR\t" << icao << "\tPlease provide sector information if you can";
                icao = "";
                continue;
            }
            else {
                icao = icao.left(p);
            }
        }
        if(!icao.isEmpty() && firs.contains(icao)) {
            this->fir = firs[icao];
        }
    }
}

QString Controller::facilityString() const {
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

QString Controller::getCenter() {
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
    return result;
}

QString Controller::getApproach() const {
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

QString Controller::getTower() const {
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

QString Controller::getGround() const {
    if(!isATC())
        return QString();

    QStringList list = label.split('_');
    if(list.size() > 3) return QString();
    if(list.size() == 3 &&
            (list[1].startsWith("X") || list[1].startsWith("T")))
        return QString();

    if(list.last().startsWith("GND") || list.last().startsWith("DEL")) {
        if(list.first().length() == 3)
            return "K" + list.first(); // VATSIMmers don't think ICAO codes are cool
        return list.first();
    }

    return QString();
}

bool Controller::couldBeAtcCallsign() const {
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

void Controller::showDetailsDialog() {
    ControllerDetails *infoDialog = ControllerDetails::getInstance();

    infoDialog->refresh(this);
    infoDialog->show();
    infoDialog->raise();
    infoDialog->activateWindow();
    infoDialog->setFocus();
}

QString Controller::rank() const {
    if(network == VATSIM) {
        switch(rating) {
        case 1: return QString("OBS"); break;
        case 2: return QString("S1"); break;
        case 3: return QString("S2"); break; // will be re-introduced in 2009/08
        case 4: return QString("S3"); break;
        case 5: return QString("C1"); break;
        case 6: return QString("C2"); break;
        case 7: return QString("C3"); break;
        case 8: return QString("I1"); break;
        case 9: return QString("I2"); break;
        case 10: return QString("I3"); break;
        case 11: return QString("SUP"); break;
        case 12: return QString("ADM"); break;
        default: return QString("unknown:%1").arg(rating); break;
        }
    } else {
        switch(rating) {
        case 1: return QString("OBS"); break;
        case 2: return QString("S1"); break;
        case 3: return QString("S2"); break;
        case 4: return QString("S3"); break;
        case 5: return QString("C1"); break;
        case 6: return QString("C2"); break;
        case 7: return QString("C3"); break;
        case 8: return QString("I1"); break;
        case 9: return QString("I2"); break;
        case 10: return QString("I3"); break;
        default: return QString("unknown:%1").arg(rating); break;
        }
    }
}

QString Controller::toolTip() const {
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

QString Controller::mapLabel() const {
    if(label.endsWith("_CTR")) // hack to make _CTR labels smaller
        return label.left(label.length() - 4);
    return label;
}

QString Controller::voiceLink() const {
    switch(Settings::voiceType()) {
        case Settings::TEAMSPEAK: {
            QStringList serverChannel = voiceServer.split('/');
            return QString("teamspeak://%1?nickname=%2?loginname=%3?password=%4?channel=%5")
                .arg(serverChannel.first())
                .arg(Settings::voiceCallsign())
                .arg(Settings::voiceUser()).arg(Settings::voicePassword())
                .arg(serverChannel.last());
        }

        case Settings::VRC:
            // insert something useful here - I dont know how vatsim voice works.
            // should return something like vrc://server?user=user ...
            // ...or something else that can be passed to system(). See ControllerDetails.cpp on how this is used
            return QString();

        case Settings::NONE:
        default:
            return QString();
    }
}
