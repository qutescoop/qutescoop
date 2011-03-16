/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "NavData.h"

#include "FileReader.h"
#include "SectorReader.h"
#include "helpers.h"
#include "Settings.h"

NavData *navDataInstance = 0;
NavData *NavData::getInstance(bool createIfNoInstance) {
    if(navDataInstance == 0)
        if (createIfNoInstance) {
            navDataInstance = new NavData();
        }
    return navDataInstance;
}

NavData::NavData() {
    loadAirports(Settings::applicationDataDirectory("data/airports.dat"));
    loadSectors();
    loadCountryCodes(Settings::applicationDataDirectory("data/countrycodes.dat"));
}

void NavData::loadAirports(const QString& filename) {
    airportHash.clear();
    activeAirportsCongestionMap.clear();
    FileReader fileReader(filename);
    while (!fileReader.atEnd()) {
        QString line = fileReader.nextLine();
        if (line.isNull())
            return;
        Airport *airport = new Airport(line.split(':'));
        if (airport != 0 && !airport->isNull())
            airportHash[airport->label] = airport;
    }
}

void NavData::loadCountryCodes(const QString& filename) {
    countryCodes.clear();
    FileReader fileReader(filename);
    while (!fileReader.atEnd()) {
        QString line = fileReader.nextLine();
        if (line.isNull())
            return;
        QStringList list = line.split(':');
        if(!list.size() == 2)
            continue;
        countryCodes[list.first()] = list.last();
    }
}

void NavData::loadSectors() {
    SectorReader().loadSectors(sectorHash);
}

double NavData::distance(double lat1, double lon1, double lat2, double lon2) {
    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;
    double result = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1-lon2));
    if(qIsNaN(result))
        return 0;
    return result * 60.0 / Pi180;
}

QPair<double, double> NavData::pointDistanceBearing(double lat, double lon, double dist, double heading) {
    lat *= Pi180;
    lon *= Pi180;
    heading = (360 - heading) * Pi180;
    dist = dist / 60.0 * Pi180;
    double rlat = asin(sin(lat) * cos(dist) + cos(lat) * sin(dist) * cos(heading));
    double dlon = atan2(sin(heading) * sin(dist) * cos(lat), cos(dist) - sin(lat) * sin(lat));
    double rlon = fmod(lon - dlon + M_PI, 2 * M_PI) - M_PI;

    return QPair<double, double> (rlat / Pi180, rlon / Pi180);
}

QList<Airport*> NavData::airportsAt(double lat, double lon, double maxDist) {
    QList<Airport*> result;
    QList<Airport*> airports = airportHash.values();
    for(int i = 0; i < airports.size(); i++) {
        if(distance(airports[i]->lat, airports[i]->lon, lat, lon) <= maxDist) {
            result.append(airports[i]);
        }
    }
    return result;
}

void NavData::updateData(const WhazzupData& whazzupData) {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    qDebug() << "NavData::updateData() on" << airportHash.size() << "airports";
    foreach (Airport *a, activeAirportsCongestionMap.values())
        a->resetWhazzupStatus();

    QSet<Airport*> newActiveAirportsSet;
    QList<Pilot*> allpilots = whazzupData.getAllPilots();
    for(int i = 0; i < allpilots.size(); i++) {
        Pilot *p = allpilots[i];
        if(p == 0) continue;
        Airport *dep = airportHash.value(p->planDep, 0);
        if(dep != 0) {
            dep->addDeparture(p);
            newActiveAirportsSet.insert(dep);
            if(Settings::filterTraffic()) { // Airport traffic filtered
                if(p->distanceFromDeparture() < Settings::filterDistance())
                    dep->numFilteredDepartures++;
            }
        }
        else if(p->flightStatus() == Pilot::BUSH) { // no flightplan yet?
            QList<Airport*> aA = airportsAt(p->lat, p->lon, 3);
            if(!aA.isEmpty()) {
                aA.first()->addDeparture(p);
                aA.first()->numFilteredDepartures++;
                newActiveAirportsSet.insert(aA.first());
            }
        }
        Airport *dest = airportHash.value(p->planDest, 0);
        if(dest != 0) {
            dest->addArrival(p);
            newActiveAirportsSet.insert(dest);
            if(Settings::filterTraffic()) { // Airport traffic filtered
                if((p->distanceToDestination() < Settings::filterDistance())
                    || (p->distanceToDestination() / p->groundspeed < Settings::filterArriving()))
                    dest->numFilteredArrivals++;
            }
        }
    }

    for(int i = 0; i < whazzupData.getControllers().size(); i++) {
        Controller *c = dynamic_cast<Controller*>(whazzupData.getControllers()[i]);
        QString icao = c->getApproach();
        if(!icao.isNull() && airportHash.contains(icao)) {
            airportHash[icao]->addApproach(c);
            newActiveAirportsSet.insert(airportHash[icao]);
        }
        icao = c->getTower();
        if(!icao.isNull() && airportHash.contains(icao)) {
            airportHash[icao]->addTower(c);
            newActiveAirportsSet.insert(airportHash[icao]);
        }
        icao = c->getGround();
        if(!icao.isNull() && airportHash.contains(icao)) {
            airportHash[icao]->addGround(c);
            newActiveAirportsSet.insert(airportHash[icao]);
        }
        icao = c->getDelivery();
        if(!icao.isNull() && airportHash.contains(icao)) {
            airportHash[icao]->addDelivery(c);
            newActiveAirportsSet.insert(airportHash[icao]);
        }
    }

    // new method with MultiMap. Tests show: 450ms vs. 3800ms for 800 pilots :)
    activeAirportsCongestionMap.clear();
    foreach(Airport *a, newActiveAirportsSet) {
        int congestion = a->numFilteredArrivals + a->numFilteredDepartures; // sort key
        activeAirportsCongestionMap.insert(congestion, a);
    }

    qDebug() << "NavData::updateData() -- finished";
    qApp->restoreOverrideCursor();
}

void NavData::accept(MapObjectVisitor* visitor) {
    QList<Airport*> airports = airportHash.values();
    for(int i = 0; i < airports.size(); i++) {
        if(airports[i] != 0)
            visitor->visit(airports[i]);
    }
}

double NavData::courseTo(double lat1, double lon1, double lat2, double lon2) {
    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;

    double d = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1-lon2));

    double tc1;
    if(sin(lon2 - lon1) < 0) {
        tc1 = acos((sin(lat2) - sin(lat1) * cos(d)) / (sin(d) * cos(lat1)));
    } else {
        tc1 = 2 * M_PI - acos((sin(lat2) - sin(lat1) * cos(d)) / (sin(d) * cos(lat1)));
    }
    return 360 - (tc1 / Pi180);
}

QPair<double, double> NavData::greatCircleFraction(double lat1, double lon1, double lat2, double lon2,
                                  double f) {
    if (qFuzzyCompare(lat1, lat2) && qFuzzyCompare(lon1, lon2))
        return QPair<double, double>(lat1, lon1);

    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;

    double d = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1-lon2));

    double A = sin((1. - f) * d) / sin(d);
    double B = sin(f*d) / sin(d);
    double x = A * cos(lat1) * cos(lon1) + B * cos(lat2) * cos(lon2);
    double y = A * cos(lat1) * sin(lon1) + B * cos(lat2) * sin(lon2);
    double z = A * sin(lat1)             + B * sin(lat2);
    double rLat = atan2(z, sqrt(x*x + y*y));
    double rLon = atan2(y, x);

    return QPair<double, double>(rLat / Pi180, rLon / Pi180);
}

QList<QPair<double, double> > NavData::greatCirclePoints(double lat1, double lon1, double lat2, double lon2,
                                                         double pointEachNm) { // omits last point
    QList<QPair<double, double> > result;
    if (qFuzzyCompare(lat1, lat2) && qFuzzyCompare(lon1, lon2))
        return (result << QPair<double, double>(lat1, lon1));
    double fractionIncrement = qMin(1., pointEachNm / NavData::distance(lat1, lon1, lat2, lon2));
    for (double currentFraction = 0.; currentFraction < 1.; currentFraction += fractionIncrement)
        result.append(greatCircleFraction(lat1, lon1, lat2, lon2, currentFraction));
    return result;
}

void NavData::plotPointsOnEarth(const QList<QPair<double, double> > &points) { // plot greatcircles of lat/lon points on Earth
	if (points.isEmpty())
		return;
	if (points.size() > 1) {
		DoublePair wpOld = points[0];
		for (int i=1; i < points.size(); i++) {
			foreach(DoublePair p, greatCirclePoints(wpOld.first, wpOld.second,
													points[i].first, points[i].second))
				VERTEX(p.first, p.second);
			wpOld = points[i];
		}
	}
	VERTEX(points.last().first, points.last().second); // last points gets ommitted by greatCirclePoints by design
}

const QPair<double, double> *NavData::fromArinc(const QString &str) { // returning 0 on error
	QRegExp arinc("(\\d{2})([NSEW]?)(\\d{2})([NSEW]?)"); // ARINC424 waypoints (strict)
	if (arinc.exactMatch(str)) {
		if (!arinc.capturedTexts()[2].isEmpty() ||
			!arinc.capturedTexts()[4].isEmpty()) {
			double wLat = arinc.capturedTexts()[1].toDouble();
			double wLon = arinc.capturedTexts()[3].toDouble();
			if (QRegExp("[SW]").exactMatch(arinc.capturedTexts()[2]) ||
				QRegExp("[SW]").exactMatch(arinc.capturedTexts()[4]))
				wLat = -wLat;
			if (!arinc.capturedTexts()[2].isEmpty())
				wLon = wLon + 100.;
			if (QRegExp("[NW]").exactMatch(arinc.capturedTexts()[2]) ||
				QRegExp("[NW]").exactMatch(arinc.capturedTexts()[4]))
				wLon = -wLon;
			return new QPair<double, double>(wLat, wLon);
		}
	}
	return 0;
}

const QString NavData::toArinc(const short lat, const short lon) { // returning QString() on error
	if (qAbs(lat) > 90 || qAbs(lon) > 180)
		return QString();
	QString q; // ARINC 424 quadrant
	if (lat > 0) {
		if (lon > 0)
			q = "E";
		else
			q = "N";
	} else {
		if (lon > 0)
			q = "S";
		else
			q = "W";
	}
	return QString("%1%2%3%4").
			arg(qAbs(lat), 2, 10, QChar('0')).
			arg(qAbs(lon) >= 100? q: "").
			arg(qAbs(lon) >= 100? qAbs(lon) - 100: qAbs(lon), 2, 10, QChar('0')).
			arg(qAbs(lon) >= 100? "": q);
}
