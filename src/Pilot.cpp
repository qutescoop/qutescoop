#include "Pilot.h"

#include "Airac.h"
#include "Airport.h"
#include "Client.h"
#include "helpers.h"
#include "NavData.h"
#include "Settings.h"
#include "Whazzup.h"
#include "dialogs/PilotDetails.h"
#include "src/mustache/Renderer.h"

#include <QJsonObject>

int Pilot::altToFl(int alt_ft, int qnh_mb) {
    float diff = qnh_mb - 1013.25;

    // https://edwilliams.org/avform147.htm
    // https://www.pprune.org/tech-log/123900-temp-height-change-per-millibar.html

    // 30ft per mb - this is what xPilot uses for XP11
    float fl_ft = alt_ft - diff * 30;

    return qRound(fl_ft / 100.);
}

Pilot::Pilot(const QJsonObject& json, const WhazzupData* whazzup)
    : MapObject(), Client(json, whazzup),
      airline(0) {
    whazzupTime = QDateTime(whazzup->whazzupTime); // need some local reference to that

    lat = json["latitude"].toDouble();
    lon = json["longitude"].toDouble();

    pilotRating = json["pilot_rating"].toInt();
    militaryRating = json["military_rating"].toInt();

    altitude = json["altitude"].toInt();
    groundspeed = json["groundspeed"].toInt();

    QJsonObject flightPlan = json["flight_plan"].toObject();
    // The JSON data provides 3 different aircraft data
    // 1: The full ICAO Data, this is too long to display
    // 2: The FAA Data, this is what was displayed previously
    // 3: The short Data, consisting only of the aircraft code
    planAircraftFull = flightPlan["aircraft"].toString();
    planAircraft = flightPlan["aircraft_short"].toString();
    planAircraftFaa = flightPlan["aircraft_faa"].toString();

    planTAS = flightPlan["cruise_tas"].toString();
    planDep = flightPlan["departure"].toString();
    planAlt = flightPlan["altitude"].toString();
    planDest = flightPlan["arrival"].toString();

    QRegExp _airlineRegEx("([A-Z]{3})[0-9].*");
    if (_airlineRegEx.exactMatch(callsign)) {
        auto _capturedTexts = _airlineRegEx.capturedTexts();
        airline = NavData::instance()->airlines.value(_capturedTexts[1], 0);
    }

    transponder = json["transponder"].toString();
    transponderAssigned = flightPlan["assigned_transponder"].toString();

    planRevision = QString::number(flightPlan["revision_id"].toInt());
    planFlighttype = flightPlan["flight_rules"].toString();
    planDeptime = flightPlan["deptime"].toString();
    planActtime = flightPlan["deptime"].toString(); // The new data doesn't provide the actual departure

    QString timeEnroute = flightPlan["enroute_time"].toString();
    QString timeFuel = flightPlan["fuel_time"].toString();

    QString tmpStr = timeEnroute.left(2);
    if (tmpStr.isNull()) {
        planEnroute_hrs = -1;
    } else {
        planEnroute_hrs = tmpStr.toInt();
    }
    planEnroute_mins = timeEnroute.rightRef(2).toInt();
    planFuel_hrs = timeFuel.leftRef(2).toInt();
    planFuel_mins = timeFuel.rightRef(2).toInt();
    planAltAirport = flightPlan["alternate"].toString();
    planRemarks = flightPlan["remarks"].toString();
    planRoute = flightPlan["route"].toString();

    trueHeading = json["heading"].toInt();
    qnh_inHg = json["qnh_i_hg"].toDouble();
    qnh_mb = json["qnh_mb"].toInt();
    // day of flight
    if (!QTime::fromString(planDeptime, "HHmm").isValid()) { // no Plan ETA given: maybe some more magic needed here
        dayOfFlight = whazzupTime.date();
    } else if ((QTime::fromString(planDeptime, "HHmm").hour() - whazzupTime.time().hour()) % 24 <= 2) {
        // allow for early birds (up to 2 hours before planned departure)
        dayOfFlight = whazzupTime.date();
    } else {
        dayOfFlight = whazzupTime.date().addDays(-1); // started the day before

    }

    checkStatus();
}

Pilot::~Pilot() {
    MustacheQs::Renderer::teardownContext(this);
}

void Pilot::showDetailsDialog() {
    PilotDetails* infoDialog = PilotDetails::instance();
    infoDialog->refresh(this);
    infoDialog->show();
    infoDialog->raise();
    infoDialog->setFocus();
    infoDialog->activateWindow();
}

QString Pilot::mapLabel() const {
    auto tmpl = Settings::pilotPrimaryContent();
    return MustacheQs::Renderer::render(tmpl, (QObject*) this);
}

QString Pilot::mapLabelHovered() const {
    auto tmpl = Settings::pilotPrimaryContentHovered();
    return MustacheQs::Renderer::render(tmpl, (QObject*) this);
}

QStringList Pilot::mapLabelSecondaryLines() const {
    auto tmpl = Settings::pilotSecondaryContent();
    return Helpers::linesFilteredTrimmed(
        MustacheQs::Renderer::render(tmpl, (QObject*) this)
    );
}

QStringList Pilot::mapLabelSecondaryLinesHovered() const {
    auto tmpl = Settings::pilotSecondaryContentHovered();
    return Helpers::linesFilteredTrimmed(
        MustacheQs::Renderer::render(tmpl, (QObject*) this)
    );
}


Pilot::FlightStatus Pilot::flightStatus() const {
    Airport* dep = depAirport();
    Airport* dst = destAirport();

    if (qFuzzyIsNull(lat) && qFuzzyIsNull(lon)) {
        return PREFILED;
    }

    // flying?
    const bool flying = groundspeed > 50 || altitude > 9000;

    if (dep == 0 || dst == 0) {
        if (flying) {
            return EN_ROUTE;
        }
        return BUSH;
    }

    const double totalDist = NavData::distance(dep->lat, dep->lon, dst->lat, dst->lon);
    const double distDone = NavData::distance(lat, lon, dep->lat, dep->lon);
    const double distRemaining = totalDist - distDone;

    // arriving?
    const bool arriving = distRemaining < 50 && distRemaining < distDone;

    // departing?
    const bool departing = distDone < 50 && distRemaining > distDone;

    // BOARDING: !flying, speed = 0, departing
    // GROUND_DEP: !flying, speed > 0, departing
    // DEPARTING: flying, departing
    // EN_ROUTE: flying, !departing, !arriving
    // ARRIVING: flying, arriving
    // GROUND_ARR: !flying, speed > 0, arriving
    // BLOCKED: !flying, speed = 0, arriving
    // PREFILED: !flying, lat=0, lon=0

    if (!flying && groundspeed == 0 && departing) {
        return BOARDING;
    }
    if (!flying && groundspeed > 0 && departing) {
        return GROUND_DEP;
    }
    if (flying && arriving) {
        return ARRIVING; // put before departing; on small hops tend to show "arriving", not departing
    }
    if (flying && departing) {
        return DEPARTING;
    }
    if (flying && !departing && !arriving) {
        return EN_ROUTE;
    }
    if (!flying && groundspeed > 0 && arriving) {
        return GROUND_ARR;
    }
    if (!flying && groundspeed == 0 && arriving) {
        return BLOCKED;
    }
    return CRASHED; // should never happen
}

QString Pilot::flightStatusString() const {
    QString result;
    switch (flightStatus()) {
        case BOARDING:
            return "TTG " + eet().toString("H:mm") + " hrs"
                + ", ETA " + eta().toString("HHmm") + "z"
                + (delayString().isEmpty()? "": ", Delay " + delayString());
        case GROUND_DEP:
            return "TTG " + eet().toString("H:mm") + " hrs"
                + ", ETA " + eta().toString("HHmm") + "z"
                + (delayString().isEmpty()? "": ", Delay " + delayString());
        case DEPARTING:
            return "TTG " + eet().toString("H:mm") + " hrs"
                + ", ETA " + eta().toString("HHmm") + "z"
                + (delayString().isEmpty()? "": ", Delay " + delayString());
        case ARRIVING:
            return "TTG " + eet().toString("H:mm") + " hrs"
                + ", ETA " + eta().toString("HHmm") + "z"
                + (delayString().isEmpty()? "": ", Delay " + delayString());
        case GROUND_ARR:
            return "ATA " + eta().toString("HHmm") + "z" // Actual Time of Arrival :)
                + (delayString().isEmpty()? "": ", Delay " + delayString());
        case BLOCKED:
            return "ATA " + eta().toString("HHmm") + "z" // Actual Time of Arrival :)
                + (delayString().isEmpty()? "": ", Delay " + delayString());
        case CRASHED: return "";
        case BUSH: return "";
        case EN_ROUTE: {
            result = "";
            Airport* dep = depAirport();
            Airport* dst = destAirport();
            if (dst == 0) {
                return result;
            }
            if (dep != 0) {
                // calculate %done
                int total_dist = (int) NavData::distance(dep->lat, dep->lon, dst->lat, dst->lon);
                if (total_dist > 0) {
                    int dist_done = (int) NavData::distance(lat, lon, dep->lat, dep->lon);
                    result += QString("%1%").arg(dist_done * 100 / total_dist);
                }
            }
            result += ", TTG " + eet().toString("H:mm") + " hrs";
            result += ", ETA " + eta().toString("HHmm") + "z";
            result += (delayString().isEmpty()? "": ", Delay " + delayString());
            return result;
        }
        case PREFILED:
            return "";
    }
    return "Unknown";
}

QString Pilot::flightStatusShortString() const {
    switch (flightStatus()) {
        case BOARDING: return "boarding";
        case GROUND_DEP: return QString("taxiing out");
        case DEPARTING: return QString("departing");
        case ARRIVING: return QString("arriving");
        case GROUND_ARR: return QString("taxiing in");
        case BLOCKED: return QString("blocked");
        case CRASHED: return QString("crashed");
        case BUSH: return QString("bush pilot");
        case EN_ROUTE: return QString("en route");
        case PREFILED: return QString("prefiled");
        default: return QString("unknown");
    }
}

QString Pilot::planFlighttypeString() const {
    if (planFlighttype == "I") {
        return QString("IFR");
    } else if (planFlighttype == "V") {
        return QString("VFR");
    } else if (planFlighttype == "Y") {
        return QString("Y: IFR to VFR");
    } else if (planFlighttype == "Z") {
        return QString("Z: VFR to IFR");
    } else if (planFlighttype == "S") {
        return QString("SVFR");
    } else {
        return QString(planFlighttype);
    }
}

QString Pilot::toolTip() const {
    QString result = callsign + " (" + realName();
    if (!rank().isEmpty()) {
        result += ", " + rank();
    }
    return result += ")"
            + (!planDep.isEmpty() || !planDest.isEmpty()? " " + planDep + "-" + planDest: "")
            + (altitude > 0 && groundspeed > 0? ", " + humanAlt(): "")
    ;
}

QString Pilot::rank() const {
    QStringList ret;

    if (pilotRating > 0) { // we hide "NEW"
        auto pilotRatingString = Whazzup::instance()->realWhazzupData().pilotRatings.value(pilotRating, QString("#%1").arg(pilotRating));
        ret << pilotRatingString;
    }

    if (militaryRating > 0) { // we hide "M0"
        auto militaryRatingString = Whazzup::instance()->realWhazzupData().militaryRatings.value(militaryRating, QString("#%1").arg(militaryRating));
        ret << militaryRatingString;
    }

    return ret.join(" ");
}

QString Pilot::aircraftType() const {
    QStringList acftSegments = planAircraft.split("/");

    // VATSIM can be a real PITA, really
    // FAA-style without WTC prefix (e.g. "B737/G")
    if (acftSegments.size() == 2 && acftSegments[0].length() >= 2) {
        return acftSegments[0];
    }
    // ICAO-style (e.g. "A320/M-SDE2E3FGHIJ1RWXY/LB2")
    if (acftSegments.size() == 3 && acftSegments[0].length() >= 2) {
        return acftSegments[0];
    }
    // FAA-style with ("H/B763/L") or without equipment suffix ("H/B763")
    else if (acftSegments.size() >= 2) {
        return acftSegments[1];
    }

    return planAircraft;
}

Airport* Pilot::depAirport() const {
    return NavData::instance()->airports.value(planDep, 0);
}

Airport* Pilot::destAirport() const {
    return NavData::instance()->airports.value(planDest, 0);
}

Airport* Pilot::altAirport() const {
    return NavData::instance()->airports.value(planAltAirport, 0);
}

double Pilot::distanceFromDeparture() const {
    Airport* dep = depAirport();
    if (dep == 0) {
        return 0;
    }

    return NavData::distance(lat, lon, dep->lat, dep->lon);
}

double Pilot::distanceToDestination() const {
    Airport* dest = destAirport();
    if (dest == 0) {
        return 0;
    }

    if (flightStatus() == Pilot::PREFILED) {
        Airport* dep = depAirport();
        if (dep == 0) {
            return 0;
        }
        return NavData::distance(dep->lat, dep->lon, dest->lat, dest->lon);
    }

    return NavData::distance(lat, lon, dest->lat, dest->lon);
}

int Pilot::planTasInt() const { // defuck flightplanned TAS
    if (planTAS.startsWith("M")) { // approximate Mach -> TAS conversion
        if (planTAS.contains(".")) {
            return (int) planTAS.mid(1).toDouble() * 550;
        } else {
            return static_cast<int>(planTAS.mid(1).toInt() * 5.5);
        }
    }
    return planTAS.toInt();
}

QDateTime Pilot::etd() const { // Estimated Time of Departure - but according to the web form SOBT
    QString planDeptimeFixed = planDeptime;

    if (planDeptime.length() == 3) {
        planDeptimeFixed.prepend("0"); // fromString("Hmm") does not handle "145" correctly -> "14:05"

    }
    QTime time = QTime::fromString(planDeptimeFixed, "HHmm");

    return QDateTime(dayOfFlight, time, Qt::UTC);
}

QDateTime Pilot::eta() const { // Estimated Time of Arrival
    FlightStatus status = flightStatus();
    if (status == PREFILED || status == BOARDING) {
        if (whazzupTime < etaPlan()) {
            return etaPlan();
        }
        return whazzupTime.addSecs(etd().secsTo(etaPlan()));
    } else if (status == DEPARTING || status == GROUND_DEP) { // try to calculate with flightplanned speed
        int enrouteSecs;
        if (planTasInt() == 0) {
            if (groundspeed == 0) {
                return QDateTime(); // abort
            }
            enrouteSecs = (int) (distanceToDestination() * 3600) / groundspeed;
        } else {
            enrouteSecs = (int) (distanceToDestination() * 3600) / planTasInt();
        }
        if (status == GROUND_DEP) {
            enrouteSecs += taxiTimeOutbound; // taxi time outbound
        }
        return whazzupTime.addSecs(enrouteSecs);
    } else if (status == EN_ROUTE || status == ARRIVING) { // try groundspeed
        int enrouteSecs;
        if (groundspeed == 0) {
            if (planTasInt() == 0) {
                return QDateTime(); // abort
            }
            enrouteSecs = (int) (distanceToDestination() * 3600) / planTasInt();
        } else {
            enrouteSecs = (int) (distanceToDestination() * 3600) / groundspeed;
        }
        return whazzupTime.addSecs(enrouteSecs);
    } else if (status == GROUND_ARR || status == BLOCKED) {
        return whazzupTime;
    }
    return QDateTime();
}

QTime Pilot::eet() const { // Estimated Enroute Time remaining
    int secs = whazzupTime.secsTo(eta());
    return QTime((secs / 3600) % 24, (secs / 60) % 60);
}

QDateTime Pilot::etaPlan() const { // Estimated Time of Arrival as flightplanned
    if (planEnroute_hrs < 0) {
        return QDateTime();
    }
    return etd().addSecs(planEnroute_hrs * 3600 + planEnroute_mins * 60);
}

QString Pilot::delayString() const { // delay
    auto status = flightStatus();
    if (status == BOARDING || status == GROUND_DEP || status == PREFILED) {
        // output delta to planned off-block
        auto secs = etd().secsTo(whazzupTime);
        auto calcSecs = (secs < 0? -secs: secs);
        return QString("%1%2 hrs")
            .arg(secs < 0? "-": "", QTime((calcSecs / 3600) % 24, (calcSecs / 60) % 60).toString("H:mm"));
    }

    if (!etaPlan().isValid()) {
        return "";
    }
    auto secs = etaPlan().secsTo(eta());
    if (secs == 0) {
        return QString("n/a");
    }
    auto calcSecs = (secs < 0? -secs: secs);
    return QString("%1%2 hrs")
        .arg(secs < 0? "-": "", QTime((calcSecs / 3600) % 24, (calcSecs / 60) % 60).toString("H:mm"));
}

int Pilot::defuckPlanAlt(QString altStr) const { // returns an altitude from various flightplan strings
    altStr = altStr.trimmed();
    if (altStr.length() < 4 && altStr.toInt() != 0) { // 280: naive mode
        return altStr.toInt() * 100;
    }
    if (altStr.leftRef(2) == "FL") { // FL280: bastard mode
        return altStr.midRef(2).toInt() * 100;
    }
    if (altStr.leftRef(1) == "F") { // F280
        return altStr.midRef(1).toInt() * 100;
    }
    if (altStr.leftRef(1) == "A" && altStr.length() <= 4) { // A45
        return altStr.midRef(1).toInt() * 100;
    }
    if (altStr.leftRef(1) == "A" && altStr.length() > 4) { // A4500: idiot mode...
        return altStr.midRef(1).toInt();
    }
    if (altStr.leftRef(1) == "S") { // S1130 (FL 1130m)
        return mToFt(altStr.midRef(1).toInt());
    }
    if (altStr.leftRef(1) == "M" && altStr.length() <= 4) { // M0840 (840m)
        return mToFt(altStr.midRef(1).toInt());
    }
    return altStr.toInt();
}

QString Pilot::humanAlt() const {
    if (altitude < 10000) {
        return QString("%1 ft").arg(round(altitude / 100.) * 100);
    }
    return QString("FL %1").arg(Pilot::altToFl(altitude, qnh_mb));
}

QString Pilot::flOrEmpty() const {
    if (groundspeed == 0) {
        return "";
    }
    return QString("F%1").arg(Pilot::altToFl(altitude, qnh_mb));
}

QStringList Pilot::waypoints() const {
    // @todo put this into a RouteResolver
    auto parts = planRoute.toUpper().split(
        QRegExp(
            "[\\s\\-+.,/]|"
            "\\b(?:[MNAFSMK]\\d{3,4}){2,}\\b|"
            "\\b\\d{2}\\D?\\b|"
            "DCT"
        ),
        Qt::SkipEmptyParts
    ); // split and throw away DCT + /N450F230 etc.

    if (!parts.isEmpty()) {
        if (parts.constFirst() == planDep) {
            parts.removeFirst();
        }
    }

    if (!parts.isEmpty()) {
        if (parts.constLast() == planDest) {
            parts.removeLast();
        }
    }

    return parts;
}

QPair<double, double> Pilot::positionInFuture(int seconds) const {
    double dist = (double) groundspeed * ((double) seconds / 3600.0);
    return NavData::pointDistanceBearing(lat, lon, dist, trueHeading);
}

int Pilot::nextPointOnRoute(const QList<Waypoint*> &waypoints) const { // next point after present position
    if ((qFuzzyIsNull(lat) && qFuzzyIsNull(lon)) || waypoints.isEmpty()) {
        return 0; // prefiled flight or no known position
    }
    int nextPoint;
    // find the point that is nearest to the plane
    double minDist = NavData::distance(
        lat, lon, waypoints[0]->lat,
        waypoints[0]->lon
    );
    int minPoint = 0; // next to departure as default
    for (int i = 1; i < waypoints.size(); i++) {
        if (NavData::distance(lat, lon, waypoints[i]->lat, waypoints[i]->lon) < minDist) {
            minDist = NavData::distance(lat, lon, waypoints[i]->lat, waypoints[i]->lon);
            minPoint = i;
        }
    }
    // with the nearest point, look which one is the next point ahead - saves from trouble with zig-zag routes
    if (minPoint == 0) {
        nextPoint = 1;
    } else if (minPoint == waypoints.size() - 1) {
        nextPoint = waypoints.size() - 1;
    } else {
        nextPoint = minPoint + 1; // default
        // look for the first route segment where the planned course deviates > 90° from the bearing to the
        // plane
        int courseRoute, courseToPlane, courseDeviation;
        for (int i = minPoint - 1; i <= minPoint; i++) {
            courseRoute = (int) NavData::courseTo(
                waypoints[i]->lat, waypoints[i]->lon,
                waypoints[i + 1]->lat, waypoints[i + 1]->lon
            );
            courseToPlane = (int) NavData::courseTo(
                waypoints[i]->lat, waypoints[i]->lon,
                lat, lon
            );
            courseDeviation = (qAbs(courseRoute - courseToPlane)) % 360;
            if (courseDeviation > 90) {
                nextPoint = i;
                break;
            }
        }
    }
    return qMin(nextPoint, waypoints.size());
}

QString Pilot::livestreamString() const {
    return Client::livestreamString(planRemarks);
}

bool Pilot::hasPrimaryAction() const {
    return true;
}

void Pilot::primaryAction() {
    showDetailsDialog();
}

QList<Waypoint*> Pilot::routeWaypoints() {
    if (
        (planDep == routeWaypointsPlanDepCache) // we might have cached the route already
        && (planDest == routeWaypointsPlanDestCache)
        && (planRoute == routeWaypointsPlanRouteCache)
    ) {
        return routeWaypointsCache; // no changes
    }

    routeWaypointsPlanDepCache = planDep;
    routeWaypointsPlanDestCache = planDest;
    routeWaypointsPlanRouteCache = planRoute;

    const double maxDist = planFlighttype == "I"
                               ? Airac::ifrMaxWaypointInterval
                               : Airac::nonIfrMaxWaypointInterval;

    if (depAirport() != 0) {
        routeWaypointsCache = Airac::instance()->resolveFlightplan(
            waypoints(), depAirport()->lat, depAirport()->lon, maxDist
        );
    } else if (!qFuzzyIsNull(lat) || !qFuzzyIsNull(lon)) {
        routeWaypointsCache = Airac::instance()->resolveFlightplan(
            waypoints(), lat, lon, maxDist
        );
    } else {
        routeWaypointsCache = QList<Waypoint*>();
    }

    return routeWaypointsCache;
}

QString Pilot::routeWaypointsString() {
    QStringList ret;
    foreach (const Waypoint* w, routeWaypoints()) {
        ret.append(w->id);
    }
    return ret.join(" ");
}

QList<Waypoint*> Pilot::routeWaypointsWithDepDest() {
    QList<Waypoint*> waypoints = routeWaypoints();
    if (depAirport() != 0) {
        waypoints.prepend(new Waypoint(depAirport()->id, depAirport()->lat, depAirport()->lon));
    }
    if (destAirport() != 0) {
        waypoints.append(new Waypoint(destAirport()->id, destAirport()->lat, destAirport()->lon));
    }
    return waypoints;
}

void Pilot::checkStatus() {
    drawLabel = flightStatus() == Pilot::DEPARTING
        || flightStatus() == Pilot::EN_ROUTE
        || flightStatus() == Pilot::ARRIVING
        || flightStatus() == Pilot::CRASHED
        || flightStatus() == Pilot::BUSH
        || flightStatus() == Pilot::PREFILED;
}
