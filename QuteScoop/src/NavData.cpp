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
        if (createIfNoInstance)
            navDataInstance = new NavData();
    return navDataInstance;
}

NavData::NavData() {
    loadAirports(Settings::applicationDataDirectory("data/airports.dat"));
    loadSectors();
    loadCountryCodes(Settings::applicationDataDirectory("data/countrycodes.dat"));
    if(Settings::checkForUpdates())
        checkForDataUpdates();
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

void NavData::accept(MapObjectVisitor* visitor) {
    foreach(Airport *a, airports)
        visitor->visit(a);
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
            foreach(const DoublePair p, greatCirclePoints(wpOld.first, wpOld.second,
                                                    points[i].first, points[i].second))
                VERTEX(p.first, p.second);
            wpOld = points[i];
        }
    }
    VERTEX(points.last().first, points.last().second); // last points gets ommitted by greatCirclePoints by design
}

QPair<double, double> *NavData::fromArinc(const QString &str) { // returning 0 on error
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

void NavData::checkForDataUpdates() {
    if(dataVersionsAndFilesDownloader != 0)
        dataVersionsAndFilesDownloader = 0;
    dataVersionsAndFilesDownloader = new QHttp(this);
    QUrl url("http://qutescoop.svn.sourceforge.net/svnroot/qutescoop/trunk/QuteScoop/data/dataversions.txt");
    dataVersionsAndFilesDownloader->setHost(url.host());
    Settings::applyProxySetting(dataVersionsAndFilesDownloader);

    connect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)), this, SLOT(dataVersionsDownloaded(bool)));

    dataVersionsBuffer = new QBuffer;
    dataVersionsBuffer->open(QBuffer::ReadWrite);

    dataVersionsAndFilesDownloader->get(url.path(), dataVersionsBuffer);
    qDebug() << "Checking for datafile versions:" << url.toString();
}

void NavData::dataVersionsDownloaded(bool error) {
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)), this, SLOT(dataVersionsDownloaded(bool)));
    if(dataVersionsBuffer == 0)
        return;

    if(error) {
        GuiMessages::criticalUserInteraction(dataVersionsAndFilesDownloader->errorString(), "Datafile download");
        return;
    }
    QList< QPair< QString, int> > serverDataVersionsList, localDataVersionsList;

    dataVersionsBuffer->seek(0);
    while(dataVersionsBuffer->canReadLine()) {
        QStringList splitLine = QString(dataVersionsBuffer->readLine()).split("%%");
        QPair< QString, int> rawPair;
        rawPair.first = splitLine.first();
        rawPair.second = splitLine.last().toInt();
        if (splitLine.size() == 2) // only xx%%xx accepted
            serverDataVersionsList.append(rawPair);
        //qDebug() << "Server versions are " << rawPair.first << " : " << rawPair.second;
    }

    QFile localVersionsFile(Settings::applicationDataDirectory("data/dataversions.txt"));
    if (!localVersionsFile.open(QIODevice::ReadOnly | QIODevice::Text))  {
        GuiMessages::informationUserAttention(QString("Could not read %1.\nThus we are updating all datafiles.")
                       .arg(localVersionsFile.fileName()),
                       "Complete datafiles update necessary");
    }
    while(!localVersionsFile.atEnd()) {
        QStringList splitLine = QString(localVersionsFile.readLine()).split("%%");
        QPair< QString, int> rawPair;
        rawPair.first = splitLine.first();
        rawPair.second = splitLine.last().toInt();
        if (splitLine.size() == 2) // only xx%%xx accepted
            localDataVersionsList.append(rawPair);
        //qDebug() << "Local versions are " << rawPair.first << " : " << rawPair.second;
    }
    localVersionsFile.close();

    //collecting files to update
    connect(dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
            this, SLOT(dataFilesRequestFinished(int,bool)));
    connect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
            this, SLOT(dataFilesDownloaded(bool)));
    for(int i = 0; i < serverDataVersionsList.size(); i++) {
        // download also files that are locally not available
        if(serverDataVersionsList[i].second >
           localDataVersionsList.value(i, QPair< QString, int>(QString(), 0)).second){
            dataFilesToDownload.append(new QFile(Settings::applicationDataDirectory("data/%1.newFromServer")
                                                 .arg(serverDataVersionsList[i].first)));
            QUrl url(QString("http://qutescoop.svn.sourceforge.net/svnroot/qutescoop/trunk/QuteScoop/data/%1")
                 .arg(serverDataVersionsList[i].first));
            dataVersionsAndFilesDownloader->get(url.path(), dataFilesToDownload.last());
            //qDebug() << "Downloading datafile" << url.toString();
        }
    }
    if (!dataFilesToDownload.isEmpty())
        GuiMessages::informationUserAttention(QString("New sector-/ airport- or geography-files are available. They will be downloaded now."),
                                             "New datafiles");
    else {
        disconnect(dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
                this, SLOT(dataFilesRequestFinished(int,bool)));
        disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
                this, SLOT(dataFilesDownloaded(bool)));
        dataVersionsAndFilesDownloader->abort();
        delete dataVersionsAndFilesDownloader;
        delete dataVersionsBuffer;
    }
}

void NavData::dataFilesRequestFinished(int id, bool error) {
    if (error) {
        GuiMessages::criticalUserInteraction(QString("Error downloading %1:\n%2")
                                            .arg(dataVersionsAndFilesDownloader->currentRequest().path())
                                            .arg(dataVersionsAndFilesDownloader->errorString()),
                                            "New datafiles");
        return;
    }
    GuiMessages::informationUserAttention(QString("Downloaded %1")
                                         .arg(dataVersionsAndFilesDownloader->currentRequest().path()),
                                         "New datafiles");
}

void NavData::dataFilesDownloaded(bool error) {
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
            this, SLOT(dataFilesRequestFinished(int,bool)));
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
            this, SLOT(dataFilesDownloaded(bool)));
    if(dataVersionsBuffer == 0)
        return;

    if(error) {
        GuiMessages::criticalUserInteraction(QString("New sector- / airport- / geography-files could not be downloaded.\n%1")
                                            .arg(dataVersionsAndFilesDownloader->errorString()),
                                            "New datafiles");
        return;
    }

    GuiMessages::informationUserAttention("All scheduled files have been downloaded.\nThese changes will take effect on the next start of QuteScoop.",
                                         "New datafiles");

    int errors = 0;
    for(int i = 0; i < dataFilesToDownload.size(); i++) {
        dataFilesToDownload[i]->flush();
        dataFilesToDownload[i]->close();

        if(dataFilesToDownload[i]->exists()) {
            QString datafileFilePath = dataFilesToDownload[i]->fileName().remove(".newFromServer");
            if (QFile::exists(datafileFilePath) && !QFile::remove(datafileFilePath)) {
                GuiMessages::criticalUserInteraction(QString("Unable to delete\n%1")
                                                    .arg(datafileFilePath), "New datafiles");
                errors++;
            }
            if (!dataFilesToDownload[i]->rename(datafileFilePath)) {
                GuiMessages::criticalUserInteraction(QString("Unable to move downloaded file to\n%1")
                                                    .arg(datafileFilePath), "New datafiles");
                errors++;
            }
        }

        delete dataFilesToDownload[i];
    }

    if (errors == 0) {
        QFile localDataVersionsFile(Settings::applicationDataDirectory("data/dataversions.txt"));
        if (localDataVersionsFile.open(QIODevice::WriteOnly))
            localDataVersionsFile.write(dataVersionsBuffer->data());
        else
            GuiMessages::criticalUserInteraction(QString("Error writing %1").arg(localDataVersionsFile.fileName()),
                                                "New datafiles");
    } else
        GuiMessages::criticalUserInteraction(QString("Errors occured. All datafiles will be redownloaded on next launch of QuteScoop."),
                                            "New datafiles");

    dataVersionsBuffer->close();
    delete dataVersionsBuffer;
    dataVersionsAndFilesDownloader->abort();
    delete dataVersionsAndFilesDownloader;
    dataFilesToDownload.clear();
}
