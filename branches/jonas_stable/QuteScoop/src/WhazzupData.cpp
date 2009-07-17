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
#include "Settings.h"
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
    enum ParserState {STATE_NONE, STATE_GENERAL, STATE_CLIENTS, STATE_SERVERS, STATE_VOICESERVERS, STATE_PREFILE};
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
            else if(line.startsWith("!VOICE SERVERS"))
                state = STATE_VOICESERVERS;
            else if(line.startsWith("!PREFILE"))
                state = STATE_PREFILE;
            else
                state = STATE_NONE;

            continue;
        }

        switch(state) {
        case STATE_NONE:
        case STATE_SERVERS: {
                QStringList list = line.split(':');
                if(list.size() < 5)
                    continue;
                //; !SERVERS section -         ident:hostname_or_IP:location:name:clients_connection_allowed:
                //EUROPE-C2:88.198.19.202:Europe:Center Europe Server Two:1:
            }
            break;
        case STATE_VOICESERVERS: {
                QStringList list = line.split(':');
                if(list.size() < 5)
                    continue;
                //; !VOICE SERVERS section -   hostname_or_IP:location:name:clients_connection_allowed:type_of_voice_server:
                //voice2.vacc-sag.org:Nurnberg:Europe-CW:1:R:
            }
            break;
        case STATE_GENERAL: {
                QStringList list = line.split('=');
                if(list.size() != 2)
                    continue;
                if(line.startsWith("CONNECTED CLIENTS")) {
                    connectedClients = list[1].trimmed().toInt();
                } else if(line.startsWith("CONNECTED SERVERS")) {
                    connectedServers = list[1].trimmed().toInt();
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
                } else if(line.startsWith("RELOAD")) {
                    //maybe schedule reloading here
                    /*
                    int reloadInMin = list[1].trimmed().toInt();

                    if (whazzupTime.isValid() && reloadIn > 0) {
                        TDateTime reloadAt = whazzupTime.addSecs(reloadInMin * 60);
                        if(reloadAt < QDateTime::currentDateTime().addSecs(Settings::downloadInterval() * 60))
                            reloadAt = QDateTime::currentDateTime().addSecs(Settings::downloadInterval() * 60);
                    }
                    */

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
                        while (list.size() > 42 && whazzupVersion == 8) { // fix ":" in Controller Infos... - should be done by the server I think :(
                            list[35] = list[35] + ":" + list[36];
                            list.removeAt(36);
                        }
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
                    bookedpilots[p->label] = p;
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
            QStringList sl;
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
            sl[35] = QString::fromUtf8("^§BOOKED from %1, online until %2^§%3") // dont't change this String, it is needed for correctly assigning onlineUntil
                     .arg(bc[i]->starts().toString("HHmm'z'"))
                     .arg(bc[i]->ends().toString("HHmm'z'"))
                     .arg(bc[i]->bookingInfoStr);

            //timeConnected = QDateTime::fromString(getField(stringList, 37), "yyyyMMddHHmmss");
            sl[37] = bc[i]->timeConnected.toString("yyyyMMddHHmmss");

            //server = getField(stringList, 14);
            sl[14] = "BOOKED SESSION";

            //visualRange = getField(stringList, 19).toInt();
            sl[19] = QString("%1").arg(bc[i]->visualRange);

            // not applicable:
            //frequency = getField(stringList, 4);
            //visualRange = getField(stringList, 19).toInt();
            //timeLastAtisReceived = QDateTime::fromString(getField(stringList, 36), "yyyyMMddHHmmss");
            //protrevision = getField(stringList, 15).toInt();
            //rating = getField(stringList, 16).toInt();

            controllers[bc[i]->label] = new Controller(sl, this);
        }
    }

    // let controllers be in until he states in his Controller Info also if only found in Whazzup, not booked (to allow for smoother realtime simulation).
    QList<Controller*> c = data.getControllers();
    for (int i = 0; i < c.size(); i++) {
        QDateTime showUntil = predictionBasedOnTime.addSecs(Settings::downloadInterval() * 4 * 60); // standard for online controllers
        if(c[i]->assumeOnlineUntil.isValid()) {
            if(predictionBasedOnTime.secsTo(c[i]->assumeOnlineUntil) > 0) // use only if we catched him before his stated leave-time.
                showUntil = c[i]->assumeOnlineUntil;
        }

        if (predictTime < showUntil && predictTime > predictionBasedOnTime) {
            controllers[c[i]->label] = new Controller(*c[i]);
        }
    }

    QList<Pilot*> p = data.getAllPilots();

    QDateTime startTime, endTime = QDateTime();
    double startLat, startLon, endLat, endLon = 0.0;
    int altitude = 0;

    for (int i=0; i < p.size(); i++) {
        if (p[i] == 0)
            continue;
        if (!p[i]->eta().isValid())
            continue; // no ETA, no prediction...
        if (!p[i]->etd().isValid() && predictTime < predictionBasedOnTime)
            continue; // no ETD, difficult prediction. Before the WhazzupTime, no prediction...
        if(p[i]->destAirport() == 0)
            continue; // sorry, no magic available yet. Just let him fly the last heading until etaPlan()? Does not make sense
        if(p[i]->etd() > predictTime || p[i]->eta() < predictTime) {
            if (p[i]->flightStatus() == Pilot::PREFILED && p[i]->etd() > predictTime) { // we want prefiled before their departure as in non-Warped view
                Pilot* np = new Pilot(*p[i]);
                np->whazzupTime = QDateTime(predictTime);
                bookedpilots[np->label] = np; // just copy him over
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
        } else {
            startTime = data.timestamp();
            startLat = p[i]->lat;
            startLon = p[i]->lon;

            endTime = p[i]->eta();
            endLat = p[i]->destAirport()->lat;
            endLon = p[i]->destAirport()->lon;
        }
        // altitude
        if(p[i]->planAlt.toInt() != 0)
            altitude = p[i]->defuckPlanAlt(p[i]->planAlt);

        // position
        double fraction = (double) startTime.secsTo(predictTime) / startTime.secsTo(endTime);
        double lat, lon;
        NavData::greatCirclePlotTo(startLat, startLon, endLat, endLon, fraction, &lat, &lon);

        double dist = NavData::distance(startLat, startLon, endLat, endLon);
        double enrouteHrs = ((double) startTime.secsTo(endTime)) / 3600.0;
        if (enrouteHrs == 0) enrouteHrs = 0.1;
        double groundspeed = dist / enrouteHrs;
        double trueHeading = NavData::courseTo(lat, lon, endLat, endLon);

        // create Pilot instance and assign values
        Pilot* np = new Pilot(*p[i]);
        np->whazzupTime = QDateTime(predictTime);
        np->lat = lat;
        np->lon = lon;
        np->altitude = altitude;
        np->trueHeading = trueHeading;
        np->groundspeed = (int) groundspeed;

        pilots[np->label] = np;
    }
    connectedClients = controllers.size() + pilots.size();
    qDebug() << "Warped to\t" << predictTime.toString() << "\t- Faked and inserted\t" << connectedClients << "clients";
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

        bookedpilots.clear();
        callsigns = data.bookedpilots.keys();
        for(int i = 0; i < callsigns.size(); i++) {
            bookedpilots[callsigns[i]] = new Pilot(*data.bookedpilots[callsigns[i]]);
        }

        controllers.clear();
        callsigns = data.controllers.keys();
        for(int i = 0; i < callsigns.size(); i++)
            controllers[callsigns[i]] = new Controller(*data.controllers[callsigns[i]]);
    }
    if (data.dataType == ATCBOOKINGS || data.dataType == UNIFIED) {
        if(dataType == WHAZZUP) dataType = UNIFIED;

        bookedcontrollers.clear();
        for (int i = 0; i < data.bookedcontrollers.size(); i++) {
            bookedcontrollers.append(new BookedController(*data.bookedcontrollers[i]));
        }

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

    bookedpilots.clear();
    callsigns = data.bookedpilots.keys();
    for(int i = 0; i < callsigns.size(); i++) {
        Pilot *p = new Pilot(*data.bookedpilots[callsigns[i]]);
        bookedpilots[p->label] = p;
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
    bookedcontrollers.clear();
    for (int i = 0; i < data.bookedcontrollers.size(); i++) {
        bookedcontrollers.append(new BookedController(*data.bookedcontrollers[i]));
    }
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

    callsigns = bookedpilots.keys();
    for(int i = 0; i < callsigns.size(); i++)
        delete bookedpilots[callsigns[i]];
    bookedpilots.clear();

    callsigns = controllers.keys();
    for(int i = 0; i < callsigns.size(); i++)
        delete controllers[callsigns[i]];
    controllers.clear();

    for(int i = 0; i < bookedcontrollers.size(); i++)
        delete bookedcontrollers[i];
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
                qDebug() << "Unknown FIR\t" << icao << "\tPlease provide sector information if you can";
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

QList<Pilot*> WhazzupData::getBookedPilots() const {
    return bookedpilots.values();
}

QList<Pilot*> WhazzupData::getAllPilots() const {
    QList<Pilot*> ap = bookedpilots.values();
    ap += pilots.values();
    return ap;
}

void WhazzupData::accept(MapObjectVisitor* visitor) const {
    for(int i = 0; i < controllers.size(); i++)
        visitor->visit(controllers.values()[i]);
    for(int i = 0; i < pilots.size(); i++)
        visitor->visit(pilots.values()[i]);
    for(int i = 0; i < bookedpilots.size(); i++)
        visitor->visit(bookedpilots.values()[i]);
}
