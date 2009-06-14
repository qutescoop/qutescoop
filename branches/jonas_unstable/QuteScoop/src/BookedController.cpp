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

#include "BookedController.h"
#include "ControllerDetails.h"
#include "Window.h"
#include "NavData.h"
#include "Settings.h"

BookedController::BookedController(const QStringList& stringList, const WhazzupData* whazzup):
	Client(stringList, whazzup)
{
    qDebug() << stringList;
    
    //frequency = getField(stringList, 4);
	facilityType = 10; //fixme
	//visualRange = getField(stringList, 19).toInt();
	//atisMessage = getField(stringList, 35);
	//timeLastAtisReceived = QDateTime::fromString(getField(stringList, 36), "yyyyMMddhhmmss");
	//voiceServer = atisLines[0];
	//atisMessage
    fir = 0;
    server = "";
	timeFrom = getField(stringList, 14);
	date = getField(stringList, 16);
    //always some miracle "1" here: getField(stringList, 19);
	timeTo = getField(stringList, 17);
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

	if(NavData::getInstance()->firs().contains(result)) {
		Fir *f = NavData::getInstance()->firs()[result];
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
	ControllerDetails *infoDialog = ControllerDetails::getInstance();

	infoDialog->refresh(dynamic_cast<Controller *>(this));
	infoDialog->show();
	infoDialog->raise();
	infoDialog->activateWindow();
	infoDialog->setFocus();
}

QString BookedController::rank() const {
	if(network == VATSIM) {
		switch(rating) {
		case 0:
		case 1: return "OBS"; break;
		case 2: return "STU"; break;
		case 3:
		case 4: return "STU+"; break;
		case 5: return "CTR"; break;
		case 6:
		case 7: return "CTR+"; break;
		case 8: return "INS"; break;
		case 9:
		case 10: return "INS+"; break;
		default: return "???"; break;
		}
	} else {
		switch(rating) {
		case 0:
		case 1: return "OBS"; break;
		case 2: return "S1"; break;
		case 3: return "S2"; break;
		case 4: return "S3"; break;
		case 5: return "C1"; break;
		case 6: return "C2"; break;
		case 7: return "C3"; break;
		case 8: return "I1"; break;
		case 9: return "I2"; break;
		case 10: return "I3"; break;
		default: return "??"; break;
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
            QDate::fromString(date, QString("yyyymmdd")), 
            QTime::fromString(timeFrom, QString("hhmm")),
            Qt::UTC
            );
}

QDateTime BookedController::ends() const {
    return QDateTime(
            QDate::fromString(date, QString("yyyymmdd")), 
            QTime::fromString(timeTo, QString("hhmm")), 
            Qt::UTC
            );
}
