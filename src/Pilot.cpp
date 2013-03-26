/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Pilot.h"

#include "PilotDetails.h"
#include "NavData.h"
#include "helpers.h"
#include "Settings.h"

Pilot::Pilot(const QStringList& stringList, const WhazzupData* whazzup):
        Client(stringList, whazzup),
        onGround(false),
        showDepDestLine(false) {
    whazzupTime = QDateTime(whazzup->whazzupTime); // need some local reference to that

    altitude = field(stringList, 7).toInt(); // we could do some barometric
                                    // calculations here (only for VATSIM needed)
    groundspeed = field(stringList, 8).toInt();
    planAircraft = field(stringList, 9);
    planTAS = field(stringList, 10);
    planDep = field(stringList, 11);
    planAlt = field(stringList, 12);
    planDest = field(stringList, 13);

    QString airlineCode = field(stringList,0);
    airlineCode.resize(3);
    airline = NavData::instance()->airline(airlineCode);

    transponder = field(stringList, 17);
    planRevision = field(stringList, 20);
    planFlighttype = field(stringList, 21);
    planDeptime = field(stringList, 22);
    planActtime = field(stringList, 23);

    QString tmpStr = field(stringList, 24);
    if(tmpStr.isNull())
        planEnroute_hrs = -1;
    else
        planEnroute_hrs = tmpStr.toInt();
    planEnroute_mins = field(stringList, 25).toInt();
    planFuel_hrs = field(stringList, 26).toInt();
    planFuel_mins = field(stringList, 27).toInt();
    planAltAirport = field(stringList, 28);
    planRemarks = field(stringList, 29);
    planRoute = field(stringList, 30);

    if(whazzup->isIvao()) {
        planAltAirport2 = field(stringList, 42); // IVAO only
        planTypeOfFlight = field(stringList, 43); // IVAO only
        pob = field(stringList, 44).toInt(); // IVAO only

        trueHeading = field(stringList, 45).toInt();
        onGround = field(stringList, 46).toInt() == 1; // IVAO only
    }

    if(whazzup->isVatsim()) {
        trueHeading = field(stringList, 38).toInt();
        qnh_inHg = field(stringList, 39); // VATSIM only
        qnh_mb = field(stringList, 40); // VATSIM only
    }
    // day of flight
    if(!QTime::fromString(planDeptime, "HHmm").isValid()) // no Plan ETA given: maybe some more magic needed here
        dayOfFlight = whazzupTime.date();
    else if((QTime::fromString(planDeptime, "HHmm").hour() - whazzupTime.time().hour()) % 24 <= 2) // allow for early birds (up to 2 hours before planned departure)
        dayOfFlight = whazzupTime.date();
    else
        dayOfFlight = whazzupTime.date().addDays(-1); // started the day before

    checkStatus();

    // anti-idiot hack: some guys like routes like KORLI/MARPI/UA551/FOF/FUN...
    // let's keep him... waypoints() takes care of it in places where we need it.
    //if(planRoute.count("/") > 4)
    //    planRoute.replace(QRegExp("[/]"), " ");
}

void Pilot::showDetailsDialog() {
    //qDebug() << "Pilot::showDetailsDialog()";
    PilotDetails *infoDialog = PilotDetails::instance();
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
    const bool flying = groundspeed > 50 || altitude > 9000
                        || (network == IVAO && !onGround);

    if (dep == 0 || dst == 0) {
        if(flying)
            return EN_ROUTE;
        else
            return BUSH;
    }

    const double totalDist = NavData::distance(dep->lat, dep->lon, dst->lat, dst->lon);
    const double distDone = NavData::distance(lat, lon, dep->lat, dep->lon);
    const double distRemaining = totalDist - distDone;

    // arriving?
    const bool arriving = distRemaining < 50;

    // departing?
    const bool departing = distDone < 50;


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
    if(!flying && qFuzzyIsNull(lat) && qFuzzyIsNull(lon)) // must be before BLOCKED
        return PREFILED;
    if(!flying && groundspeed == 0 && arriving)
        return BLOCKED;
    return CRASHED; // should never happen
}

QString Pilot::flightStatusString() const {
    QString result;
    switch(flightStatus()) {
        case BOARDING:
            return "TTG " + eet().toString("H:mm") + " hrs"
                    + ", ETA " + eta().toString("HHmm") + "z"
                    + (delayStr().isEmpty()? "": ", Delay " + delayStr() + " hrs");
        case GROUND_DEP:
            return "TTG " + eet().toString("H:mm") + " hrs"
                    + ", ETA " + eta().toString("HHmm") + "z"
                    + (delayStr().isEmpty()? "": ", Delay " + delayStr() + " hrs");
        case DEPARTING:
            return "TTG " + eet().toString("H:mm") + " hrs"
                    + ", ETA " + eta().toString("HHmm") + "z"
                    + (delayStr().isEmpty()? "": ", Delay " + delayStr() + " hrs");
        case ARRIVING:
            return "TTG " + eet().toString("H:mm") + " hrs"
                    + ", ETA " + eta().toString("HHmm") + "z"
                    + (delayStr().isEmpty()? "": ", Delay " + delayStr() + " hrs");
        case GROUND_ARR:
            return "ATA " + eta().toString("HHmm") + "z" // Actual Time of Arrival :)
                    + (delayStr().isEmpty()? "": ", Delay " + delayStr() + " hrs");
        case BLOCKED:
            return "ATA " + eta().toString("HHmm") + "z" // Actual Time of Arrival :)
                    + (delayStr().isEmpty()? "": ", Delay " + delayStr() + " hrs");
        case CRASHED: return QString();
        case BUSH: return QString();
        case EN_ROUTE: {
            result = QString();
            Airport *dep = depAirport();
            Airport *dst = destAirport();
            if(dst == 0)
                return result;
            if(dep != 0) {
                // calculate %done
                int total_dist = (int)NavData::distance(dep->lat, dep->lon, dst->lat, dst->lon);
                if(total_dist > 0) {
                    int dist_done = (int)NavData::distance(lat, lon, dep->lat, dep->lon);
                    result += QString("%1%").arg(dist_done * 100 / total_dist);
                }
            }
            result += ", TTG " + eet().toString("H:mm") + " hrs";
            result += ", ETA " + eta().toString("HHmm") + "z";
            result += (delayStr().isEmpty()? "": ", Delay " + delayStr() + " hrs");
            return result;
        }
        case PREFILED:
            return QString();
    }
    return QString("Unknown");
}

QString Pilot::flightStatusShortString() const {
    switch(flightStatus()) {
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
    if (planFlighttype == "I")
        return QString("IFR");
    else if (planFlighttype == "V")
        return QString("VFR");
    else if (planFlighttype == "Y")
        return QString("Y: IFR to VFR");
    else if (planFlighttype == "Z")
        return QString("Z: VFR to IFR");
    else if (planFlighttype == "S")
        return QString("SVFR");
    else
        return QString(planFlighttype);
}

QString Pilot::toolTip() const {
    return Client::toolTip() +
            (!planDep.isEmpty() || !planDest.isEmpty()? " " + planDep + "-" + planDest: "");
}

QString Pilot::rank() const {
    if(network == IVAO) {
        switch(rating) {
            case 2: return "FS1"; //Basic Flight Student
            case 3: return "FS2"; //Flight Student
            case 4: return "FS3"; //Advanced Flight Student
            case 5: return "PP"; //Private Pilot
            case 6: return "SPP"; //Senior Private Pilot
            case 7: return "CP"; //Commercial Pilot
            case 8: return "ATP"; //Airline Transport Pilot (currently not available)
            case 9: return "SFI"; //Senior Flight Instructor
            case 10: return "CFI"; //Chief Flight Instructor
            default: return QString("? (%1)").arg(rating);
        }
    } /*else if(network == VATSIM) {
        switch(rating) { // experimental, I do not know which ratings really get reported yet
            case 1: return "P0"; // Unrated Pilot
            case 2: return "P1"; // Pilot
            case 3: return "P2"; // VFR Pilot
            case 4: return "P3"; // IFR Pilot
            case 5: return "P4"; // Command Pilot
            case 6: return "P5"; // Master Pilot
            default: return QString("? (%1)").arg(rating);
        }
    }*/
    return QString();
}

QString Pilot::aircraftType() const {
    QStringList acftSegments = planAircraft.split("/");

    if(network == IVAO && acftSegments.size() >= 2)
        return acftSegments[1];

    if(network == VATSIM) {
        // VATSIM can be a real PITA.
        if(acftSegments.size() == 2 && acftSegments[0].length() > 2)
            return acftSegments[0];
        else if(acftSegments.size() >= 2)
            return acftSegments[1];
    }

    return planAircraft;
}

Airport *Pilot::depAirport() const {
    if(NavData::instance()->airports.contains(planDep))
        return NavData::instance()->airports[planDep];
    else return 0;
}

Airport *Pilot::destAirport() const {
    if(NavData::instance()->airports.contains(planDest))
        return NavData::instance()->airports[planDest];
    else return 0;
}

Airport *Pilot::altAirport() const {
    if(NavData::instance()->airports.contains(planAltAirport))
        return NavData::instance()->airports[planAltAirport];
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
            return static_cast<int>(planTAS.mid(1).toInt() * 5.5);
    }
    return planTAS.toInt();
}

QDateTime Pilot::etd() const { // Estimated Time of Departure
    QString planDeptimeFixed = planDeptime;

    if(planDeptime.length() == 3)
        planDeptimeFixed.prepend("0"); // fromString("Hmm") does not handle "145" correctly -> "14:05"

    QTime time = QTime::fromString(planDeptimeFixed, "HHmm");

    return QDateTime(dayOfFlight, time, Qt::UTC);
}

QDateTime Pilot::eta() const { // Estimated Time of Arrival
    FlightStatus status = flightStatus();
    if(status == PREFILED || status == BOARDING)
        return etaPlan();
    else if(status == DEPARTING || status == GROUND_DEP) { // try to calculate with flightplanned speed
        int enrouteSecs;
        if(planTasInt() == 0) {
            if(groundspeed == 0)
                return QDateTime(); // abort
            enrouteSecs = (int) (distanceToDestination() * 3600) / groundspeed;
        } else
            enrouteSecs = (int) (distanceToDestination() * 3600) / planTasInt();
        if(status == GROUND_DEP) enrouteSecs += 240; // taxi time outbound
        return whazzupTime.addSecs(enrouteSecs);
    } else if(status == EN_ROUTE || status == ARRIVING) { // try groundspeed
        int enrouteSecs;
        if(groundspeed == 0) {
            if(planTasInt() == 0)
                return QDateTime(); // abort
            enrouteSecs = (int) (distanceToDestination() * 3600) / planTasInt();
        } else
            enrouteSecs = (int) (distanceToDestination() * 3600) / groundspeed;
        return whazzupTime.addSecs(enrouteSecs);
    } else if(status == GROUND_ARR || status == BLOCKED)
        return whazzupTime;
    return QDateTime();
}

QTime Pilot::eet() const { // Estimated Enroute Time remaining
    int secs = whazzupTime.secsTo(eta());
    QTime ret = QTime((secs / 3600) % 24, (secs / 60) % 60);
    return ret;
}

QDateTime Pilot::etaPlan() const { // Estimated Time of Arrival as flightplanned
    if(planEnroute_hrs < 0)
        return QDateTime();
    return etd().addSecs(planEnroute_hrs * 3600 + planEnroute_mins * 60);
}

QString Pilot::delayStr() const { // delay
    if(!etaPlan().isValid())
        return QString();
    int secs, calcSecs;
    secs = etaPlan().secsTo(eta());
    if (secs == 0)
        return QString("n/a");
    calcSecs = (secs < 0? -secs: secs);
    return QString("%1%2")
            .arg(secs < 0? "-": "")
            .arg(QTime((calcSecs / 3600) % 24, (calcSecs / 60) % 60).toString("H:mm"));
}

int Pilot::defuckPlanAlt(QString altStr) const { // returns an altitude from various flightplan strings
    altStr = altStr.trimmed();
    if(altStr.length() < 4 && altStr.toInt() != 0) // 280: naive mode
        return altStr.toInt() * 100;
    if(altStr.left(2) == "FL") // FL280: bastard mode
        return altStr.mid(2).toInt() * 100;
    if(altStr.left(1) == "F") // F280
        return altStr.mid(1).toInt() * 100;
    if(altStr.left(1) == "A" && altStr.length() <= 4) // A45
        return altStr.mid(1).toInt() * 100;
    if(altStr.left(1) == "A" && altStr.length() > 4) // A4500: idiot mode...
        return altStr.mid(1).toInt();
    return altStr.toInt();
}

QStringList Pilot::waypoints() const {
    return planRoute.toUpper().split(
                QRegExp("[\\s\\-+.,/]|"
                        "\\b(?:[MNAFSMK]\\d{3,4}){2,}\\b|"
                        "\\b\\d{2}\\D?\\b|"
                        "DCT"),
                QString::SkipEmptyParts); // split and throw away DCT + /N450F230 etc.
}

QPair<double, double> Pilot::positionInFuture(int seconds) const {
    double dist = (double)groundspeed * ((double)seconds / 3600.0);
    return NavData::pointDistanceBearing(lat, lon, dist, trueHeading);
}

int Pilot::nextPointOnRoute(const QList<Waypoint *> &waypoints) const { // next point after present position
    if((qFuzzyIsNull(lat) && qFuzzyIsNull(lon)) || waypoints.isEmpty())
        return 0; // prefiled flight or no known position
    int nextPoint;
    // find the point that is nearest to the plane
    double minDist = NavData::distance(lat, lon, waypoints[0]->lat,
                                       waypoints[0]->lon);
    int minPoint = 0; // next to departure as default
    for(int i = 1; i < waypoints.size(); i++) {
        if(NavData::distance(lat, lon, waypoints[i]->lat, waypoints[i]->lon) <
           minDist) {
            minDist = NavData::distance(lat, lon, waypoints[i]->lat, waypoints[i]->lon);
            minPoint = i;
        }
    }
    // with the nearest point, look which one is the next point ahead - saves from trouble with zig-zag routes
    if(minPoint == 0) {
        nextPoint = 1;
    } else if(minPoint == waypoints.size() - 1) {
        nextPoint = waypoints.size() - 1;
    } else {
        nextPoint = minPoint + 1; // default
        // look for the first route segment where the planned course deviates > 90° from the bearing to the plane
        int courseRoute, courseToPlane, courseDeviation;
        for(int i = minPoint - 1; i <= minPoint; i++) {
            courseRoute = (int) NavData::courseTo(waypoints[i]->lat, waypoints[i]->lon,
                                                  waypoints[i + 1]->lat, waypoints[i + 1]->lon);
            courseToPlane = (int) NavData::courseTo(waypoints[i]->lat, waypoints[i]->lon,
                                                    lat, lon);
            courseDeviation = (qAbs(courseRoute - courseToPlane)) % 360;
            if (courseDeviation > 90) {
                nextPoint = i;
                break;
            }
        }
    }
    return qMin(nextPoint, waypoints.size());
}

bool Pilot::showDepLine() const {
    if (depAirport() != 0)
        return (Settings::depLineStrength() > 0. &&
                (showDepDestLine || depAirport()->showFlightLines));
    else
        return (Settings::depLineStrength() > 0. && showDepDestLine);
}

bool Pilot::showDestLine() const {
    if (destAirport() != 0)
        return (Settings::destLineStrength() > 0. &&
                (showDepDestLine || destAirport()->showFlightLines));
    else
        return (Settings::destLineStrength() > 0. && showDepDestLine);
}

QList<Waypoint*> Pilot::routeWaypoints() {
    //qDebug() << "Pilot::routeWaypoints()" << label;
    if ((planDep == routeWaypointsPlanDepCache) // we might have cached the route already
        && (planDest == routeWaypointsPlanDestCache)
        && (planRoute == routeWaypointsPlanRouteCache)) {
        return routeWaypointsCache; // no changes
    }

    routeWaypointsPlanDepCache = planDep;
    routeWaypointsPlanDestCache = planDest;
    routeWaypointsPlanRouteCache = planRoute;

    if (depAirport() != 0)
        routeWaypointsCache = Airac::instance()->resolveFlightplan(
                    waypoints(), depAirport()->lat, depAirport()->lon);
    else if (!qFuzzyIsNull(lat) || !qFuzzyIsNull(lon))
        routeWaypointsCache = Airac::instance()->resolveFlightplan(
                    waypoints(), lat, lon);
    else
        routeWaypointsCache = QList<Waypoint*>();

    //qDebug() << "Pilot::routeWaypoints() -- finished" << label;
    return routeWaypointsCache;
}

QString Pilot::routeWaypointsStr() {
    QStringList ret;
    foreach(const Waypoint *w, routeWaypoints())
        ret.append(w->label);
    return ret.join(" ");
}

QList<Waypoint*> Pilot::routeWaypointsWithDepDest() {
    QList<Waypoint*> waypoints = routeWaypoints();
    if(depAirport() != 0)
        waypoints.prepend(new Waypoint(depAirport()->label, depAirport()->lat,
                                       depAirport()->lon));
    if(destAirport() != 0)
        waypoints.append(new Waypoint(destAirport()->label, destAirport()->lat,
                                      destAirport()->lon));
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
