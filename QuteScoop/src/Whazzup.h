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
#include <QPair>
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
    const WhazzupData& whazzupData() { return (predictedTime.isValid()? predictedData: data); } // we fake it when predicting a certain time
    const WhazzupData& realWhazzupData() { return data; } // this is always the really downloaded thing

    void setPredictedTime(QDateTime predictedTime);
    QDateTime getPredictedTime() const { return predictedTime; }

    QString getUserLink(const QString& id) const;
    QString getAtisLink(const QString& id) const;

    QList<QPair<QDateTime, QString>*> getDownloadedWhazzups();

signals:
    void newData(bool isNew);
    void networkMessage(QString message);
    void downloadError(QString message);
    void statusDownloaded();
    void needBookings();

public slots:
    void download();
    void fromFile(QString filename);
    void setStatusLocation(const QString& url);
    void downloadBookings();

private slots:
    void statusDownloaded(bool error);
    void whazzupDownloading(int prog, int tot);
    void bookingsDownloading(int prog, int tot);
    void whazzupDownloaded(bool error);
    void bookingsDownloaded(bool error);

private:
    Whazzup();
    virtual ~Whazzup();

    WhazzupData data;
    WhazzupData predictedData;

    QDateTime predictedTime;

    QHttp *statusDownloader;
    QBuffer *statusBuffer;

    QHttp *whazzupDownloader;
    QBuffer *whazzupBuffer;

    QHttp *bookingsDownloader;
    QBuffer *bookingsBuffer;

    QStringList urls;
    QStringList gzurls;
    QString metarUrl;
    QString tafUrl;
    QString shorttafUrl;
    QString userLink;
    QString atisLink;
    QString message;

    QTime lastDownloadTime;

    QTimer *downloadTimer, *bookingsTimer;
};

#endif /*WHAZZUP_H_*/
