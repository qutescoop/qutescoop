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
#include "NavData.h"

WhazzupData::WhazzupData():
	connectedClients(0),
	connectedServers(0),
	whazzupVersion(0),
	whazzupTime(QDateTime())
{
}

WhazzupData::WhazzupData(QBuffer* buffer):
	connectedClients(0),
	connectedServers(0),
	whazzupVersion(0),
	whazzupTime(QDateTime())
{
	enum ParserState {STATE_NONE, STATE_GENERAL, STATE_CLIENTS, STATE_SERVERS};
	ParserState state = STATE_NONE;

	while(buffer->canReadLine()) {
		QString line = QString(buffer->readLine()).trimmed();
		if(line.isEmpty()) break;

		if(line.startsWith('!')) {
			if(line.startsWith("!CLIENTS"))
				state = STATE_CLIENTS;
			else if(line.startsWith("!GENERAL"))
				state = STATE_GENERAL;
			else if(line.startsWith("!SERVERS"))
				state = STATE_SERVERS;
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

				if(line.startsWith("CONNECTED CLIENTS"))
					connectedClients = list[1].trimmed().toInt();
				else if(line.startsWith("CONNECTED SERVERS"))
					connectedServers= list[1].trimmed().toInt();
				else if(line.startsWith("UPDATE"))
					whazzupTime = QDateTime::fromString(list[1].trimmed(), "yyyyMMddHHmmss");
				else if(line.startsWith("VERSION"))
					whazzupVersion = list[1].trimmed().toInt();
			}
			break;
		case STATE_CLIENTS: {
				QStringList list = line.split(':');
				if(list.size() < 4)
					continue;

				if(list[3] == "PILOT") {
					Pilot *p = new Pilot(list, this);
					pilots[p->label] = p;
				}
				else if(list[3] == "ATC") {
					Controller *c = new Controller(list, this);
					controllers[c->label] = c;
				}
			}
			break;
		}
	}
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

	connectedClients = data.connectedClients;
	connectedServers = data.connectedServers;
	whazzupTime = data.whazzupTime;

	pilots.clear();
	QList<QString> callsigns = data.pilots.keys();
	for(int i = 0; i < callsigns.size(); i++)
		pilots[callsigns[i]] = new Pilot(*data.pilots[callsigns[i]]);

	controllers.clear();
	callsigns = data.controllers.keys();
	for(int i = 0; i < callsigns.size(); i++)
		controllers[callsigns[i]] = new Controller(*data.controllers[callsigns[i]]);
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

void WhazzupData::updateFrom(const WhazzupData& data) {
	if(this == &data)
		return;

	if(data.isNull())
		return;

	updatePilotsFrom(data);
	updateControllersFrom(data);

	connectedClients = data.connectedClients;
	connectedServers = data.connectedServers;
	whazzupVersion = data.whazzupVersion;
	whazzupTime = data.whazzupTime;
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

void WhazzupData::accept(MapObjectVisitor* visitor) const {
	for(int i = 0; i < controllers.size(); i++)
		visitor->visit(controllers.values()[i]);
	for(int i = 0; i < pilots.size(); i++)
		visitor->visit(pilots.values()[i]);
}
