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

#ifndef WHAZZUP_H_
#define WHAZZUP_H_

#include <QStringList>
#include <QList>
#include <QHttp>
#include <QBuffer>
#include <QTime>
#include <QTimer>

#include "WhazzupData.h"

class WhazzupData;

class Whazzup: public QObject
{
	Q_OBJECT
	
public:
	static Whazzup* getInstance();
	
	/**
	 * Set the download location for whazzup status file
	 */
	void setStatusLocation(const QString& url);
	
	const WhazzupData& whazzupData() { return data; }
	
	QString getUserLink(const QString& id) const;
	QString getAtisLink(const QString& id) const;
	
signals:
	void newData();
	void networkMessage(QString message);
	void downloadError(QString message);
	void statusDownloaded();
	
public slots:
	void download();
	
private slots:
	void statusDownloaded(bool error);
	void whazzupDownloaded(bool error);
		
private:
	Whazzup();
	virtual ~Whazzup();
	
	WhazzupData data;

	QHttp *statusDownloader;
	QBuffer *statusBuffer;
	
	QHttp *whazzupDownloader;
	QBuffer *whazzupBuffer;
	
	QStringList urls;
	QStringList gzurls;
	QString metarUrl;
	QString tafUrl;
	QString shorttafUrl;
	QString userLink;
	QString atisLink;
	QString message;
	
	QTime lastDownloadTime;
	
	QTimer *downloadTimer;
};

#endif /*WHAZZUP_H_*/
