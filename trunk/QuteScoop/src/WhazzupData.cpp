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
    connectedServerList(QList<QStringList>()),
    connectedVoiceServerList(QList<QStringList>()),
    whazzupVersion(0),
    whazzupTime(QDateTime()),
    bookingsTime(QDateTime()),
    updateEarliest(QDateTime()),
    dataType(UNIFIED)
{
}

WhazzupData::WhazzupData(QBuffer* buffer, WhazzupType type):
    connectedServerList(QList<QStringList>()),
    connectedVoiceServerList(QList<QStringList>()),
    whazzupVersion(0),
    whazzupTime(QDateTime()),
    bookingsTime(QDateTime()),
    updateEarliest(QDateTime())
{
    qDebug() << "WhazzupData::WhazzupData(buffer)" << type << "[NONE, WHAZZUP, ATCBOOKINGS, UNIFIED]";
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    dataType = type;
    int reloadInMin;
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
                connectedServerList += list;
                //; !SERVERS section -         ident:hostname_or_IP:location:name:clients_connection_allowed:
                //EUROPE-C2:88.198.19.202:Europe:Center Europe Server Two:1:
            }
            break;
        case STATE_VOICESERVERS: {
                QStringList list = line.split(':');
                if(list.size() < 5)
                    continue;
                connectedVoiceServerList += list;
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

    // set the earliest time the server will have new data
    if (whazzupTime.isValid() && reloadInMin > 0) {
        updateEarliest = whazzupTime.addSecs(reloadInMin * 60).toUTC();
        //qDebug() << "next update in" << reloadInMin << "min from" << whazzupTime << ":" << updateEarliest << QDateTime::currentDateTimeUtc().secsTo(updateEarliest);
    }
    qApp->restoreOverrideCursor();
}

WhazzupData::WhazzupData(const QDateTime predictTime, const WhazzupData& data):
    connectedServerList(QList<QStringList>()),
    connectedVoiceServerList(QList<QStringList>()),
    whazzupVersion(0),
    whazzupTime(QDateTime()),
    bookingsTime(QDateTime()),
    predictionBasedOnTime(QDateTime()),
    predictionBasedOnBookingsTime(QDateTime()),
    updateEarliest(QDateTime())
{
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    qDebug() << "WhazzupData::WhazzupData(predictTime)" << predictTime;

    whazzupVersion = data.whazzupVersion;
    whazzupTime = predictTime;
    predictionBasedOnTime = QDateTime(data.whazzupTime);
    predictionBasedOnBookingsTime = QDateTime(data.bookingsTime);

    dataType = data.dataType;
    // so now lets fake some controllers
    QList<BookedController*> bc = data.getBookedControllers();
    for (int i = 0; i < bc.size(); i++) {
        // keep GUI responsive - leads to hangups?
        qApp->processEvents();

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
        // keep GUI responsive - leads to hangups?
        //qApp->processEvents();

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
        // keep GUI responsive - leads to hangups?
        //qApp->processEvents();

        if (p[i] == 0)
            continue;
        if (!p[i]->eta().isValid())
            continue; // no ETA, no prediction...
        if (!p[i]->etd().isValid() && predictTime < predictionBasedOnTime)
            continue; // no ETD, difficult prediction. Before the WhazzupTime, no prediction...
        if(p[i]->destAirport() == 0)
            continue; // sorry, no magic available yet. Just let him fly the last heading until etaPlan()? Does not make sense
        if(p[i]->etd() > predictTime || p[i]->eta() < predictTime) {
            if (p[i]->flightStatus() == Pilot::PREFILED && p[i]->etd() > predictTime) { // we want prefiled before their
                                                                                        //departure as in non-Warped view
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
        QPair<double, double> pos = NavData::greatCircleFraction(startLat, startLon, endLat, endLon, fraction);

        double dist = NavData::distance(startLat, startLon, endLat, endLon);
        double enrouteHrs = ((double) startTime.secsTo(endTime)) / 3600.0;
        if (qFuzzyIsNull(enrouteHrs)) enrouteHrs = 0.1;
        double groundspeed = dist / enrouteHrs;
        double trueHeading = NavData::courseTo(pos.first, pos.second, endLat, endLon);

        // create Pilot instance and assign values
        Pilot* np = new Pilot(*p[i]);
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

WhazzupData::WhazzupData(const WhazzupData& data) {
    assignFrom(data);
}

WhazzupData& WhazzupData::operator=(const WhazzupData& data) {
    assignFrom(data);
    return *this;
}

void WhazzupData::assignFrom(const WhazzupData& data) {
    qDebug() << "WhazzupData::assignFrom()";
    if(this == &data)
        return;

    if (data.dataType == WHAZZUP || data.dataType == UNIFIED) {
        if(dataType == ATCBOOKINGS) dataType = UNIFIED;
        connectedServerList = data.connectedServerList;
        connectedVoiceServerList = data.connectedVoiceServerList;
        whazzupTime = data.whazzupTime;
        predictionBasedOnTime = data.predictionBasedOnTime;
        updateEarliest = data.updateEarliest;

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
    qDebug() << "WhazzupData::assignFrom() -- finished";
}

void WhazzupData::updatePilotsFrom(const WhazzupData& data) {
    qDebug() << "WhazzupData::updatePilotsFrom()";
    QList<QString> callsigns = pilots.keys();
    for(int i = 0; i < callsigns.size(); i++) { // remove pilots that are no longer there
        if(!data.pilots.contains(callsigns[i])) {
            foreach (Pilot *p, pilots.values(callsigns[i])) // there might be several...
                delete p;
            pilots.remove(callsigns[i]);
        }
    }
    callsigns = data.pilots.keys();
    for(int i = 0; i < callsigns.size(); i++) {
        if(!pilots.contains(callsigns[i])) { // new pilots
            // create a new copy of new pilot
            Pilot *p = new Pilot(*data.pilots[callsigns[i]]);
            pilots[callsigns[i]] = p;
        } else { // existing pilots: data saved in the object needs to be transferred
            data.pilots[callsigns[i]]->showDepDestLine              = pilots[callsigns[i]]->showDepDestLine;
            data.pilots[callsigns[i]]->routeWaypointsCache          = pilots[callsigns[i]]->routeWaypointsCache;
            data.pilots[callsigns[i]]->routeWaypointsPlanDepCache   = pilots[callsigns[i]]->routeWaypointsPlanDepCache;
            data.pilots[callsigns[i]]->routeWaypointsPlanDestCache  = pilots[callsigns[i]]->routeWaypointsPlanDestCache;
            data.pilots[callsigns[i]]->routeWaypointsPlanRouteCache = pilots[callsigns[i]]->routeWaypointsPlanRouteCache;

            if ((pilots[callsigns[i]]->lat != 0 || pilots[callsigns[i]]->lon != 0) &&
                (pilots[callsigns[i]]->lat != data.pilots[callsigns[i]]->lat ||
                 pilots[callsigns[i]]->lon != data.pilots[callsigns[i]]->lon))
            *pilots[callsigns[i]] = *data.pilots[callsigns[i]];
        }
    }

    callsigns = bookedpilots.keys();
    for(int i = 0; i < callsigns.size(); i++) { // remove pilots that are no longer there
        if(!data.bookedpilots.contains(callsigns[i])) {
            foreach (Pilot *p, bookedpilots.values(callsigns[i])) // there might be several...
                delete p;
            bookedpilots.remove(callsigns[i]);
        }
    }
    callsigns = data.bookedpilots.keys();
    for(int i = 0; i < callsigns.size(); i++) {
        if(!bookedpilots.contains(callsigns[i])) { // new pilots
            Pilot *p = new Pilot(*data.bookedpilots[callsigns[i]]);
            bookedpilots[callsigns[i]] = p;
        } else { // existing pilots
            *bookedpilots[callsigns[i]] = *data.bookedpilots[callsigns[i]];
        }
    }
    qDebug() << "WhazzupData::updatePilotsFrom() -- finished";
}

void WhazzupData::updateControllersFrom(const WhazzupData& data) {
    qDebug() << "WhazzupData::updateControllersFrom()";
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
    qDebug() << "WhazzupData::updateControllersFrom() -- finished";
}

void WhazzupData::updateBookedControllersFrom(const WhazzupData& data) {
    qDebug() << "WhazzupData::updateBookedControllersFrom()";
    bookedcontrollers.clear();
    for (int i = 0; i < data.bookedcontrollers.size(); i++) {
        bookedcontrollers.append(new BookedController(*data.bookedcontrollers[i]));
    }
    qDebug() << "WhazzupData::updateBookedControllersFrom() -- finished";
}

void WhazzupData::updateFrom(const WhazzupData& data) {
    qDebug() << "WhazzupData::updateFrom()";
    if(this == &data)
        return;

    if(data.isNull())
        return;

    if (data.dataType == WHAZZUP || data.dataType == UNIFIED) {
        if(dataType == ATCBOOKINGS) dataType = UNIFIED;
        updatePilotsFrom(data);
        updateControllersFrom(data);

        connectedServerList = data.connectedServerList;
        connectedVoiceServerList = data.connectedVoiceServerList;
        whazzupVersion = data.whazzupVersion;
        whazzupTime = data.whazzupTime;
        updateEarliest = data.updateEarliest;
        predictionBasedOnTime = data.predictionBasedOnTime;
    }
    if (data.dataType == ATCBOOKINGS || data.dataType == UNIFIED) {
        if(dataType == WHAZZUP) dataType = UNIFIED;
        updateBookedControllersFrom(data);
        bookingsTime = data.bookingsTime;
        predictionBasedOnBookingsTime = data.predictionBasedOnBookingsTime;
    }
    qDebug() << "WhazzupData::updateFrom() -- finished";
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
    qDebug() << "WhazzupData::activeSectors()";
    QList<Controller*> result;
    QList<Controller*> controllerList = controllers.values();
    for(int i = 0; i < controllerList.size(); i++) {
        if (controllerList[i]->sector != 0)
            result.append(controllerList[i]);
    }

    qDebug() << "WhazzupData::activeSectors() -- finished";
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
