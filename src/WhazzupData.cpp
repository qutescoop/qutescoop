/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "WhazzupData.h"

#include "Sector.h"
#include "Pilot.h"
#include "Controller.h"
#include "BookedController.h"
#include "NavData.h"
#include "Settings.h"
#include "helpers.h"

WhazzupData::WhazzupData():
    servers(QList<QStringList>()), voiceServers(QList<QStringList>()),
    updateEarliest(QDateTime()), whazzupTime(QDateTime()), bookingsTime(QDateTime()),
    whazzupVersion(0), dataType(UNIFIED)
{
}

WhazzupData::WhazzupData(QNetworkReply* buffer, WhazzupType type):
    servers(QList<QStringList>()), voiceServers(QList<QStringList>()),
    updateEarliest(QDateTime()), whazzupTime(QDateTime()), bookingsTime(QDateTime()), whazzupVersion(0)
{
    qDebug() << "WhazzupData::WhazzupData(buffer)" << type << "[NONE, WHAZZUP, ATCBOOKINGS, UNIFIED]";
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    dataType = type;
    QStringList friends = Settings::friends();
    int reloadInMin = Settings::downloadInterval();
    enum ParserState {STATE_NONE, STATE_GENERAL, STATE_CLIENTS, STATE_SERVERS, STATE_VOICESERVERS, STATE_PREFILE};
    ParserState state = STATE_NONE;
    while(buffer->canReadLine()) {
        // keep GUI responsive - leads to hangups?
        qApp->processEvents();

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
                servers += list;
                //; !SERVERS section -         ident:hostname_or_IP:location:name:clients_connection_allowed:
                //EUROPE-C2:88.198.19.202:Europe:Center Europe Server Two:1:
            }
                break;
            case STATE_VOICESERVERS: {
                QStringList list = line.split(':');
                if(list.size() < 5)
                    continue;
                voiceServers += list;
                //; !VOICE SERVERS section -   hostname_or_IP:location:name:clients_connection_allowed:type_of_voice_server:
                //voice2.vacc-sag.org:Nurnberg:Europe-CW:1:R:
            }
                break;
            case STATE_GENERAL: {
                QStringList list = line.split('=');
                if(list.size() != 2)
                    continue;
                if(line.startsWith("CONNECTED CLIENTS")) {
                    //connectedClients = list[1].trimmed().toInt(); // we do not trust the server any more. Take clients() instead.
                } else if(line.startsWith("CONNECTED SERVERS")) {
                    //connectedServers = list[1].trimmed().toInt(); // not important. Take serverList.size() instead
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
                    reloadInMin = list[1].trimmed().toInt();
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
                        if(friends.contains(p->userId)) friendsLatLon.append(QPair< double, double>(p->lat, p->lon));
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
                        if(friends.contains(c->userId)) friendsLatLon.append(QPair< double,double>(c->lat,c->lon));
                    } else if (type == ATCBOOKINGS) {
                        BookedController *bc = new BookedController(list, this);
                        bookedControllers.append(bc);
                        //if(friends.contains(bc->userId)) bookedFriendControllers.append( bc);
                    }
                }
            }
                break;
            case STATE_PREFILE: {
                if (type == WHAZZUP) {
                    QStringList list = line.split(':');
                    Pilot *p = new Pilot(list, this);
                    bookedPilots[p->label] = p;
                    //if(friends.contains(p->userId)) prefiledFriendPilots.append( p);
                }
            }
                break;
        }
    }

    // set the earliest time the server will have new data
    if (whazzupTime.isValid() && reloadInMin > 0)
        updateEarliest = whazzupTime.addSecs(reloadInMin * 60).toUTC();
    qApp->restoreOverrideCursor();
    qDebug() << "WhazzupData::WhazzupData(buffer) -- finished";
}

//faking WhazzupData based on valid data and a predictTime
WhazzupData::WhazzupData(const QDateTime predictTime, const WhazzupData &data):
    servers(QList<QStringList>()), voiceServers(QList<QStringList>()),
    updateEarliest(QDateTime()), whazzupTime(QDateTime()), bookingsTime(QDateTime()),
    predictionBasedOnTime(QDateTime()),  predictionBasedOnBookingsTime(QDateTime()),
    whazzupVersion(0)
{
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    qDebug() << "WhazzupData::WhazzupData(predictTime)" << predictTime;

    whazzupVersion = data.whazzupVersion;
    whazzupTime = predictTime;
    predictionBasedOnTime = QDateTime(data.whazzupTime);
    predictionBasedOnBookingsTime = QDateTime(data.bookingsTime);

    dataType = data.dataType;
    // so now lets fake some controllers
    foreach(const BookedController* bc, data.bookedControllers) {
        //if (bc == 0) continue;
        if (bc->starts() <= predictTime && bc->ends() >= predictTime) { // only ones booked for the selected time
            QStringList sl;
            for(int h=0; h < 40; h++)
                sl.append(QString()); // build a QStringList with enough items

            //userId = getField(stringList, 1);
            sl[0]= bc->label;
            sl[2] = bc->realName;
            sl[18] = QString("%1").arg(bc->facilityType);

            //lat = getField(stringList, 5).toDouble();
            //lon = getField(stringList, 6).toDouble();
            sl[5] = QString("%1").arg(bc->lat);
            sl[6] = QString("%1").arg(bc->lon);

            //atisMessage = getField(stringList, 35);
            sl[35] = QString::fromUtf8("^§BOOKED from %1, online until %2^§%3") // dont't change this String, it is needed for correctly assigning onlineUntil
                    .arg(bc->starts().toString("HHmm'z'"))
                    .arg(bc->ends().toString("HHmm'z'"))
                    .arg(bc->bookingInfoStr);

            //timeConnected = QDateTime::fromString(getField(stringList, 37), "yyyyMMddHHmmss");
            sl[37] = bc->timeConnected.toString("yyyyMMddHHmmss");

            //server = getField(stringList, 14);
            sl[14] = "BOOKED SESSION";

            //visualRange = getField(stringList, 19).toInt();
            sl[19] = QString("%1").arg(bc->visualRange);

            // not applicable:
            //frequency = getField(stringList, 4);
            //visualRange = getField(stringList, 19).toInt();
            //timeLastAtisReceived = QDateTime::fromString(getField(stringList, 36), "yyyyMMddHHmmss");
            //protrevision = getField(stringList, 15).toInt();
            //rating = getField(stringList, 16).toInt();

            controllers[bc->label] = new Controller(sl, this);
        }
    }

    // let controllers be in until he states in his Controller Info also if only found in Whazzup, not booked
    foreach(const Controller *c, data.controllers) {
        QDateTime showUntil = predictionBasedOnTime.addSecs(Settings::downloadInterval() * 4 * 60); // standard for online controllers: 10 min
        if(c->assumeOnlineUntil.isValid())
            if(predictionBasedOnTime.secsTo(c->assumeOnlineUntil) >= 0) // use only if we catched him before his stated leave-time.
                showUntil = c->assumeOnlineUntil;

        if (predictTime <= showUntil && predictTime >= predictionBasedOnTime)
            controllers[c->label] = new Controller(*c);
    }

    QDateTime startTime, endTime = QDateTime();
    double startLat, startLon, endLat, endLon = 0.;
    int altitude = 0;

    foreach(const Pilot *p, data.allPilots()) {
        //if (p == 0) continue;
        if (!p->eta().isValid())
            continue; // no ETA, no prediction...
        if (!p->etd().isValid() && predictTime < predictionBasedOnTime)
            continue; // no ETD, difficult prediction. Before the whazzupTime, no prediction...
        if(p->destAirport() == 0)
            continue; // sorry, no magic available yet. Just let him fly the last heading until etaPlan()? Does not make sense
        if(p->etd() > predictTime || p->eta() < predictTime) {
            if (p->flightStatus() == Pilot::PREFILED && p->etd() > predictTime) { // we want prefiled before their
                //departure as in non-Warped view
                Pilot* np = new Pilot(*p);
                np->whazzupTime = QDateTime(predictTime);
                bookedPilots[np->label] = np; // just copy him over
                continue;
            }
            continue; // not on the map on the selected time
        }
        if (p->flightStatus() == Pilot::PREFILED) {
            if(p->depAirport() == 0 || p->destAirport() == 0) {// if we dont know where a prefiled comes from, no magic available
                continue;
            }
            startTime = p->etd();
            startLat = p->depAirport()->lat;
            startLon = p->depAirport()->lon;

            endTime = p->eta();
            endLat = p->destAirport()->lat;
            endLon = p->destAirport()->lon;
        } else {
            startTime = data.whazzupTime;
            startLat = p->lat;
            startLon = p->lon;

            endTime = p->eta();
            endLat = p->destAirport()->lat;
            endLon = p->destAirport()->lon;
        }
        // altitude
        if(p->planAlt.toInt() != 0)
            altitude = p->defuckPlanAlt(p->planAlt);

        // position
        double fraction = (double) startTime.secsTo(predictTime) / startTime.secsTo(endTime);
        QPair<double, double> pos = NavData::greatCircleFraction(startLat, startLon, endLat, endLon, fraction);

        double dist = NavData::distance(startLat, startLon, endLat, endLon);
        double enrouteHrs = ((double) startTime.secsTo(endTime)) / 3600.0;
        if (qFuzzyIsNull(enrouteHrs)) enrouteHrs = 0.1;
        double groundspeed = dist / enrouteHrs;
        double trueHeading = NavData::courseTo(pos.first, pos.second, endLat, endLon);

        // create Pilot instance and assign values
        Pilot* np = new Pilot(*p);
        np->whazzupTime = QDateTime(predictTime);
        np->lat = pos.first;
        np->lon = pos.second;
        np->altitude = altitude;
        np->trueHeading = trueHeading;
        np->groundspeed = (int) groundspeed;

        pilots[np->label] = np;
    }
    qApp->restoreOverrideCursor();
    qDebug() << "WhazzupData::WhazzupData(predictTime) -- finished";
}

WhazzupData::WhazzupData(const WhazzupData &data) {
    assignFrom(data);
}

WhazzupData::~WhazzupData() {
    foreach(const QString s, pilots.keys())
        delete pilots[s];
    pilots.clear();

    foreach(const QString s, bookedPilots.keys())
        delete bookedPilots[s];
    bookedPilots.clear();

    foreach(const QString s, controllers.keys())
        delete controllers[s];
    controllers.clear();

    foreach(const BookedController *bc, bookedControllers)
        delete bc;
    bookedControllers.clear();
}

WhazzupData &WhazzupData::operator=(const WhazzupData &data) {
    assignFrom(data);
    return *this;
}

void WhazzupData::assignFrom(const WhazzupData &data) {
    qDebug() << "WhazzupData::assignFrom()";
    if(this == &data)
        return;

    if (data.dataType == WHAZZUP || data.dataType == UNIFIED) {
        if(dataType == ATCBOOKINGS) dataType = UNIFIED;
        servers = data.servers;
        voiceServers = data.voiceServers;
        whazzupTime = data.whazzupTime;
        predictionBasedOnTime = data.predictionBasedOnTime;
        updateEarliest = data.updateEarliest;

        pilots.clear();
        foreach(const QString s, data.pilots.keys())
            pilots[s] = new Pilot(*data.pilots[s]);

        bookedPilots.clear();
        foreach(const QString s, data.bookedPilots.keys())
            bookedPilots[s] = new Pilot(*data.bookedPilots[s]);

        controllers.clear();
        foreach(const QString s, data.controllers.keys())
            controllers[s] = new Controller(*data.controllers[s]);
    }
    if (data.dataType == ATCBOOKINGS || data.dataType == UNIFIED) {
        if(dataType == WHAZZUP) dataType = UNIFIED;

        bookedControllers.clear();
        foreach(const BookedController *bc, data.bookedControllers)
            bookedControllers.append(new BookedController(*bc));

        bookingsTime = QDateTime(data.bookingsTime);
        predictionBasedOnBookingsTime = QDateTime(data.predictionBasedOnBookingsTime);
    }
    qDebug() << "WhazzupData::assignFrom() -- finished";
}

void WhazzupData::updatePilotsFrom(const WhazzupData &data) {
    qDebug() << "WhazzupData::updatePilotsFrom()";
    foreach(const QString s, pilots.keys()) { // remove pilots that are no longer there
        if(!data.pilots.contains(s)) {
            foreach(const Pilot *p, pilots.values(s)) // there might be several...
                delete p;
            pilots.remove(s);
        }
    }
    foreach(const QString s, data.pilots.keys()) {
        if(!pilots.contains(s)) { // new pilots
            // create a new copy of new pilot
            Pilot *p = new Pilot(*data.pilots[s]);
            pilots[s] = p;
        } else { // existing pilots: data saved in the object needs to be transferred
            data.pilots[s]->showDepDestLine              = pilots[s]->showDepDestLine;
            data.pilots[s]->routeWaypointsCache          = pilots[s]->routeWaypointsCache;
            data.pilots[s]->routeWaypointsPlanDepCache   = pilots[s]->routeWaypointsPlanDepCache;
            data.pilots[s]->routeWaypointsPlanDestCache  = pilots[s]->routeWaypointsPlanDestCache;
            data.pilots[s]->routeWaypointsPlanRouteCache = pilots[s]->routeWaypointsPlanRouteCache;
            data.pilots[s]->checkStatus();

            if ((pilots[s]->lat != 0 || pilots[s]->lon != 0) &&
                (pilots[s]->lat != data.pilots[s]->lat ||
                 pilots[s]->lon != data.pilots[s]->lon))
                *pilots[s] = *data.pilots[s];
        }
    }

    foreach(const QString s, bookedPilots.keys()) { // remove pilots that are no longer there
        if(!data.bookedPilots.contains(s)) {
            foreach(const Pilot *p, bookedPilots.values(s)) // there might be several...
                delete p;
            bookedPilots.remove(s);
        }
    }
    foreach(const QString s, data.bookedPilots.keys()) {
        if(!bookedPilots.contains(s)) { // new pilots
            Pilot *p = new Pilot(*data.bookedPilots[s]);
            bookedPilots[s] = p;
        } else // existing pilots
            *bookedPilots[s] = *data.bookedPilots[s];
    }
    qDebug() << "WhazzupData::updatePilotsFrom() -- finished";
}

void WhazzupData::updateControllersFrom(const WhazzupData &data) {
    qDebug() << "WhazzupData::updateControllersFrom()";
    foreach(const QString s, controllers.keys()) {
        if(!data.controllers.contains(s)) {
            // remove controllers that are no longer there
            delete controllers[s];
            controllers.remove(s);
        }
    }
    foreach(const QString s, data.controllers.keys()) {
        if(!controllers.contains(s)) {
            // create a new copy of new controllers
            Controller *c = new Controller(*data.controllers[s]);
            controllers[c->label] = c;
        } else // controller already exists, assign values from data
            *controllers[s] = *data.controllers[s];
    }
    qDebug() << "WhazzupData::updateControllersFrom() -- finished";
}

void WhazzupData::updateBookedControllersFrom(const WhazzupData &data) {
    qDebug() << "WhazzupData::updateBookedControllersFrom()";
    bookedControllers.clear();
    foreach(const BookedController *bc, data.bookedControllers)
        bookedControllers.append(new BookedController(*bc));
    qDebug() << "WhazzupData::updateBookedControllersFrom() -- finished";
}

void WhazzupData::updateFrom(const WhazzupData &data) {
    qDebug() << "WhazzupData::updateFrom()";
    if(this == &data)
        return;

    if(data.isNull())
        return;

    if (data.dataType == WHAZZUP || data.dataType == UNIFIED) {
        if(dataType == ATCBOOKINGS) dataType = UNIFIED;
        updatePilotsFrom(data);
        updateControllersFrom(data);

        servers = data.servers;
        voiceServers = data.voiceServers;
        whazzupVersion = data.whazzupVersion;
        whazzupTime = data.whazzupTime;
        updateEarliest = data.updateEarliest;
        predictionBasedOnTime = data.predictionBasedOnTime;
        friendsLatLon = data.friendsLatLon;
    }
    if (data.dataType == ATCBOOKINGS || data.dataType == UNIFIED) {
        if(dataType == WHAZZUP) dataType = UNIFIED;
        updateBookedControllersFrom(data);
        bookingsTime = data.bookingsTime;
        predictionBasedOnBookingsTime = data.predictionBasedOnBookingsTime;
    }
    qDebug() << "WhazzupData::updateFrom() -- finished";
}

QSet<Controller*> WhazzupData::activeSectors() const {
    qDebug() << "WhazzupData::activeSectors()";
    QSet<Controller*> result;
    QList<Controller*> controllerList = controllers.values();
    foreach(Controller *c, controllerList)
        if (c->sector != 0)
            result.insert(c);

    qDebug() << "WhazzupData::activeSectors() -- finished";
    return result;
}

void WhazzupData::accept(MapObjectVisitor* visitor) const {
    foreach(Controller *c, controllers)
        visitor->visit(c);
    foreach(Pilot *p, pilots)
        visitor->visit(p);
    foreach(Pilot *b, bookedPilots)
        visitor->visit(b);
}
