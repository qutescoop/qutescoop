/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "NavData.h"

#include "Airport.h"
#include "FileReader.h"
#include "helpers.h"
#include "SectorReader.h"
#include "Settings.h"

#include <QRegExp>

NavData *navDataInstance = 0;
NavData *NavData::instance(bool createIfNoInstance) {
    if(navDataInstance == 0 && createIfNoInstance)
        navDataInstance = new NavData();
    return navDataInstance;
}

NavData::NavData() {
}

NavData::~NavData() {
    foreach(const Airport *a, airports)
        delete a;
    foreach(const Sector *s, sectors)
        delete s;
}

void NavData::load() {
    loadCountryCodes(Settings::dataDirectory("data/countrycodes.dat"));
    loadAirports(Settings::dataDirectory("data/airports.dat"));
    loadControllerAirportsMapping(Settings::dataDirectory("data/controllerAirportsMapping.dat"));
    loadSectors();
    loadAirlineCodes(Settings::dataDirectory("data/airlines.dat"));
    emit loaded();
}

void NavData::loadAirports(const QString& filename) {
    airports.clear();
    activeAirports.clear();
    FileReader fr(filename);

    auto countMissingCountry = 0;

    auto count = 0;
    while (!fr.atEnd()) {
        ++count;
        QString _line = fr.nextLine().trimmed();

        if(_line.isEmpty() || _line.startsWith(";")) {
            continue;
        }

        Airport *airport = new Airport(_line.split(':'), count);

        if (airport->countryCode == 0) {
            ++countMissingCountry;
        }

        if (airport != 0 && !airport->isNull()) {
            airports[airport->label] = airport;
        }
    }

    if (countMissingCountry != 0) {
        auto msg = QString("%1 airports are missing a country code. Please help by adding them in data/airports.dat.")
                       .arg(countMissingCountry);
        qWarning() << "NavData::loadAirports()" << msg;
        QTextStream(stdout) << "WARNING: " << msg << Qt::endl;
    }
}

void NavData::loadControllerAirportsMapping(const QString &filePath)
{
    m_controllerAirportsMapping.clear();
    FileReader fr(filePath);
    while(!fr.atEnd()) {
        QString _line = fr.nextLine().trimmed();

        if(_line.isEmpty() || _line.startsWith(";")) {
            continue;
        }

        QStringList _fields = _line.split(':');
        if(_fields.size() != 3) {
            auto msg = QString("Could not load line '%1' (%2 fields) from %3")
                           .arg(_line).arg(_fields.count()).arg(filePath);
            qCritical() << "NavData::loadControllerAirportsMapping()" << msg;
            QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
            exit(EXIT_FAILURE);
        }

        ControllerAirportsMapping _cam;
        _cam.prefix = _fields[0];
        _cam.suffixes = _fields[1].split(" ", Qt::SkipEmptyParts);
        foreach(const auto _airportIcao, _fields[2].split(" ", Qt::SkipEmptyParts)) {
            if (airports.contains(_airportIcao)) {
                _cam.airports.append(airports.value(_airportIcao));
            } else {
                auto msg = QString("While processing line '%1' from %2: Airport '%3' not found.")
                               .arg(_line, filePath, _airportIcao);
                qCritical() << "NavData::loadControllerAirportsMapping()" << msg;
                QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
                exit(EXIT_FAILURE);
            }
        }

        m_controllerAirportsMapping.append(_cam);
    }
}

void NavData::loadCountryCodes(const QString& filePath) {
    countryCodes.clear();
    FileReader fr(filePath);
    while (!fr.atEnd()) {
        QString _line = fr.nextLine().trimmed();

        if(_line.isEmpty() || _line.startsWith(";")) {
            continue;
        }

        QStringList _fields = _line.split(':');
        if(_fields.size() != 2) {
            auto msg = QString("Could not load line '%1' (%2 fields) from %3")
                           .arg(_line).arg(_fields.count()).arg(filePath);
            qCritical() << "NavData::loadCountryCodes()" << msg;
            QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
            exit(EXIT_FAILURE);
        }
        countryCodes[_fields.first()] = _fields.last();
    }
}

void NavData::loadSectors() {
    SectorReader().loadSectors(sectors);
}

void NavData::loadAirlineCodes(const QString &filePath) {
    qDebug() << "loading airlines from" << filePath;
    foreach(const auto _a, airlines) {
        delete _a;
    }
    airlines.clear();
    if(filePath.isEmpty()) {
        qWarning() << "NavData::loadAirlineCodes() -- bad filename";
        return;
    }

    auto count = 0;
    FileReader fr(filePath);
    while(!fr.atEnd()) {
        QString _line = fr.nextLine();

        if(_line.isEmpty() || _line.startsWith(";")) {
            continue;
        }

        QStringList _fields = _line.split(0x09);   // 0x09 code for Tabulator
        if (_fields.count() != 4) {
            auto msg = QString("Could not load line '%1' (%2 fields) from %3")
                .arg(_line).arg(_fields.count()).arg(filePath);
            qCritical() << "NavData::loadAirlineCodes()" << msg;
            QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
            exit(EXIT_FAILURE);
        }

        auto airline = new Airline(_fields[0], _fields[1], _fields[2], _fields[3]);
        airlines[airline->code] = airline;
        count++;
    }
    qDebug() << "loaded" << count << "airlines";
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

QList<Airport*> NavData::additionalMatchedAirportsForController(QString prefix, QString suffix) const
{
    QList<Airport*> ret;
    foreach(auto _cam, m_controllerAirportsMapping) {
        if (
            _cam.prefix == prefix
            && (_cam.suffixes.isEmpty() || _cam.suffixes.contains(suffix))
        ) {
            ret.append(_cam.airports);
        }
    }

    return ret;
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
        foreach(auto _airport, c->airports()) {
            if(c->isAppDep()) {
                _airport->addApproach(c);
            }
            if(c->isTwr()) {
                _airport->addTower(c);
            }
            if(c->isGnd()) {
                _airport->addGround(c);
            }
            if(c->isDel()) {
                _airport->addDelivery(c);
            }
            if(c->isAtis()) {
                _airport->addAtis(c);
            }
            newActiveAirportsSet.insert(_airport);
        }
    }

    activeAirports.clear();
    foreach(Airport *a, newActiveAirportsSet) {
        int congestion = a->numFilteredArrivals + a->numFilteredDepartures;
        activeAirports.insert(congestion, a);
    }

    qDebug() << "NavData::updateData() -- finished";
    qApp->restoreOverrideCursor();
}

void NavData::accept(SearchVisitor* visitor) {
    foreach(Airport *a, airports) {
        visitor->visit(a);
    }
    visitor->airlines = airlines;
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

/**
  plot great-circles of lat/lon points on Earth
**/
void NavData::plotGreatCirclePoints(const QList<QPair<double, double> > &points) {
    if (points.isEmpty())
        return;
    if (points.size() > 1) {
        DoublePair wpOld = points[0];
        for (int i=1; i < points.size(); i++) {
            auto subPoints = greatCirclePoints(
                wpOld.first, wpOld.second,
                points[i].first, points[i].second,
                400.
            );
            foreach(const DoublePair p, subPoints) {
                VERTEX(p.first, p.second);
            }
            wpOld = points[i];
        }
    }
    VERTEX(points.last().first, points.last().second); // last points gets ommitted by greatCirclePoints by design
}

/** converts (oceanic) points from ARINC424 format
  @return 0 on error
*/
QPair<double, double>*NavData::fromArinc(const QString &str) {
    QRegExp arinc("(\\d{2})([NSEW]?)(\\d{2})([NSEW]?)");     // ARINC424 waypoints (strict)
    if (arinc.exactMatch(str)) {
        auto capturedTexts = arinc.capturedTexts();
        if (
            !capturedTexts[2].isEmpty()
            && !capturedTexts[4].isEmpty()
            ) {
            return 0;
        }

        if (
            !capturedTexts[2].isEmpty()
            || !capturedTexts[4].isEmpty()
        ) {
            double wLat = capturedTexts[1].toDouble();
            double wLon = capturedTexts[3].toDouble();
            if (
                QRegExp("[SW]").exactMatch(capturedTexts[2])
                || QRegExp("[SW]").exactMatch(capturedTexts[4])
            ) {
                wLat = -wLat;
            }
            if (!capturedTexts[2].isEmpty()) {
                wLon = wLon + 100.;
            }
            if (
                QRegExp("[NW]").exactMatch(capturedTexts[2])
                || QRegExp("[NW]").exactMatch(capturedTexts[4])
            ) {
                wLon = -wLon;
            }
            return new QPair<double, double>(wLat, wLon);
        }
    }
    return 0;
}

/** converts (oceanic) points to ARINC424 format
  @return QString("") on error
*/
QString NavData::toArinc(const short lat, const short lon) {
    if (qAbs(lat) > 90 || qAbs(lon) > 180) {
        return QString();
    }

    // bail out if our precision is not sufficient
    if (
        !qFuzzyCompare(round(lon * 100.), lon * 100.)
        || !qFuzzyCompare(round(lat * 100.), lat * 100.)) {
        return QString();
    }


	QString q; // ARINC 424 quadrant
    if (lat > 0) {
        if (lon > 0) {
			q = "E";
        } else {
			q = "N";
        }
    } else {
        if (lon > 0) {
            q = "S";
        } else {
			q = "W";
        }
	}

	return QString("%1%2%3%4").
			arg(qAbs(lat), 2, 10, QChar('0')).
			arg(qAbs(lon) >= 100? q: "").
			arg(qAbs(lon) >= 100? qAbs(lon) - 100: qAbs(lon), 2, 10, QChar('0')).
			arg(qAbs(lon) >= 100? "": q);
}

/** converts geographic points to "Eurocontrol" (that's just how I call it, it's the basic 40N030W) format
  @return QString("") on error
*/
QString NavData::toEurocontrol(double lat, double lon, const LatLngPrecission maxPrecision) {
    if (qAbs(lat) > 90 || qAbs(lon) > 180) {
        return QString();
    }
    QString latLetter = "N";
    if (lat < 0) {
        latLetter = "S";
        lat = -lat;
    }
    QString lonLetter = "E";
    if (lon < 0) {
        lonLetter = "W";
        lon = -lon;
    }

    ushort latDeg = floor(lat);
    ushort lonDeg = floor(lon);

    if (maxPrecision >= LatLngPrecission::Mins) {
        ushort latMin = (int)(lat * 60.) % 60;
        ushort lonMin = (int)(lon * 60.) % 60;

        if (maxPrecision >= LatLngPrecission::Secs) {
            ushort latSec = (int)(lat * 3600.) % 60;
            ushort lonSec = (int)(lon * 3600.) % 60;

            if (latSec != 0 || lonSec != 0) {
                return QString("%1%2%3%4%5%6%7%8")
                    .arg(latDeg, 2, 10, QChar('0'))
                    .arg(latMin, 2, 10, QChar('0'))
                    .arg(latSec, 2, 10, QChar('0'))
                    .arg(latLetter)
                    .arg(lonDeg, 3, 10, QChar('0'))
                    .arg(lonMin, 2, 10, QChar('0'))
                    .arg(lonSec, 2, 10, QChar('0'))
                    .arg(lonLetter);
            }
        }

        if (latMin != 0 || lonMin != 0) {
            return QString("%1%2%3%4%5%6")
                .arg(latDeg, 2, 10, QChar('0'))
                .arg(latMin, 2, 10, QChar('0'))
                .arg(latLetter)
                .arg(lonDeg, 3, 10, QChar('0'))
                .arg(lonMin, 2, 10, QChar('0'))
                .arg(lonLetter);
        }
    }

    return QString("%1%2%3%4")
        .arg(latDeg, 2, 10, QChar('0'))
        .arg(latLetter)
        .arg(lonDeg, 3, 10, QChar('0'))
        .arg(lonLetter);
}
