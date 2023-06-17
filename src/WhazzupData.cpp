#include "WhazzupData.h"

#include "Airport.h"
#include "BookedController.h"
#include "Controller.h"
#include "NavData.h"
#include "Pilot.h"
#include "Sector.h"
#include "Settings.h"

WhazzupData::WhazzupData()
    : servers(QList<QStringList>()),
      updateEarliest(QDateTime()), whazzupTime(QDateTime()), bookingsTime(QDateTime()),
      _dataType(UNIFIED) {}

WhazzupData::WhazzupData(QByteArray* bytes, WhazzupType type)
    : servers(QList<QStringList>()),
      updateEarliest(QDateTime()), whazzupTime(QDateTime()),
      bookingsTime(QDateTime()) {
    qDebug() << type << "[NONE, WHAZZUP, ATCBOOKINGS, UNIFIED]";
    _dataType = type;
    int reloadInSec = Settings::downloadInterval();
    QJsonDocument data = QJsonDocument::fromJson(*bytes);
    if (data.isNull()) {
        qDebug() << "Couldn't parse JSON";
    } else if (type == WHAZZUP) {
        QJsonObject json = data.object();
        if (json.contains("general") && json["general"].isObject()) {
            QJsonObject generalObject = json["general"].toObject();
            if (generalObject.contains("update_timestamp") && generalObject["update_timestamp"].isString()) {
                whazzupTime = QDateTime::fromString(generalObject["update_timestamp"].toString(), Qt::ISODate);
            }
        }
        if (!whazzupTime.isValid()) {
            // Assume it's the current time
            whazzupTime = QDateTime::currentDateTime();
        }
        if (json.contains("servers") && json["servers"].isArray()) {
            QJsonArray serversArray = json["servers"].toArray();
            for (int i = 0; i < serversArray.size(); ++i) {
                QJsonObject serverObject = serversArray[i].toObject();
                if (
                    serverObject.contains("ident") && serverObject["ident"].isString()
                    && serverObject.contains("hostname_or_ip") && serverObject["hostname_or_ip"].isString()
                    && serverObject.contains("location") && serverObject["location"].isString()
                    && serverObject.contains("name") && serverObject["name"].isString()
                    && serverObject.contains("clients_connection_allowed") && serverObject["clients_connection_allowed"].isDouble()
                ) {
                    QStringList server;
                    server.append(serverObject["ident"].toString());
                    server.append(serverObject["hostname_or_ip"].toString());
                    server.append(serverObject["location"].toString());
                    server.append(serverObject["name"].toString());
                    server.append(serverObject["clients_connection_allowed"].toString());
                    servers += server;
                }
            }
        }
        if (json.contains("pilots") && json["pilots"].isArray()) {
            QJsonArray pilotsArray = json["pilots"].toArray();
            for (int i = 0; i < pilotsArray.size(); ++i) {
                QJsonObject pilotObject = pilotsArray[i].toObject();
                Pilot* p = new Pilot(pilotObject, this);
                pilots[p->callsign] = p;
            }
        }

        if (json.contains("controllers") && json["controllers"].isArray()) {
            QJsonArray controllersArray = json["controllers"].toArray();
            for (int i = 0; i < controllersArray.size(); ++i) {
                QJsonObject controllerObject = controllersArray[i].toObject();
                Controller* c = new Controller(controllerObject, this);
                controllers[c->callsign] = c;
            }
        }

        if (json.contains("atis") && json["atis"].isArray()) {
            QJsonArray atisArray = json["atis"].toArray();
            for (int i = 0; i < atisArray.size(); ++i) {
                QJsonObject atisObject = atisArray[i].toObject();
                Controller* c = new Controller(atisObject, this);
                controllers[c->callsign] = c;
            }
        }

        if (json.contains("prefiles") && json["prefiles"].isArray()) {
            QJsonArray prefilesArray = json["prefiles"].toArray();
            for (int i = 0; i < prefilesArray.size(); ++i) {
                QJsonObject prefileObject = prefilesArray[i].toObject();
                Pilot* p = new Pilot(prefileObject, this);
                bookedPilots[p->callsign] = p;
            }
        }

        // It looks like these were meant to be used as a bit mask but the IDs were chosen poorly
        if (json.contains("ratings") && json["ratings"].isArray()) {
            foreach (const auto rating, json["ratings"].toArray()) {
                auto o = rating.toObject();
                ratings.insert(o["id"].toInt(), o["short"].toString());
            }
            qDebug() << "ratings:" << ratings;
        }

        if (json.contains("pilot_ratings") && json["pilot_ratings"].isArray()) {
            foreach (const auto rating, json["pilot_ratings"].toArray()) {
                auto o = rating.toObject();
                pilotRatings.insert(o["id"].toInt(), o["short_name"].toString());
            }
            qDebug() << "pilotRatings:" << pilotRatings;
        }

        if (json.contains("military_ratings") && json["military_ratings"].isArray()) {
            foreach (const auto rating, json["military_ratings"].toArray()) {
                auto o = rating.toObject();
                militaryRatings.insert(o["id"].toInt(), o["short_name"].toString());
            }
            qDebug() << "militaryRatings:" << militaryRatings;
        }
    } else if (type == ATCBOOKINGS) {
        QJsonArray json = data.array();
        for (int i = 0; i < json.size(); ++i) {
            QJsonObject bookedControllerJson = json[i].toObject();
            BookedController* bc = new BookedController(bookedControllerJson);
            bookedControllers.append(bc);
        }
        bookingsTime = QDateTime::currentDateTime();
    } else {
        // Try again in 15 seconds
        updateEarliest = QDateTime::currentDateTime().addSecs(15);
    }
    // set the earliest time the server will have new data
    if (whazzupTime.isValid() && reloadInSec > 0) {
        updateEarliest = whazzupTime.addSecs(reloadInSec).toUTC();
    }
    qDebug() << "-- finished";
}

// faking WhazzupData based on valid data and a predictTime
WhazzupData::WhazzupData(const QDateTime predictTime, const WhazzupData &data)
    : servers(QList<QStringList>()),
      updateEarliest(QDateTime()), whazzupTime(QDateTime()),
      bookingsTime(QDateTime()), predictionBasedOnTime(QDateTime()),
      predictionBasedOnBookingsTime(QDateTime()) {
    qDebug() << predictTime;

    whazzupTime = predictTime;
    predictionBasedOnTime = QDateTime(data.whazzupTime);
    predictionBasedOnBookingsTime = QDateTime(data.bookingsTime);

    ratings = data.ratings;
    pilotRatings = data.pilotRatings;
    militaryRatings = data.militaryRatings;

    _dataType = data._dataType;
    // so now lets fake some controllers
    foreach (const BookedController* bc, data.bookedControllers) {
        // only ones booked for the selected time
        if (bc->starts() <= predictTime && bc->ends() >= predictTime) {
            QJsonObject controllerObject;

            controllerObject["callsign"] = bc->callsign;
            controllerObject["name"] = bc->realName();
            controllerObject["cid"] = bc->userId.toInt();
            controllerObject["facility"] = bc->facilityType;
            controllerObject["rating"] = -99; // strictly out of API range
            controllerObject["frequency"] = "?"; // cannot be empty

            QJsonArray atisLines;
            atisLines.append(
                QString("BOOKED from %1, online until %2")
                .arg(bc->starts().toString("HHmm'z'"), bc->ends().toString("HHmm'z'"))
            );
            atisLines.append(bc->bookingInfoStr);
            controllerObject["text_atis"] = atisLines;

            controllerObject["logon_time"] = bc->timeConnected.toString(Qt::ISODate);

            controllerObject["server"] = "BOOKED SESSION";

            controllerObject["visual_range"] = 0;

            controllers[bc->callsign] = new Controller(controllerObject, this);
        }
    }
    qDebug() << "added" << controllers.size() << "bookedControllers as fake controllers";

    // let controllers be in until he states in his Controller Info also if only found in Whazzup, not booked
    foreach (const Controller* c, data.controllers) {
        // standard for online controllers: 4 min
        QDateTime showUntil = predictionBasedOnTime.addSecs(4 * 60);
        if (c->assumeOnlineUntil.isValid()) {
            // only if before stated leave-time
            if (predictionBasedOnTime.secsTo(c->assumeOnlineUntil) >= 0) {
                showUntil = c->assumeOnlineUntil;
            }
        }

        if (predictTime <= showUntil && predictTime >= predictionBasedOnTime) {
            controllers[c->callsign] = new Controller(*c);
        }
    }

    QDateTime startTime, endTime = QDateTime();
    double startLat, startLon, endLat, endLon = 0.;
    int altitude = 0;

    foreach (const Pilot* p, data.allPilots()) {
        //if (p == 0) continue;
        if (!p->eta().isValid()) {
            continue; // no ETA, no prediction...
        }
        if (!p->etd().isValid() && predictTime < predictionBasedOnTime) {
            continue; // no ETD, difficult prediction. Before the whazzupTime, no prediction...
        }
        if (p->destAirport() == 0) {
            continue; // sorry, no magic available yet. Just let him fly the last heading until etaPlan()?
                      // Does not make
                      // sense
        }
        if (p->etd() > predictTime || p->eta() < predictTime) {
            if (p->flightStatus() == Pilot::PREFILED && p->etd() > predictTime) { // we want prefiled before
                                                                                  // their
                //departure as in non-Warped view
                Pilot* np = new Pilot(*p);
                np->whazzupTime = QDateTime(predictTime);
                bookedPilots[np->callsign] = np; // just copy him over
                continue;
            }
            continue; // not on the map on the selected time
        }
        if (p->flightStatus() == Pilot::PREFILED) {
            // if we dont know where a prefiled comes from, no magic available
            if (p->depAirport() == 0 || p->destAirport() == 0) {
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
        if (p->planAlt.toInt() != 0) {
            altitude = p->defuckPlanAlt(p->planAlt);
        }

        // position
        double fraction = (double) startTime.secsTo(predictTime)
            / startTime.secsTo(endTime);
        QPair<double, double> pos = NavData::greatCircleFraction(
            startLat, startLon,
            endLat, endLon, fraction
        );

        double dist = NavData::distance(startLat, startLon, endLat, endLon);
        double enrouteHrs = ((double) startTime.secsTo(endTime)) / 3600.0;
        if (qFuzzyIsNull(enrouteHrs)) {
            enrouteHrs = 0.1;
        }
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

        pilots[np->callsign] = np;
    }
    qDebug() << "-- finished";
}

WhazzupData::WhazzupData(const WhazzupData &data) {
    assignFrom(data);
}

WhazzupData::~WhazzupData() {
    foreach (const QString s, pilots.keys()) {
        delete pilots[s];
    }
    pilots.clear();

    foreach (const QString s, bookedPilots.keys()) {
        delete bookedPilots[s];
    }
    bookedPilots.clear();

    foreach (const QString s, controllers.keys()) {
        delete controllers[s];
    }
    controllers.clear();

    foreach (const BookedController* bc, bookedControllers) {
        delete bc;
    }
    bookedControllers.clear();
}

WhazzupData &WhazzupData::operator=(const WhazzupData &data) {
    assignFrom(data);
    return *this;
}

bool WhazzupData::isNull() const {
    return whazzupTime.isNull() && bookingsTime.isNull();
}

void WhazzupData::assignFrom(const WhazzupData &data) {
    qDebug();
    if (this == &data) {
        return;
    }

    ratings = data.ratings;
    pilotRatings = data.pilotRatings;
    militaryRatings = data.militaryRatings;

    if (data._dataType == WHAZZUP || data._dataType == UNIFIED) {
        if (_dataType == ATCBOOKINGS) {
            _dataType = UNIFIED;
        }
        servers = data.servers;
        whazzupTime = data.whazzupTime;
        predictionBasedOnTime = data.predictionBasedOnTime;
        updateEarliest = data.updateEarliest;

        pilots.clear();
        foreach (const QString s, data.pilots.keys()) {
            pilots[s] = new Pilot(*data.pilots[s]);
        }

        bookedPilots.clear();
        foreach (const QString s, data.bookedPilots.keys()) {
            bookedPilots[s] = new Pilot(*data.bookedPilots[s]);
        }

        controllers.clear();
        foreach (const QString s, data.controllers.keys()) {
            controllers[s] = new Controller(*data.controllers[s]);
        }
    }
    if (data._dataType == ATCBOOKINGS || data._dataType == UNIFIED) {
        if (_dataType == WHAZZUP) {
            _dataType = UNIFIED;
        }

        bookedControllers.clear();
        foreach (const BookedController* bc, data.bookedControllers) {
            bookedControllers.append(new BookedController(*bc));
        }

        bookingsTime = QDateTime(data.bookingsTime);
        predictionBasedOnBookingsTime = QDateTime(data.predictionBasedOnBookingsTime);
    }
    qDebug() << "-- finished";
}

void WhazzupData::updatePilotsFrom(const WhazzupData &data) {
    qDebug();
    foreach (const QString s, pilots.keys()) { // remove pilots that are no longer there
        if (!data.pilots.contains(s)) {
            delete pilots.value(s);
            pilots.remove(s);
        }
    }
    foreach (const QString s, data.pilots.keys()) {
        if (!pilots.contains(s)) { // new pilots
            // create a new copy of new pilot
            Pilot* p = new Pilot(*data.pilots[s]);
            pilots[s] = p;
        } else { // existing pilots: data saved in the object needs to be transferred
            data.pilots[s]->showRoute = pilots[s]->showRoute;
            data.pilots[s]->routeWaypointsCache = pilots[s]->routeWaypointsCache;
            data.pilots[s]->routeWaypointsPlanDepCache = pilots[s]->routeWaypointsPlanDepCache;
            data.pilots[s]->routeWaypointsPlanDestCache = pilots[s]->routeWaypointsPlanDestCache;
            data.pilots[s]->routeWaypointsPlanRouteCache = pilots[s]->routeWaypointsPlanRouteCache;
            data.pilots[s]->checkStatus();

            *pilots[s] = *data.pilots[s];
        }
    }

    foreach (const QString s, bookedPilots.keys()) { // remove pilots that are no longer there
        if (!data.bookedPilots.contains(s)) {
            delete bookedPilots.value(s);
            bookedPilots.remove(s);
        }
    }
    foreach (const QString s, data.bookedPilots.keys()) {
        if (!bookedPilots.contains(s)) { // new pilots
            Pilot* p = new Pilot(*data.bookedPilots[s]);
            bookedPilots[s] = p;
        } else { // existing pilots
            *bookedPilots[s] = *data.bookedPilots[s];
        }
    }
    qDebug() << "-- finished";
}

void WhazzupData::updateControllersFrom(const WhazzupData &data) {
    qDebug();
    foreach (const QString s, controllers.keys()) {
        if (!data.controllers.contains(s)) {
            // remove controllers that are no longer there
            delete controllers[s];
            controllers.remove(s);
        }
    }
    foreach (const QString s, data.controllers.keys()) {
        if (!controllers.contains(s)) {
            // create a new copy of new controllers
            Controller* c = new Controller(*data.controllers[s]);
            controllers[c->callsign] = c;
        } else { // controller already exists, assign values from data
            *controllers[s] = *data.controllers[s];
        }
    }
    qDebug() << "-- finished";
}

void WhazzupData::updateBookedControllersFrom(const WhazzupData &data) {
    qDebug() << data.bookedControllers.size() << "bookedControllers";
    bookedControllers.clear();
    foreach (const BookedController* bc, data.bookedControllers) {
        bookedControllers.append(new BookedController(*bc));
    }
    qDebug() << "-- finished";
}

void WhazzupData::updateFrom(const WhazzupData &data) {
    qDebug();
    if (this == &data) {
        return;
    }

    if (data.isNull()) {
        return;
    }

    ratings = data.ratings;
    pilotRatings = data.pilotRatings;
    militaryRatings = data.militaryRatings;

    if (data._dataType == WHAZZUP || data._dataType == UNIFIED) {
        if (_dataType == ATCBOOKINGS) {
            _dataType = UNIFIED;
        }
        updatePilotsFrom(data);
        updateControllersFrom(data);

        servers = data.servers;
        whazzupTime = data.whazzupTime;
        updateEarliest = data.updateEarliest;
        predictionBasedOnTime = data.predictionBasedOnTime;
    }
    if (data._dataType == ATCBOOKINGS || data._dataType == UNIFIED) {
        if (_dataType == WHAZZUP) {
            _dataType = UNIFIED;
        }
        updateBookedControllersFrom(data);
        bookingsTime = data.bookingsTime;
        predictionBasedOnBookingsTime = data.predictionBasedOnBookingsTime;
    }
    qDebug() << "-- finished";
}

QSet<Controller*> WhazzupData::controllersWithSectors() const {
    QSet<Controller*> result;
    foreach (Controller* c, controllers.values()) {
        if (c->sector != 0) {
            result.insert(c);
        }
    }

    return result;
}

QList<Pilot*> WhazzupData::allPilots() const {
    return bookedPilots.values() + pilots.values();
}

QList<QPair<double, double> > WhazzupData::friendsLatLon() const {
    QStringList friends = Settings::friends();
    QList<QPair<double, double> > result;
    foreach (Controller* c, controllers.values()) {
        if (c->isAtis()) {
            continue;
        }
        if (friends.contains(c->userId)) {
            result.append(QPair<double, double>(c->lat, c->lon));
        }
    }

    foreach (Pilot* p, pilots.values()) {
        if (friends.contains(p->userId)) {
            result.append(QPair<double, double>(p->lat, p->lon));
        }
    }

    return result;
}

void WhazzupData::accept(MapObjectVisitor* visitor) const {
    foreach (Controller* c, controllers) {
        visitor->visit(c);
    }
    foreach (Pilot* p, pilots) {
        visitor->visit(p);
    }
    foreach (Pilot* b, bookedPilots) {
        visitor->visit(b);
    }
}

Pilot* WhazzupData::findPilot(const QString &callsign) const {
    Pilot* pilot = pilots.value(callsign);
    if (pilot != 0) {
        return pilot;
    }
    return bookedPilots.value(callsign, 0);
}
