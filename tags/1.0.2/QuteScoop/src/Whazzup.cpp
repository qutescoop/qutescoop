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

#include <QHttp>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>
#include <QTimer>

#include "Settings.h"
#include "Whazzup.h"

Whazzup *whazzupInstance = 0;

Whazzup* Whazzup::getInstance() {
	if(whazzupInstance == 0)
		whazzupInstance = new Whazzup();
	return whazzupInstance;
}

Whazzup::Whazzup() {
	statusDownloader = new QHttp;
	whazzupDownloader = new QHttp;

	statusBuffer = 0;
	whazzupBuffer = 0;
	connect(statusDownloader, SIGNAL(done(bool)), this, SLOT(statusDownloaded(bool)));
	
	// init random seed to switch between URLs
        //srand(time(NULL));
	
	downloadTimer = new QTimer(this);
	connect(downloadTimer, SIGNAL(timeout()), this, SLOT(download()));
}

Whazzup::~Whazzup() {
	if(statusDownloader != 0) delete statusDownloader;
	if(statusBuffer != 0) delete statusBuffer;
	if(downloadTimer != 0) delete downloadTimer;
}

void Whazzup::setStatusLocation(const QString& statusLocation) {
	QUrl url(statusLocation);
	QFileInfo fileInfo(url.path());
	QString fileName = fileInfo.fileName();

	statusDownloader->abort();
	statusDownloader->setHost(url.host(), url.port() != -1 ? url.port() : 80);
	Settings::applyProxySetting(statusDownloader);
	
	if (!url.userName().isEmpty())
		statusDownloader->setUser(url.userName(), url.password());

	QString querystr = url.path() + "?" + url.encodedQuery();
	
	if(statusBuffer != 0) delete statusBuffer;
	statusBuffer = new QBuffer;
	statusBuffer->open(QBuffer::ReadWrite);
	statusDownloader->get(querystr, statusBuffer);
}

void Whazzup::statusDownloaded(bool error) {
	if(statusBuffer == 0)
		return;

	if(error) {
		emit downloadError(statusDownloader->errorString());
		return;
	}

	urls.clear();
	gzurls.clear();
	metarUrl = "";
	tafUrl = "";
	shorttafUrl = "";
	atisLink = "";
	userLink = "";
	
	statusBuffer->seek(0);
	while(statusBuffer->canReadLine()) {
		QString line = QString(statusBuffer->readLine()).trimmed();
		if(line.startsWith(";")) // ignore comments
			continue;

		QStringList list = line.split('=');
		
		if("msg0" == list[0]) {
			message = line.right(line.length() - QString("url0=").length());
			continue;
		}
		
		if(list.size() != 2) continue;
		
		if("url0" == list[0])
			urls.append(list[1]);
		else if("gzurl0" == list[0])
			gzurls.append(list[1]);
		else if("metar0" == list[0])
			metarUrl = list[1];
		else if("taf0" == list[0])
			tafUrl = list[1];
		else if("shorttaf0" == list[0])
			shorttafUrl = list[1];
		else if("user0" == list[0])
			userLink = list[1];
		else if("atis0" == list[0])
			atisLink = list[1];
	}
	
	if(!message.isEmpty())
		emit networkMessage(message);
	
	delete statusBuffer;
	statusBuffer = 0;
	lastDownloadTime = QTime();
	
	emit statusDownloaded();
}

void Whazzup::download() {
	if(urls.size() == 0)
		return;
	
	downloadTimer->stop();
	
	QTime now = QTime::currentTime();
	if(lastDownloadTime.secsTo(now) < 30)
		return; // don't allow download intervals < 30 seconds
	lastDownloadTime = now;
	
	int index = rand() % urls.size();
	QString fileLocation = urls[index];
	
	qDebug() << "Downloading whazzup from" << fileLocation;
	
	QUrl url(fileLocation);
	QFileInfo fileInfo(url.path());
	QString fileName = fileInfo.fileName();

	if(whazzupDownloader != 0) {
		whazzupDownloader->abort();
		delete whazzupDownloader;
	}
	whazzupDownloader = new QHttp(this);
	connect(whazzupDownloader, SIGNAL(done(bool)), this, SLOT(whazzupDownloaded(bool)));
	Settings::applyProxySetting(whazzupDownloader);
	
	whazzupDownloader->setHost(url.host(), url.port() != -1 ? url.port() : 80);
	if (!url.userName().isEmpty())
		whazzupDownloader->setUser(url.userName(), url.password());

	QString querystr = url.path() + "?" + url.encodedQuery();
	
	if(whazzupBuffer != 0) delete whazzupBuffer;
	whazzupBuffer = new QBuffer;
	whazzupBuffer->open(QBuffer::ReadWrite);
	whazzupDownloader->get(querystr, whazzupBuffer);
}

void Whazzup::whazzupDownloaded(bool error) {
	if(whazzupBuffer == 0)
		return;
	
	if(Settings::downloadPeriodically())
		downloadTimer->start(Settings::downloadInterval() * 60 * 1000);

	if(whazzupBuffer->data().isEmpty()) {
		return;
	}
	
	if(error) {
		emit downloadError(statusDownloader->errorString());
		return;
	}
	
	whazzupBuffer->seek(0);
	WhazzupData newWhazzupData(whazzupBuffer);
	if(!newWhazzupData.isNull()) {
		data.updateFrom(newWhazzupData);
		emit newData();
	}
}

QString Whazzup::getUserLink(const QString& id) const {
	if(userLink.isEmpty())
		return QString();
	return userLink + "?id=" + id;
}

QString Whazzup::getAtisLink(const QString& id) const {
	if(metarUrl.isEmpty())
		return QString();
	return metarUrl + "?id=" + id;
}
