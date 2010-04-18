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

#include "time.h"

#include <QHttp>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QtGui>

#include "Settings.h"
#include "Whazzup.h"
#include "Window.h"

Whazzup *whazzupInstance = 0;

Whazzup* Whazzup::getInstance() {
    if(whazzupInstance == 0)
        whazzupInstance = new Whazzup();
    return whazzupInstance;
}

Whazzup::Whazzup() {
    statusDownloader = new QHttp;
    whazzupDownloader = new QHttp;
    bookingsDownloader = new QHttp;

    statusBuffer = 0;
    whazzupBuffer = 0;
    bookingsBuffer = 0;
    connect(statusDownloader, SIGNAL(done(bool)), this, SLOT(statusDownloaded(bool)));

    // init random seed to switch between URLs
    srand(time(0));

    downloadTimer = new QTimer(this);
    bookingsTimer = new QTimer(this);
    connect(downloadTimer, SIGNAL(timeout()), this, SLOT(download()));
    connect(bookingsTimer, SIGNAL(timeout()), this, SLOT(downloadBookings()));

    connect(this, SIGNAL(needBookings()), this, SLOT(downloadBookings()));
}

Whazzup::~Whazzup() {
    if(statusDownloader != 0) delete statusDownloader;
    if(statusBuffer != 0) delete statusBuffer;
    if(downloadTimer != 0) delete downloadTimer;
    if(bookingsTimer != 0) delete bookingsTimer;
}

void Whazzup::setStatusLocation(const QString& statusLocation) {
    qDebug() << "Downloading network status from\t" << statusLocation;

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
    //qDebug() << "Status received";
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
            message = line.right(line.length() - QString("msg0=").length());
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

    qDebug() << "Got network status:\t" << urls.size() << "Whazzup URLs";

    if(urls.size() == 0){
        emit networkMessage("No Whazzup URLs found. Try again later.");
    } else {
        emit statusDownloaded();
    }
}

void Whazzup::fromFile(QString filename) {
    qDebug() << "fromFile" << filename;
    /*if(whazzupDownloader != 0) {
        whazzupDownloader->abort();
        delete whazzupDownloader;
    }
    */
    QFile *file = new QFile(filename);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "File Error on open";
        return;
    }
    // splash screen
    QPixmap pixmap(":/splash/splash");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();
    splash->showMessage("Loading Whazzup from file...", Qt::AlignCenter, QColor(0, 24, 81));

    if(whazzupBuffer != 0)
        whazzupBuffer->close();
        delete whazzupBuffer;
    whazzupBuffer = new QBuffer;
    whazzupBuffer->open(QBuffer::WriteOnly);
    whazzupBuffer->write(file->readAll());
    whazzupBuffer->close();
    whazzupDownloaded(false);
    splash->finish(Window::getInstance());
}

void Whazzup::download() {
    if(urls.size() == 0) {
        emit downloadError("No Whazzup URLs in network status. Trying to get a new network status.");
        setStatusLocation(Settings::statusLocation());
        qDebug() << "no Whazzup URLs available";
        return;
    }

    downloadTimer->stop();
    QTime now = QTime::currentTime();
    if(!data.isNull()) {
        if(data.updateEarliest().isValid()
                && QDateTime::currentDateTime().toUTC().secsTo(data.updateEarliest()) > 0
                && Settings::downloadNetwork() == data.network()) {
            Window::getInstance()->statusBar()->showMessage(
                    QString("Server said new data available in %1s. Skipping.")
                    .arg(QDateTime::currentDateTime().toUTC().secsTo(data.updateEarliest())), 3000
                    );
            return; // Server said we do not need to update
        }
    } else if(lastDownloadTime.secsTo(now) < 30) {
        Window::getInstance()->statusBar()->showMessage(
                QString("Whazzup checked %1s (less than 30s) ago. Skipping.")
                .arg(lastDownloadTime.secsTo(now)), 3000
                );
        qDebug() << "Whazzup already checked less than 30 seconds ago. Skipping.";
        return; // don't allow download intervals < 30s
    }
    lastDownloadTime = now;

    int index = rand() % urls.size();
    QString fileLocation = urls[index];

    QUrl url(fileLocation);
    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();

    Window::getInstance()->setStatusText(QString("Updating whazzup from %1").arg(url.toString(QUrl::RemoveUserInfo)));
    qDebug() << "Downloading Whazzup from\t" << fileLocation;

    if(whazzupDownloader != 0) {
        whazzupDownloader->abort();
        delete whazzupDownloader;
    }
    whazzupDownloader = new QHttp(this);
    connect(whazzupDownloader, SIGNAL(done(bool)), this, SLOT(whazzupDownloaded(bool)));
    connect(whazzupDownloader, SIGNAL(dataReadProgress(int,int)), this, SLOT(whazzupDownloading(int,int)));
    Settings::applyProxySetting(whazzupDownloader);

    whazzupDownloader->setHost(url.host(), url.port() != -1 ? url.port() : 80);
    if (!url.userName().isEmpty())
        whazzupDownloader->setUser(url.userName(), url.password());

    QString querystr = url.path() + "?" + url.encodedQuery();

    if(whazzupBuffer != 0)
        whazzupBuffer->close();
        delete whazzupBuffer;
    whazzupBuffer = new QBuffer;
    whazzupBuffer->open(QBuffer::ReadWrite);
    whazzupDownloader->get(querystr, whazzupBuffer);
}

void Whazzup::whazzupDownloading(int prog, int tot) {
    Window::getInstance()->setProgressBar(prog, tot);
}

void Whazzup::whazzupDownloaded(bool error) {
    //qDebug() << "Whazzup received";
    Window::getInstance()->setProgressBar(false);
    if(whazzupBuffer == 0) {
        emit downloadError("Download Error. Buffer unavailable.");
        emit newData(false); // update statusbar
        return;
    }

    if(Settings::downloadPeriodically())
        downloadTimer->start(Settings::downloadInterval() * 60 * 1000);

    if(whazzupBuffer->data().isEmpty()) {
        emit networkMessage("No data in Whazzup.");
        emit newData(false); // update statusbar
        return;
    }

    if(error) {
        emit downloadError(statusDownloader->errorString());
        emit newData(false); // update statusbar
        return;
    }
    // splash screen
    QPixmap pixmap(":/splash/splash");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();
    splash->showMessage("Processing Whazzup...", Qt::AlignCenter, QColor(0, 24, 81));

    whazzupBuffer->open(QBuffer::ReadOnly); // maybe fixes some issues we encounter very rarely
    whazzupBuffer->seek(0);
    WhazzupData newWhazzupData(whazzupBuffer, WhazzupData::WHAZZUP);
    whazzupBuffer->close();
    if(!newWhazzupData.isNull()) {
        if(newWhazzupData.timestamp().secsTo(QDateTime::currentDateTime().toUTC()) > 60 * 30)
            emit networkMessage("Whazzup data more than 30 minutes old.");

        if(newWhazzupData.timestamp() != data.timestamp()) {
            data.updateFrom(newWhazzupData);
            qDebug() << "Whazzup updated from timestamp\t" << data.timestamp().toString();

            // write out whazzup to a file
            QString filename = QString("downloaded/%1_%2.whazzup")
                      .arg(Settings::downloadNetwork())
                      .arg(data.timestamp().toString("yyyyMMdd-HHmmss"));
            QFile out(filename);
            if (out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                out.write(whazzupBuffer->data());
            } else {
                qDebug() << "Info: Could not write Whazzup to disk" << out.fileName();
            }

            emit newData(true);
        } else {
            Window::getInstance()->statusBar()->showMessage(
                    QString("We already have Whazzup with that Timestamp: %1")
                    .arg(data.timestamp().toString("ddd MM/dd HHmm'z'")), 3000);
            //qDebug() << "We already have Whazzup with that Timestamp\t" << data.timestamp().toString();
            emit newData(false);
        }
    }
    splash->finish(Window::getInstance());
}

void Whazzup::downloadBookings() {
    QUrl url(Settings::bookingsLocation());
    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();

    bookingsTimer->stop();

    Window::getInstance()->setStatusText(QString("Updating ATC Bookings from %1").arg(url.toString(QUrl::RemoveUserInfo)));
    qDebug() << "Downloading ATC bookings from\t" << url.toString(QUrl::RemoveUserInfo);

    if(bookingsDownloader != 0) {
        bookingsDownloader->abort();
        delete bookingsDownloader;
    }
    bookingsDownloader = new QHttp(this);
    connect(bookingsDownloader, SIGNAL(done(bool)), this, SLOT(bookingsDownloaded(bool)));
    connect(bookingsDownloader, SIGNAL(dataReadProgress(int,int)), this, SLOT(bookingsDownloading(int,int)));
    Settings::applyProxySetting(bookingsDownloader);

    bookingsDownloader->setHost(url.host(), url.port() != -1 ? url.port() : 80);
    if (!url.userName().isEmpty())
        bookingsDownloader->setUser(url.userName(), url.password());

    QString querystr = url.path() + "?" + url.encodedQuery();

    if(bookingsBuffer != 0)
        delete bookingsBuffer;
    bookingsBuffer = new QBuffer;
    bookingsBuffer->open(QBuffer::ReadWrite);
    bookingsDownloader->get(querystr, bookingsBuffer);
}

void Whazzup::bookingsDownloading(int prog, int tot) {
    Window::getInstance()->setProgressBar(prog, tot);
}

void Whazzup::bookingsDownloaded(bool error) {
    //qDebug() << "Bookings received";
    Window::getInstance()->setProgressBar(false);
    if(bookingsBuffer == 0) {
        emit downloadError("Download Error. Buffer unavailable.");
        emit newData(false); // update statusbar
        return;
    }

    if(Settings::downloadBookings() && Settings::bookingsPeriodically()) {
        bookingsTimer->start(Settings::bookingsInterval() * 60 * 1000);
    }

    bookingsBuffer->open(QBuffer::ReadOnly); // maybe fixes some issues we encounter very rarely
    if(bookingsBuffer->data().isEmpty()) {
        emit networkMessage("No data in Bookings.");
        emit newData(false); // update statusbar
        return;
    }

    if(error) {
        emit downloadError(bookingsDownloader->errorString());
        emit newData(false); // update statusbar
        return;
    }

    // splash screen
    QPixmap pixmap(":/splash/splash");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();
    splash->showMessage("Processing Bookings...", Qt::AlignCenter, QColor(0, 24, 81));

    WhazzupData newBookingsData(bookingsBuffer, WhazzupData::ATCBOOKINGS);
    bookingsBuffer->close();
    if(!newBookingsData.isNull()) {
        if(newBookingsData.bookingsTimestamp().secsTo(QDateTime::currentDateTime().toUTC()) > 60 * 60 * 3)
                emit networkMessage("Bookings data more than 3 hours old.");

        if(newBookingsData.bookingsTimestamp() != data.bookingsTimestamp()) {
            data.updateFrom(newBookingsData);
            qDebug() << "Bookings updated from timestamp\t" << data.bookingsTimestamp().toString();

            QFile out(QString("downloaded/%1_%2.bookings")
                      .arg(Settings::downloadNetwork())
                      .arg(data.bookingsTimestamp().toString("yyMMdd-HHmmss")));
            if (out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                out.write(bookingsBuffer->data());
            } else {
                qDebug() << "Info: Could not write Bookings to disk" << out.fileName();
            }

            emit newData(true);
        } else {
            Window::getInstance()->statusBar()->showMessage(
                    QString("We already have Bookings with that Timestamp: %1")
                    .arg(data.bookingsTimestamp().toString("ddd MM/dd HHmm'z'")), 3000);
            //qDebug() << "We already have Bookings with that Timestamp\t" << data.bookingsTimestamp().toString();
            emit newData(false);
        }
    }
    splash->finish(Window::getInstance());
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

void Whazzup::setPredictedTime(QDateTime predictedTime) {
    this->predictedTime = predictedTime;
    if (Settings::downloadBookings() && !data.bookingsTimestamp().isValid()) {
        emit needBookings();
    }

    WhazzupData newdata = WhazzupData(predictedTime, data);
    predictedData.updateFrom(newdata);
    emit newData(true);
}

QList<QPair<QDateTime, QString>*> Whazzup::getDownloadedWhazzups() {
    // Process directory
    QDir myDir("downloaded/");
    QStringList list = myDir.entryList(QStringList(QString("%1_*.whazzup")
                            .arg(Settings::downloadNetwork()))
                            , QDir::Files | QDir::Readable);
    qDebug() << "getDLWhazzup" << list;
    for (int i = 0; i < list.size(); i++) {
        QRegExp dtRe = QRegExp("\\d+_(\\d{8}-\\d{6})");
        qDebug() << list[i] << dtRe.indexIn(list[i]);
        if (dtRe.indexIn(list[i]) > 0) {
            qDebug() << dtRe.cap(1);
            QDateTime dt = QDateTime::fromString(dtRe.cap(1), "yyyyMMdd-HHmmss");
        }
    }
    return QList<QPair<QDateTime, QString>*>();
}
