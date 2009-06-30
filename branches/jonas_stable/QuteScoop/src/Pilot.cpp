/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
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

#include "Pilot.h"
#include "PilotDetails.h"
#include "Window.h"
#include "NavData.h"
#include "helpers.h"
#include "Settings.h"

Pilot::Pilot(const QStringList& stringList, const WhazzupData* whazzup):
	Client(stringList, whazzup),
	onGround(false),
	displayLineFromDep(false),
	displayLineToDest(false)
{
    whazzupTime = QDateTime(whazzup->timestamp()); // need some local reference to that

    altitude = getField(stringList, 7).toInt(); // we could do some barometric calculations here (only for VATSIM needed)
	groundspeed = getField(stringList, 8).toInt();
	planAircraft = getField(stringList, 9);
	planTAS = getField(stringList, 10);
	planDep = getField(stringList, 11);
	planAlt = getField(stringList, 12);
	planDest = getField(stringList, 13);

	transponder = getField(stringList, 17);
	planRevision = getField(stringList, 20);
	planFlighttype = getField(stringList, 21);
	planDeptime = getField(stringList, 22);
	planActtime = getField(stringList, 23);
    
	planHrsEnroute = getField(stringList, 24).toInt();
	planMinEnroute = getField(stringList, 25).toInt();
	planHrsFuel = getField(stringList, 26).toInt();
	planMinFuel = getField(stringList, 27).toInt();
	planAltAirport = getField(stringList, 28);
	planRemarks = getField(stringList, 29);
	planRoute = getField(stringList, 30);

	if(whazzup->isIvao()) {
		planAltAirport2 = getField(stringList, 42); // IVAO only
		planTypeOfFlight = getField(stringList, 43); // IVAO only
		pob = getField(stringList, 44).toInt(); // IVAO only

		trueHeading = getField(stringList, 45).toInt();
		onGround = getField(stringList, 46).toInt() == 1; // IVAO only
	}

	if(whazzup->isVatsim()) {
		trueHeading = getField(stringList, 38).toInt();
		qnhInHg = getField(stringList, 39); // VATSIM only
		qnhMb = getField(stringList, 40); // VATSIM only
	}
    // day of flight
    if(QTime::fromString(planDeptime, "HHmm") <= whazzupTime.time())
        dayOfFlight = whazzupTime.date();
    else
        dayOfFlight = whazzupTime.date().addDays(-1); // started the day before

	// anti-idiot hack: some guys like routes like KORLI/MARPI/UA551/FOF/FUN...
	if(planRoute.count('/') > 4)
		planRoute.replace(QRegExp("[/]"), " ");

	if(planRoute.isEmpty())
		planRoute = "n/a";
}

void Pilot::showDetailsDialog() {
	PilotDetails *infoDialog = PilotDetails::getInstance();
	infoDialog->refresh(this);
	infoDialog->show();
	infoDialog->raise();
	infoDialog->setFocus();
	infoDialog->activateWindow();
}

Pilot::FlightStatus Pilot::flightStatus() const {
	Airport *dep = depAirport();
	Airport *dst = destAirport();

	// flying?
	bool flying = true;
	if(groundspeed < 50 && altitude < 9000) flying = false;
	if(onGround) flying = false;

	if ( dep == NULL || dst == NULL ) {
		if(flying)
			return EN_ROUTE;
		else
			return BUSH;
	}

	double totalDist = NavData::distance(dep->lat, dep->lon, dst->lat, dst->lon);
	double distDone = NavData::distance(lat, lon, dep->lat, dep->lon);
	double distRemaining = totalDist - distDone;

	// arriving?
	bool arriving = false;
	if(distRemaining < 50) arriving = true;

	// departing?
	bool departing = false;
	if(distDone < 50) departing = true;


	// BOARDING: !flying, speed = 0, departing
	// GROUND_DEP: !flying, speed > 0, departing
	// DEPARTING: flying, departing
	// EN_ROUTE: flying, !departing, !arriving
	// ARRIVING: flying, arriving
	// GROUND_ARR: !flying, speed > 0, arriving
	// BLOCKED: !flying, speed = 0, arriving
	// PREFILED: !flying, lat=0, lon=0

	if(!flying && groundspeed == 0 && departing)
		return BOARDING;
	if(!flying && groundspeed > 0 && departing)
		return GROUND_DEP;
	if(flying && arriving)
		return ARRIVING; // put before departing; on small hops tend to show "arriving", not departing
	if(flying && departing)
		return DEPARTING;
	if(flying && !departing && !arriving)
		return EN_ROUTE;
	if(!flying && groundspeed > 0 && arriving)
		return GROUND_ARR;
    if(!flying && lat == 0 && lon == 0) // must be before BLOCKED
		return PREFILED;
	if(!flying && groundspeed == 0 && arriving)
		return BLOCKED;
	return CRASHED;
}

QString Pilot::flightStatusString() const {
    QString result;
    switch(flightStatus()) {
        case BOARDING:
            result = QString("Boarding");
            result += ", EET " + eet().toString("H:mm") + " hrs";
            result += ", ETA " + eta().toString("HH:mm") + " UTC";
            result += ", Delay " + delayStr() + " hrs";
            return result;
        case GROUND_DEP:
            result = QString("Taxi to runway");
            result += ", EET " + eet().toString("H:mm") + " hrs";
            result += ", ETA " + eta().toString("HH:mm") + " UTC";
            result += ", Delay " + delayStr() + " hrs";
            return result;
        case DEPARTING:
            result = QString("Departing");
            result += ", EET " + eet().toString("H:mm") + " hrs";
            result += ", ETA " + eta().toString("HH:mm") + " UTC";
            result += ", Delay " + delayStr() + " hrs";
            return result;
        case ARRIVING:
            result = QString("Arriving");
            result += ", EET " + eet().toString("H:mm") + " hrs";
            result += ", ETA " + eta().toString("HH:mm") + " UTC";
            result += ", Delay " + delayStr() + " hrs";
            return result;
        case GROUND_ARR:
            result = QString("Taxi to gate");
            result += ", ATA " + eta().toString("HH:mm") + " UTC"; // Actual Time of Arrival :)
            result += ", Delay " + delayStr() + " hrs";
            return result;
        case BLOCKED:
            result = QString("Blocked at gate");
            result += ", Delay " + delayStr();
            return result;
        case CRASHED: return "Crashed";
		case BUSH: return "Bush pilot";
		case EN_ROUTE: {
				Airport *dep = depAirport();
				Airport *dst = destAirport();
				if(dst == 0)
					return "En route";

                result = "En route";
				if(dep != 0) {
					// calculate %done
					int total_dist = (int)NavData::distance(dep->lat, dep->lon, dst->lat, dst->lon);
					if(total_dist > 0) {
						int dist_done = (int)NavData::distance(lat, lon, dep->lat, dep->lon);
						result += QString(" (%1%)").arg(dist_done * 100 / total_dist);
					}
				}
                result += ", EET " + eet().toString("H:mm") + " hrs";
                result += ", ETA " + eta().toString("HH:mm") + " UTC";
                result += ", Delay " + delayStr() + " hrs";
				return result;
            }
        case PREFILED:
            return QString("Prefiled (ETD %1, ETA %2)")
                    .arg(etd().toString("ddd MM/dd HH:mm"))
                    .arg(etaPlan().toString("ddd MM/dd HH:mm"));
	}
	return "Unknown";
}

QString Pilot::flightStatusShortString() const {
    switch(flightStatus()) {
        case BOARDING: return QString("Boarding");
        case GROUND_DEP: return QString("Taxi to runway");
        case DEPARTING: return QString("Departing");
        case ARRIVING: return QString("Arriving");
        case GROUND_ARR: return QString("Taxi to gate");
        case BLOCKED: return QString("Blocked at gate");
        case CRASHED: return QString("Crashed");
        case BUSH: return QString("Bush pilot");
        case EN_ROUTE: return QString("En Route");
        case PREFILED: return QString("Prefiled");
    }
    return QString("Unknown");
}

QString Pilot::rank() const {
	if(network == IVAO) {
		switch(rating) {
		case 2: return "SFO"; break;
		case 3: return "FFO"; break;
		case 4: return "C"; break;
		case 5: return "FC"; break;
		case 6: return "SC"; break;
		case 7: return "SFC"; break;
		case 8: return "CC"; break;
		case 9: return "CFC"; break;
		case 10: return "CSC"; break;
		default: return "??"; break;
		}
	}
	return QString();
}

QString Pilot::aircraftType() const {
	QStringList acftSegments = planAircraft.split('/');

	if(network == IVAO && acftSegments.size() >= 2) {
		return acftSegments[1];
	}

	if(network == VATSIM) {
		// VATSIM can be a real PITA.
		QString result = planAircraft;
		if(acftSegments.size() == 2 && acftSegments[0].length() > 2) result = acftSegments[0];
		else if(acftSegments.size() >= 2) result = acftSegments[1];

		return result;
	}

	return planAircraft;
}

Airport* Pilot::depAirport() const {
    if(NavData::getInstance()->airports().contains(planDep))
        return NavData::getInstance()->airports()[planDep];
    else return 0;
}

Airport* Pilot::destAirport() const {
    if(NavData::getInstance()->airports().contains(planDest))
        return NavData::getInstance()->airports()[planDest];
    else return 0;
}

Airport* Pilot::altAirport() const {
    if(NavData::getInstance()->airports().contains(planAltAirport))
        return NavData::getInstance()->airports()[planAltAirport];
    else return 0;
}

double Pilot::distanceFromDeparture() const {
	Airport *dep = depAirport();
	if(dep == 0)
		return 0;

	return NavData::distance(lat, lon, dep->lat, dep->lon);
}

double Pilot::distanceToDestination() const {
	Airport *dest = destAirport();
	if(dest == 0)
		return 0;

	return NavData::distance(lat, lon, dest->lat, dest->lon);
}

int Pilot::planTasInt() const { // defuck flightplanned TAS
    if(planTAS.startsWith("M")) { // approximate Mach -> TAS conversion
        if(planTAS.contains("."))
            return (int) planTAS.mid(1).toDouble() * 550;
        else
            return (int) planTAS.mid(1).toInt() * 5.5;
    }
    return planTAS.toInt();
}

QDateTime Pilot::etd() const { // Estimated Time of Departure
    if(planDeptime.length() < 3)
        return QDateTime();
    QString planDeptimeFixed = planDeptime;

    if(planDeptime.length() == 3)
        planDeptimeFixed.prepend("0"); // fromString("Hmm") does not handle "145" correctly -> "14:05"

    QTime time = QTime::fromString(planDeptimeFixed, "HHmm");
    return QDateTime(dayOfFlight, time, Qt::UTC);
}

QDateTime Pilot::eta() const { // Estimated Time of Arrival
    FlightStatus status = flightStatus();
    if(status == PREFILED || status == BOARDING || status == GROUND_DEP) {
        return etaPlan();
    } else if(status == DEPARTING) { // try to calculate with flightplanned speed
        int enrouteSecs;
        if(planTasInt() == 0) {
            if(groundspeed == 0)
                return QDateTime(); // abort
            enrouteSecs = (int) (distanceToDestination() * 3600) / groundspeed;
        } else {
            enrouteSecs = (int) (distanceToDestination() * 3600) / planTasInt();
        }
        return whazzupTime.addSecs(enrouteSecs);
    } else if(status == EN_ROUTE || status == ARRIVING) { // try groundspeed
        int enrouteSecs;
        if(groundspeed == 0) {
            if(planTasInt() == 0)
                return QDateTime(); // abort
            enrouteSecs = (int) (distanceToDestination() * 3600) / planTasInt();
        } else {
            enrouteSecs = (int) (distanceToDestination() * 3600) / groundspeed;
        }
        QDateTime ret = whazzupTime.addSecs(enrouteSecs);
        return ret;
    } else if(status == GROUND_ARR) {
        return whazzupTime;
    }
    return QDateTime();
}

QTime Pilot::eet() const { // Estimated Enroute Time remaining
    int secs = whazzupTime.secsTo(eta());
    QTime ret = QTime((secs / 3600) % 24, (secs / 60) % 60);
    return ret;
}

QDateTime Pilot::etaPlan() const { // Estimated Time of Arrival as flightplanned
    return etd().addSecs(planHrsEnroute * 3600 + planMinEnroute * 60);
}

QString Pilot::delayStr() const { // delay
    int secs, calcSecs;
    secs = etaPlan().secsTo(eta());
    if (secs == 0)
        return QString("none");
    calcSecs = (secs < 0? -secs: secs);
    return QString("%1%2")
            .arg(secs < 0? "-": "")
            .arg(QTime((calcSecs / 3600) % 24, (calcSecs / 60) % 60).toString("H:mm"));
}

int Pilot::defuckPlanAlt(QString altStr) const { // returns an altitude from various flightplan strings
    altStr = altStr.trimmed();
    if(altStr.length() < 4) // 280
        return altStr.toInt() * 100;
    if(altStr.left(2) == "FL") // FL280
        return altStr.mid(2).toInt() * 100;
    if(altStr.left(1) == "F") // F280
        return altStr.mid(1).toInt() * 100;
    return altStr.toInt();
}


QStringList Pilot::waypoints() const {
	QStringList result = planRoute.split(QRegExp("([\\s\\+\\-\\.\\,]|//)"), QString::SkipEmptyParts);
	if(result.isEmpty()) {
		result.append("n/a");
	}
	return result;
}

void Pilot::positionInFuture(double *futureLat, double *futureLon, int seconds) const {
	if(groundspeed == 0) {
		*futureLat = lat;
		*futureLon = lon;
		return;
	}

	double dist = (double)groundspeed * ((double)seconds / 3600.0);
	NavData::distanceTo(lat, lon, dist, trueHeading, futureLat, futureLon);
}

void Pilot::toggleDisplayPath() {
	if(displayLineToDest) {
		displayLineFromDep = false;
		displayLineToDest = false;
	} else {
		displayLineFromDep = true;
		displayLineToDest = true;
	}
}

void Pilot::plotFlightPath() const {
	if(displayLineToDest)
		plotPathToDest();

	if(displayLineFromDep)
		plotPathFromDep();

	if(displayLineToDest || displayLineFromDep)
		plotPlannedLine();
}

void Pilot::plotPath(double lat1, double lon1, double lat2, double lon2) const {

	// always start plotting at the origin
	VERTEX(lat1, lon1);

	double d = NavData::distance(lat1, lon1, lat2, lon2);
	if(d < 1) return; // less than 1 mile - not worth plotting

    double fractionIncrement = 30 / d; // one dot every 30nm
	if(fractionIncrement > 1) fractionIncrement = 1;
	double currentFraction = 0;

	double myLat = lat1;
	double myLon = lon1;
	do {
		currentFraction += fractionIncrement;
		if(currentFraction > 1) currentFraction = 1;

		if(currentFraction < 1) {
			// don't plot last dot - we do that in the caller. Avoids plotting vertices twice
			NavData::greatCirclePlotTo(lat1, lon1, lat2, lon2, currentFraction, &myLat, &myLon);
			VERTEX(myLat, myLon);
		}

	} while(currentFraction < 1);
}

void Pilot::plotPathFromDep() const {
	if(Settings::trackLineStrength() == 0)
		return;

	Airport *dep = depAirport();
	if(dep == 0) return; // dont know where to plot to - abort

	double currLat = dep->lat;
	double currLon = dep->lon;

	if(!Settings::dashedTrackInFront())
		glLineStipple(3, 0xAAAA);

	QColor lineCol = Settings::trackLineColor();
	glColor4f(lineCol.redF(), lineCol.greenF(), lineCol.blueF(), lineCol.alphaF());
	glLineWidth(Settings::trackLineStrength());
	glBegin(GL_LINE_STRIP);

		for(int i = 0; i < oldPositions.size(); i++) {
			plotPath(currLat, currLon, oldPositions[i].first, oldPositions[i].second);
			currLat = oldPositions[i].first;
			currLon = oldPositions[i].second;
		}

		plotPath(currLat, currLon, lat, lon);
		VERTEX(lat, lon);
	glEnd();
	glLineStipple(1, 0xFFFF);
}

void Pilot::plotPathToDest() const {
	if(Settings::trackLineStrength() == 0)
		return;

	Airport *dest = destAirport();
	if(dest == 0) return; // dont know where to plot to - abort

	QColor lineCol = Settings::trackLineColor();
	glColor4f(lineCol.redF(), lineCol.greenF(), lineCol.blueF(), lineCol.alphaF());

	if(Settings::dashedTrackInFront())
		glLineStipple(3, 0xAAAA);

	glLineWidth(Settings::trackLineStrength());
	glBegin(GL_LINE_STRIP);
		plotPath(lat, lon, dest->lat, dest->lon);
		VERTEX(dest->lat, dest->lon);
	glEnd();
	glLineStipple(1, 0xFFFF);
}

void Pilot::plotPlannedLine() const {
	if(Settings::planLineStrength() == 0)
		return;

	QList<Waypoint*> points = resolveFlightplan();

	if(points.size() < 2)
		return;

	double currLat = points[0]->lat;
	double currLon = points[0]->lon;

	QColor lineCol = Settings::planLineColor();
	glColor4f(lineCol.redF(), lineCol.greenF(), lineCol.blueF(), lineCol.alphaF());

	glLineWidth(Settings::planLineStrength());
	glBegin(GL_LINE_STRIP);

		Airport* ap = depAirport();
		if(ap != 0) {
			VERTEX(ap->lat, ap->lon);
		}

		for(int i = 1; i < points.size(); i++) {
			plotPath(currLat, currLon, points[i]->lat, points[i]->lon);
			currLat = points[i]->lat;
			currLon = points[i]->lon;
		}
		VERTEX(currLat, currLon);

		ap = destAirport();
		if(ap != 0) {
			VERTEX(ap->lat, ap->lon);
		}

	glEnd();
}

QList<Waypoint*> Pilot::resolveFlightplan() const {
	QList<Waypoint*> result;

	Airport *dep = depAirport();
	if(dep == 0) return QList<Waypoint*>();

	QStringList list = planRoute.split(' ', QString::SkipEmptyParts);
	return NavData::getInstance()->getAirac().resolveFlightplan(list, dep->lat, dep->lon);
}
