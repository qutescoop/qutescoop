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

#include <QDebug>

#include "WhazzupData.h"

#include "Fir.h"
#include "Pilot.h"
#include "Controller.h"
#include "BookedController.h"
#include "NavData.h"
#include "helpers.h"

WhazzupData::WhazzupData():
	connectedClients(0),
	connectedServers(0),
	whazzupVersion(0),
    whazzupTime(QDateTime()),
    bookingsTime(QDateTime()),
    dataType(UNIFIED)
{
}

WhazzupData::WhazzupData(QBuffer* buffer, WhazzupType type):
	connectedClients(0),
	connectedServers(0),
	whazzupVersion(0),
    whazzupTime(QDateTime()),
    bookingsTime(QDateTime())
{
    dataType = type;
    enum ParserState {STATE_NONE, STATE_GENERAL, STATE_CLIENTS, STATE_SERVERS, STATE_PREFILE};
	ParserState state = STATE_NONE;
	while(buffer->canReadLine()) {
		QString line = QString(buffer->readLine()).trimmed();
		if(line.isEmpty())
			continue;

		if(line.startsWith(';'))  // comments
			continue;

		if(line.startsWith('!')) {
			if(line.startsWith("!CLIENTS"))
				state = STATE_CLIENTS;
			else if(line.startsWith("!GENERAL"))
				state = STATE_GENERAL;
			else if(line.startsWith("!SERVERS"))
				state = STATE_SERVERS;
			else if(line.startsWith("!PREFILE"))
				state = STATE_PREFILE;
			else
				state = STATE_NONE;

			continue;
		}

		switch(state) {
		case STATE_NONE:
		case STATE_SERVERS:
			break;
		case STATE_GENERAL: {
				QStringList list = line.split('=');
				if(list.size() != 2)
					continue;
				if(line.startsWith("CONNECTED CLIENTS")) {
					connectedClients = list[1].trimmed().toInt();
                } else if(line.startsWith("CONNECTED SERVERS")) {
					connectedServers = list[1].trimmed().toInt();
                } else if(line.startsWith("RELOAD")) {
                    // maybe schedule reloading here
                } else if(line.startsWith("BOOKING")) {
                    // always "1" for bookings, but we select already by the whazzup location
                } else if(line.startsWith("UPDATE")) {
                    if(type == WHAZZUP) {
                        whazzupTime = QDateTime::fromString(list[1].trimmed(), "yyyyMMddHHmmss");
                        whazzupTime.setTimeSpec(Qt::UTC);
                    } else if(type == ATCBOOKINGS) {
                        bookingsTime = QDateTime::fromString(list[1].trimmed(), "yyyyMMddHHmmss");
                        bookingsTime.setTimeSpec(Qt::UTC);
                    }
                } else if(line.startsWith("VERSION")) {
					whazzupVersion = list[1].trimmed().toInt();
                }
			}
			break;
		case STATE_CLIENTS: {
				QStringList list = line.split(':');
				if(list.size() < 4)
					continue;

				if(list[3] == "PILOT") {
					if (type == WHAZZUP) {
                        Pilot *p = new Pilot(list, this);
                        pilots[p->label] = p;
                    }
				}
				else if(list[3] == "ATC") {
                    if (type == WHAZZUP) {
                        Controller *c = new Controller(list, this);
                        controllers[c->label] = c;
                    } else if (type == ATCBOOKINGS) {
                        BookedController *bc = new BookedController(list, this);
                        bookedcontrollers.append(bc);
                    }
				}
			}
			break;
		case STATE_PREFILE: {
				if (type == WHAZZUP) {
                    QStringList list = line.split(':');
                    Pilot *p = new Pilot(list, this);
    				pilots[p->label] = p;
                }
			}
			break;
		}
	}
}

WhazzupData::WhazzupData(const QDateTime predictTime, const WhazzupData& data):
    connectedClients(0),
    connectedServers(0),
    whazzupVersion(0),
    whazzupTime(QDateTime()),
    bookingsTime(QDateTime()),
    predictionBasedOnTime(QDateTime()),
    predictionBasedOnBookingsTime(QDateTime())
{
    whazzupVersion = data.whazzupVersion;
    whazzupTime = predictTime;
    predictionBasedOnTime = QDateTime(data.whazzupTime);
    predictionBasedOnBookingsTime = QDateTime(data.bookingsTime);

    dataType = data.dataType;
    // so now lets fake some controllers
    QList<BookedController*> bc = data.getBookedControllers();
    for (int i=0; i<bc.size(); i++) {
        if (bc[i] == 0) continue;
        if (bc[i]->starts() <= predictTime && bc[i]->ends() >= predictTime) { // only ones booked for the selected time
            QStringList sl;// = new QStringList();
            for (int h=0; h<50; h++) sl.append(QString()); // build a QStringList with enough items

            //userId = getField(stringList, 1);
            sl[0]= bc[i]->label;
            sl[2] = bc[i]->realName;
            sl[18] = QString("%1").arg(bc[i]->facilityType);

            //lat = getField(stringList, 5).toDouble();
            //lon = getField(stringList, 6).toDouble();
            sl[5] = QString("%1").arg(bc[i]->lat);
            sl[6] = QString("%1").arg(bc[i]->lon);

            //atisMessage = getField(stringList, 35);
            sl[35] = QString::fromUtf8("^§BOOKED from %1 to %2 UTC^§%3")
                     .arg(bc[i]->starts().toString("hh:mm"))
                     .arg(bc[i]->ends().toString("hh:mm"))
                     .arg(bc[i]->bookingInfoStr);

            //timeConnected = QDateTime::fromString(getField(stringList, 37), "yyyyMMddhhmmss");
            sl[37] = bc[i]->timeConnected.toString("yyyyMMddhhmmss");

            //server = getField(stringList, 14);
            sl[14] = "BOOKED SESSION"; 

            // not applicable:
            //frequency = getField(stringList, 4);
            //visualRange = getField(stringList, 19).toInt();
            //timeLastAtisReceived = QDateTime::fromString(getField(stringList, 36), "yyyyMMddhhmmss");
            //protrevision = getField(stringList, 15).toInt();
            //rating = getField(stringList, 16).toInt();
            
            controllers[bc[i]->label] = new Controller(sl, this);
        }
    }

    QList<Pilot*> p = data.getPilots();
    int altitude;
    QDateTime startTime, endTime = QDateTime();
    double lat, lon, startLat, startLon, endLat, endLon
            , dist, groundspeed, enrouteHrs, trueHeading = 0.0;

    for (int i=0; i < p.size(); i++) {
        if (p[i] == 0)
            continue;
        if (!p[i]->eta().isValid())
            continue; // no ETA, no prediction...
        if(p[i]->destAirport() == 0)
            continue; // sorry, no magic available yet. Just let him fly the last heading until etaPlan()? Does not make sense
        if(p[i]->etd() > predictTime || p[i]->eta() < predictTime) {
            if (p[i]->flightStatus() == Pilot::PREFILED && p[i]->etd() > predictTime) { // we want prefiled before their departure as in non-Warped view
                Pilot* np = new Pilot(*p[i]);
                np->whazzupTime = QDateTime(predictTime);
                pilots[np->label] = np; // just copy him over
                continue;
            }
            continue; // not on the map on the selected time
        }
        if (p[i]->flightStatus() == Pilot::PREFILED) {
            if(p[i]->depAirport() == 0 || p[i]->destAirport() == 0) {// if we dont know where a prefiled comes from, no magic available
                continue;
            }
            startTime = p[i]->etd();
            startLat = p[i]->depAirport()->lat;
            startLon = p[i]->depAirport()->lon;

            endTime = p[i]->eta();
            endLat = p[i]->destAirport()->lat;
            endLon = p[i]->destAirport()->lon;
            qDebug() << "pred " << p[i]->label << "PREFILED"; //fixme
        } else {
            startTime = data.timestamp();
            startLat = p[i]->lat;
            startLon = p[i]->lon;

            endTime = p[i]->eta();
            endLat = p[i]->destAirport()->lat;
            endLon = p[i]->destAirport()->lon;
        }
        dist = NavData::distance(startLat, startLon, endLat, endLon);
        enrouteHrs = ((double) startTime.secsTo(endTime)) / 3600.0;
        if (enrouteHrs == 0) enrouteHrs = 0.1;
        groundspeed = dist / enrouteHrs;
        trueHeading = NavData::courseTo(startLat, startLon, endLat, endLon);

        // altitude
        if(p[i]->planAlt.toInt() != 0)
            altitude = p[i]->defuckPlanAlt(p[i]->planAlt);

        // calculate position
        double partdist = (double) groundspeed * ((double) startTime.secsTo(predictTime) / 3600.0);
        NavData::distanceTo(startLat, startLon, partdist, trueHeading, &lat, &lon);

        // create Pilot instance and assign values
        Pilot* np = new Pilot(*p[i]);
        np->whazzupTime = QDateTime(predictTime);
        np->lat = lat;
        np->lon = lon;
        np->altitude = altitude;
        np->trueHeading = trueHeading;
        np->groundspeed = groundspeed;

        pilots[np->label] = np;
    }
    connectedClients = controllers.size() + pilots.size();
    qDebug() << "faked and inserted" << connectedClients << "clients for the Warp";
}

WhazzupData::WhazzupData(const WhazzupData& data) {
	assignFrom(data);
}

WhazzupData& WhazzupData::operator=(const WhazzupData& data) {
	assignFrom(data);
	return *this;
}

void WhazzupData::assignFrom(const WhazzupData& data) {
	if(this == &data)
		return;

    if (data.dataType == WHAZZUP || data.dataType == UNIFIED) {
        if(dataType == ATCBOOKINGS) dataType = UNIFIED;
        connectedClients = data.connectedClients;
        connectedServers = data.connectedServers;
        whazzupTime = data.whazzupTime;
        predictionBasedOnTime = data.predictionBasedOnTime;

        pilots.clear();
        QList<QString> callsigns = data.pilots.keys();
        for(int i = 0; i < callsigns.size(); i++) {
            pilots[callsigns[i]] = new Pilot(*data.pilots[callsigns[i]]);
        }

        controllers.clear();
        callsigns = data.controllers.keys();
        for(int i = 0; i < callsigns.size(); i++)
            controllers[callsigns[i]] = new Controller(*data.controllers[callsigns[i]]);
        
        if (!data.isVatsim()) {
            for(int i = 0; i < bookedcontrollers.size(); i++)
                delete bookedcontrollers[i];
            bookedcontrollers.clear();

            bookingsTime = QDateTime();
            predictionBasedOnBookingsTime = QDateTime();
        }
    }
    if (data.dataType == ATCBOOKINGS || data.dataType == UNIFIED) {
        if(dataType == WHAZZUP) dataType = UNIFIED;

        bookedcontrollers = data.bookedcontrollers;

        bookingsTime = QDateTime(data.bookingsTime);
        predictionBasedOnBookingsTime = QDateTime(data.predictionBasedOnBookingsTime);
    }
}

void WhazzupData::updatePilotsFrom(const WhazzupData& data) {
	QList<QString> callsigns = pilots.keys();
	for(int i = 0; i < callsigns.size(); i++) {
		if(!data.pilots.contains(callsigns[i])) {
			// remove pilots that are no longer there
			delete pilots[callsigns[i]];
			pilots.remove(callsigns[i]);
		}
	}
	callsigns = data.pilots.keys();
	for(int i = 0; i < callsigns.size(); i++) {
		if(!pilots.contains(callsigns[i])) {

			// create a new copy of new pilot
			Pilot *p = new Pilot(*data.pilots[callsigns[i]]);
			pilots[p->label] = p;

		} else {

			// pilot already exists, assign values from data
			bool showFrom = pilots[callsigns[i]]->displayLineFromDep;
			bool showTo = pilots[callsigns[i]]->displayLineToDest;
			QList<QPair<double, double> > track = pilots[callsigns[i]]->oldPositions;
			double oldLat = pilots[callsigns[i]]->lat;
			double oldLon = pilots[callsigns[i]]->lon;
			QString oldDest = pilots[callsigns[i]]->planDest;

			*pilots[callsigns[i]] = *data.pilots[callsigns[i]];
			pilots[callsigns[i]]->displayLineFromDep = showFrom;
			pilots[callsigns[i]]->displayLineToDest = showTo;

			if(pilots[callsigns[i]]->planDest != oldDest) {
				// if flightplan (=destination) changed, clear the flight path
				pilots[callsigns[i]]->oldPositions.clear();

			} else {

				double newLat = pilots[callsigns[i]]->lat;
				double newLon = pilots[callsigns[i]]->lon;
				if(!(oldLat == 0 && oldLon == 0) // dont add 0/0 to the waypoint list.
					&& (oldLat != newLat || oldLon != newLon))
						track.append(QPair<double, double>(oldLat, oldLon));

				pilots[callsigns[i]]->oldPositions = track;
			}
		}
	}
}

void WhazzupData::updateControllersFrom(const WhazzupData& data) {
	QList<QString> callsigns = controllers.keys();
	for(int i = 0; i < callsigns.size(); i++) {
		if(!data.controllers.contains(callsigns[i])) {
			// remove controllers that are no longer there
			delete controllers[callsigns[i]];
			controllers.remove(callsigns[i]);
		}
	}
	callsigns = data.controllers.keys();
	for(int i = 0; i < callsigns.size(); i++) {
		if(!controllers.contains(callsigns[i])) {
			// create a new copy of new controllers
			Controller *c = new Controller(*data.controllers[callsigns[i]]);
			controllers[c->label] = c;
		} else {
			// controller already exists, assign values from data
			*controllers[callsigns[i]] = *data.controllers[callsigns[i]];
		}
	}
}

void WhazzupData::updateBookedControllersFrom(const WhazzupData& data) {
    bookedcontrollers = data.bookedcontrollers;
}

void WhazzupData::updateFrom(const WhazzupData& data) {
	if(this == &data)
		return;

	if(data.isNull())
		return;
    
    if (data.dataType == WHAZZUP || data.dataType == UNIFIED) {
        if(dataType == ATCBOOKINGS) dataType = UNIFIED;
        updatePilotsFrom(data);
        updateControllersFrom(data);
    
        connectedClients = data.connectedClients;
        connectedServers = data.connectedServers;
        whazzupVersion = data.whazzupVersion;        
        whazzupTime = data.whazzupTime;
        predictionBasedOnTime = data.predictionBasedOnTime;
        if (!data.isVatsim()) {
            for(int i = 0; i < bookedcontrollers.size(); i++)
                delete bookedcontrollers[i];
            bookedcontrollers.clear();
            bookingsTime = QDateTime();
            predictionBasedOnBookingsTime = QDateTime();
        }
    }
    if (data.dataType == ATCBOOKINGS || data.dataType == UNIFIED) {
        if(dataType == WHAZZUP) dataType = UNIFIED;
        updateBookedControllersFrom(data);
        bookingsTime = data.bookingsTime;
        predictionBasedOnBookingsTime = data.predictionBasedOnBookingsTime;
    }
}

WhazzupData::~WhazzupData() {
	QList<QString> callsigns = pilots.keys();
	for(int i = 0; i < callsigns.size(); i++)
		delete pilots[callsigns[i]];
	pilots.clear();

	callsigns = controllers.keys();
    for(int i = 0; i < callsigns.size(); i++)
        delete controllers[callsigns[i]];
    controllers.clear();

    bookedcontrollers.clear();
}

QList<Controller*> WhazzupData::activeSectors() const {
	QList<Controller*> result;
	QHash<QString, Fir*> firs = NavData::getInstance()->firs();

	QList<Controller*> controllerList = controllers.values();
	for(int i = 0; i < controllerList.size(); i++) {
		QString icao = controllerList[i]->getCenter();
		if(icao.isNull() || icao.isEmpty())
			continue;

		while(!firs.contains(icao) && !icao.isEmpty()) {
			int p = icao.lastIndexOf('_');
			if(p == -1) {
				qDebug() << "Unknown FIR" << icao;
				icao = "";
				continue;
			}
			else {
				icao = icao.left(p);
			}
		}
		if(!icao.isEmpty() && firs.contains(icao)) {
			controllerList[i]->fir = firs[icao];
			controllerList[i]->lat = firs[icao]->lat();
			controllerList[i]->lon = firs[icao]->lon();
			result.append(controllerList[i]);
		}
	}

	return result;
}

QList<Pilot*> WhazzupData::getPilots() const { 
	return pilots.values(); 
}

QList<Pilot*> WhazzupData::getActivePilots() const {
	QList<Pilot*> result = pilots.values();
	for (int i = 0; i < result.size(); i++) {
        // exclude prefiled flights
        if (result[i]->flightStatus() == Pilot::PREFILED || (result[i]->lat == 0 && result[i]->lon == 0) ) {
			result.removeAt(i);
			i--;
		}
	}
	return result;
}

void WhazzupData::accept(MapObjectVisitor* visitor) const {
	for(int i = 0; i < controllers.size(); i++)
		visitor->visit(controllers.values()[i]);
	for(int i = 0; i < pilots.size(); i++)
		visitor->visit(pilots.values()[i]);
}