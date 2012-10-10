/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "NavData.h"

#include <QRegExp>

#include "FileReader.h"
#include "SectorReader.h"
#include "helpers.h"
#include "Settings.h"


NavData *navDataInstance = 0;
NavData *NavData::getInstance(bool createIfNoInstance) {
    if(navDataInstance == 0)
        if (createIfNoInstance)
            navDataInstance = new NavData();
    return navDataInstance;
}

NavData::NavData() {
    loadAirports(Settings::applicationDataDirectory("data/airports.dat"));
    loadSectors();
    loadCountryCodes(Settings::applicationDataDirectory("data/countrycodes.dat"));
    if(Settings::useESAirlines()){loadAirlineCodes(Settings::ESAirlinesDirectory());}
    else{ loadAirlineCodes(Settings::applicationDataDirectory("data/airlines.dat"));}
}
NavData::~NavData() {
    foreach(const Airport *a, airports)
        delete a;
    foreach(const Sector *s, sectors)
        delete s;
}

void NavData::loadAirports(const QString& filename) {
    airports.clear();
    activeAirports.clear();
    FileReader fileReader(filename);
    while (!fileReader.atEnd()) {
        QString line = fileReader.nextLine();
        if (line.isNull())
            return;
        Airport *airport = new Airport(line.split(':'));
        if (airport != 0 && !airport->isNull())
            airports[airport->label] = airport;
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
    SectorReader().loadSectors(sectors);
}

void NavData::loadAirlineCodes(const QString &filename)
{
    airlineCodes.clear();
    if(filename.isEmpty()){
        qDebug() << "NavData::loadAirlineCodes -- no airline data loaded";
        return;
    }
    FileReader fileReader(filename);

    while(!fileReader.atEnd()){

        QStringList line = fileReader.nextLine().split(0x09);   // 0x09 code for Tabulator
        if(line.size()< 3){         // if prefent crashing when loading a wrong format
            GuiMessages::warning(QString(tr("%1 has not the correct format to load the airlines!")).arg(filename));
            return;
        }
        airlineCodes[line.value(0)] = line.value(2);
    }
}

double NavData::distance(double lat1, double lon1, double lat2, double lon2) {
    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;
    double result = qAcos(qSin(lat1) * qSin(lat2) + qCos(lat1) * qCos(lat2) * qCos(lon1-lon2));
    if(qIsNaN(result))
        return 0;
    return result * 60.0 / Pi180;
}

QPair<double, double> NavData::pointDistanceBearing(double lat, double lon, double dist, double heading) {
    lat *= Pi180;
    lon *= Pi180;
    heading = (360 - heading) * Pi180;
    dist = dist / 60.0 * Pi180;
    double rlat = qAsin(qSin(lat) * qCos(dist) + qCos(lat) * qSin(dist) * qCos(heading));
    double dlon = qAtan2(qSin(heading) * qSin(dist) * qCos(lat), qCos(dist) - qSin(lat) * qSin(lat));
    double rlon = fmod(lon - dlon + M_PI, 2 * M_PI) - M_PI;

    return QPair<double, double> (rlat / Pi180, rlon / Pi180);
}

Airport* NavData::airportAt(double lat, double lon, double maxDist) const {
    foreach(Airport *a, airports.values())
        if(distance(a->lat, a->lon, lat, lon) <= maxDist)
            return a;
    return 0;
}

void NavData::updateData(const WhazzupData& whazzupData) {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    qDebug() << "NavData::updateData() on" << airports.size() << "airports";
    foreach(Airport *a, activeAirports.values())
        a->resetWhazzupStatus();

    QSet<Airport*> newActiveAirportsSet;
    QList<Pilot*> allpilots = whazzupData.allPilots();
    for(int i = 0; i < allpilots.size(); i++) {
        Pilot *p = allpilots[i];
        if(p == 0) continue;
        Airport *dep = airports.value(p->planDep, 0);
        if(dep != 0) {
            dep->addDeparture(p);
            newActiveAirportsSet.insert(dep);
            if(Settings::filterTraffic()) { // Airport traffic filtered
                if(p->distanceFromDeparture() < Settings::filterDistance())
                    dep->numFilteredDepartures++;
            }
        }
        else if(p->flightStatus() == Pilot::BUSH) { // no flightplan yet?
            Airport *a = airportAt(p->lat, p->lon, 3.);
            if(a != 0) {
                a->addDeparture(p);
                a->numFilteredDepartures++;
                newActiveAirportsSet.insert(a);
            }
        }
        Airport *dest = airports.value(p->planDest, 0);
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

    foreach(Controller *c, whazzupData.controllers) {
        QString icao = c->getApproach();
        if(!icao.isNull() && airports.contains(icao)) {
            airports[icao]->addApproach(c);
            newActiveAirportsSet.insert(airports[icao]);
        }
        icao = c->getTower();
        if(!icao.isNull() && airports.contains(icao)) {
            airports[icao]->addTower(c);
            newActiveAirportsSet.insert(airports[icao]);
        }
        icao = c->getGround();
        if(!icao.isNull() && airports.contains(icao)) {
            airports[icao]->addGround(c);
            newActiveAirportsSet.insert(airports[icao]);
        }
        icao = c->getDelivery();
        if(!icao.isNull() && airports.contains(icao)) {
            airports[icao]->addDelivery(c);
            newActiveAirportsSet.insert(airports[icao]);
        }
    }

    // new method with MultiMap. Tests show: 450ms vs. 3800ms for 800 pilots :)
    activeAirports.clear();
    foreach(Airport *a, newActiveAirportsSet) {
        int congestion = a->numFilteredArrivals + a->numFilteredDepartures; // sort key
        activeAirports.insert(congestion, a);
    }

    qDebug() << "NavData::updateData() -- finished";
    qApp->restoreOverrideCursor();
}

void NavData::accept(SearchVisitor* visitor) {
    foreach(Airport *a, airports){
        visitor->visit(a);
    }
    visitor->AirlineCodes = airlineCodes;
    visitor->checkAirlines();
}

double NavData::courseTo(double lat1, double lon1, double lat2, double lon2) {
    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;

    double d = qAcos(qSin(lat1) * qSin(lat2) + qCos(lat1) * qCos(lat2) * qCos(lon1-lon2));

    double tc1;
    if(qSin(lon2 - lon1) < 0.)
        tc1 = qAcos((qSin(lat2) - qSin(lat1) * qCos(d)) / (qSin(d) * qCos(lat1)));
    else
        tc1 = 2 * M_PI - qAcos((qSin(lat2) - qSin(lat1) * qCos(d)) / (qSin(d) * qCos(lat1)));
    return 360. - (tc1 / Pi180);
}

QPair<double, double> NavData::greatCircleFraction(double lat1, double lon1, double lat2, double lon2,
                                  double f) {
    if (qFuzzyCompare(lat1, lat2) && qFuzzyCompare(lon1, lon2))
        return QPair<double, double>(lat1, lon1);

    lat1 *= Pi180;
    lon1 *= Pi180;
    lat2 *= Pi180;
    lon2 *= Pi180;

    double d = qAcos(qSin(lat1) * qSin(lat2) + qCos(lat1) * qCos(lat2) * qCos(lon1-lon2));

    double A = qSin((1. - f) * d) / qSin(d);
    double B = qSin(f*d) / qSin(d);
    double x = A * qCos(lat1) * qCos(lon1) + B * qCos(lat2) * qCos(lon2);
    double y = A * qCos(lat1) * qSin(lon1) + B * qCos(lat2) * qSin(lon2);
    double z = A * qSin(lat1)             + B * qSin(lat2);
    double rLat = qAtan2(z, sqrt(x*x + y*y));
    double rLon = qAtan2(y, x);

    return QPair<double, double>(rLat / Pi180, rLon / Pi180);
}

QList<QPair<double, double> > NavData::greatCirclePoints(double lat1, double lon1, double lat2, double lon2,
                                                         double intervalNm) { // omits last point
    QList<QPair<double, double> > result;
    if (qFuzzyCompare(lat1, lat2) && qFuzzyCompare(lon1, lon2))
        return (result << QPair<double, double>(lat1, lon1));
    double fractionIncrement = qMin(1., intervalNm / NavData::distance(lat1, lon1, lat2, lon2));
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
            foreach(const DoublePair p, greatCirclePoints(wpOld.first, wpOld.second,
                                                    points[i].first, points[i].second))
                VERTEX(p.first, p.second);
            wpOld = points[i];
        }
    }
    VERTEX(points.last().first, points.last().second); // last points gets ommitted by greatCirclePoints by design
}

/* converts (oceanic) points from ARINC424 format
  @return 0 on error
*/
QPair<double, double> *NavData::fromArinc(const QString &str) {
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

/* converts (oceanic) points from ARINC424 format
  @return QString("") on error
*/
QString NavData::toArinc(const short lat, const short lon) { // returning QString() on error
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



QString NavData::getAirline(QString airlineCode)
{
    QString result;
    result = airlineCodes[airlineCode];
    if(result.isEmpty()) result = tr("general aviation");
    return result;
}
