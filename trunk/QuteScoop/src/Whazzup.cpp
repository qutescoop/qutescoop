/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Whazzup.h"

#include <QHttp>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QtGui>
#include "time.h"

#include "Settings.h"
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
        emit hasGuiMessage(statusDownloader->errorString(), GuiMessage::Warning);
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
        emit hasGuiMessage(message, GuiMessage::Warning);

    delete statusBuffer;
    statusBuffer = 0;
    lastDownloadTime = QTime();

    qDebug() << "Got network status:\t" << urls.size() << "Whazzup URLs";

    if(urls.size() == 0){
        emit hasGuiMessage("No Whazzup-URLs found. Try again later.", GuiMessage::Warning);
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
        qDebug() << "Error opening" << filename;
        return;
    }
    emit hasGuiMessage("Loading Whazzup from file...");

    if(whazzupBuffer != 0) {
        whazzupBuffer->close();
        delete whazzupBuffer;
    }
    whazzupBuffer = new QBuffer;
    whazzupBuffer->open(QBuffer::WriteOnly);
    whazzupBuffer->write(file->readAll());
    whazzupBuffer->close();
    whazzupDownloaded(false);
}

void Whazzup::download() {
    if(urls.size() == 0) {
        emit hasGuiMessage("No Whazzup URLs in network status. Trying to get a new network status.", GuiMessage::ErrorUserAttention);
        setStatusLocation(Settings::statusLocation());
        return;
    }

    downloadTimer->stop();
    QTime now = QTime::currentTime();

    if(lastDownloadTime.secsTo(now) < 30) {
        emit hasGuiMessage(QString("Whazzup checked %1s (less than 30s) ago. Download scheduled.")
                           .arg(lastDownloadTime.secsTo(now)), GuiMessage::Temporary);
        downloadTimer->start((30 - lastDownloadTime.secsTo(now)) * 1000);
        return; // don't allow download intervals < 30s
    }
    lastDownloadTime = now;

    QUrl url(urls[qrand() % urls.size()]);

    emit hasGuiMessage(QString("Updating whazzup from %1").arg(url.toString(QUrl::RemoveUserInfo)), GuiMessage::ProgressBar, "whazzupDownload");

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
    emit hasGuiMessage("", GuiMessage::ProgressBar, "whazzupDownload", prog, tot);
}

void Whazzup::whazzupDownloaded(bool error) {
    emit hasGuiMessage("", GuiMessage::Remove, "whazzupDownload");
    if(whazzupBuffer == 0) {
        emit hasGuiMessage("Download Error. Buffer unavailable.", GuiMessage::ErrorUserAttention);
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }

    if(whazzupBuffer->data().isEmpty()) {
        emit hasGuiMessage("No data in Whazzup.", GuiMessage::Warning);
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }

    if(error) {
        emit hasGuiMessage(whazzupDownloader->errorString(), GuiMessage::Warning);
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }
    emit hasGuiMessage("Processing Whazzup", GuiMessage::Persistent, "whazzupProcess");

    whazzupBuffer->open(QBuffer::ReadOnly); // maybe fixes some issues we encounter very rarely
    whazzupBuffer->seek(0);
    WhazzupData newWhazzupData(whazzupBuffer, WhazzupData::WHAZZUP);

    whazzupBuffer->close();
    if(!newWhazzupData.isNull()) {

        if(newWhazzupData.timestamp().secsTo(QDateTime::currentDateTime().toUTC()) > 60 * 30)
            emit hasGuiMessage("Whazzup data more than 30 minutes old.", GuiMessage::Warning);

        if(newWhazzupData.timestamp() != data.timestamp()) {
            data.updateFrom(newWhazzupData);
            qDebug() << "Whazzup updated from timestamp\t" << data.timestamp().toString();


            if(Settings::saveWhazzupData()){
                // write out Whazzup to a file
                QString filename = Settings::applicationDataDirectory(
                        QString("downloaded/%1_%2.whazzup")
                        .arg(Settings::downloadNetwork())
                        .arg(data.timestamp().toString("yyyyMMdd-HHmmss")));
                QFile out(filename);
                if (out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    out.write(whazzupBuffer->data());
                }
                else {
                    qDebug() << "Info: Could not write Whazzup to disk" << out.fileName();
                }
            }

            emit newData(true);
        }
        else {
            emit hasGuiMessage(QString("We already have Whazzup with that Timestamp: %1")
                               .arg(data.timestamp().toString("ddd MM/dd HHmm'z'")));
        }
    }

    if(Settings::downloadPeriodically()) {
        const int serverNextUpdateInSec = QDateTime::currentDateTimeUtc().secsTo(data.updateEarliest);
        if (data.updateEarliest.isValid() &&
            (Settings::downloadInterval() * 60 < serverNextUpdateInSec)) {
            downloadTimer->start(serverNextUpdateInSec * 1000 + 20000); // 20s after later than reported from server
                                                                        // to adjust for inconsistently set clocks
            qDebug() << "Whazzup::whazzupDownloaded() correcting next update to"
                    << serverNextUpdateInSec << "s to respect the server's minimum interval";
        } else
            downloadTimer->start(Settings::downloadInterval() * 60 * 1000);
    }

    emit hasGuiMessage("", GuiMessage::Remove, "whazzupProcess");
}

void Whazzup::downloadBookings() {
    QUrl url(Settings::bookingsLocation());

    bookingsTimer->stop();

    emit hasGuiMessage(QString("Updating ATC Bookings from %1").arg(url.toString(QUrl::RemoveUserInfo)), GuiMessage::ProgressBar, "bookingsDownload");

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
    emit hasGuiMessage("", GuiMessage::ProgressBar, "bookingsDownload", prog, tot);
}

void Whazzup::bookingsDownloaded(bool error) {
    emit hasGuiMessage("", GuiMessage::Remove, "bookingsDownload");
    if(bookingsBuffer == 0) {
        emit hasGuiMessage("Download Error. Buffer unavailable.", GuiMessage::ErrorUserAttention);
        return;
    }

    if(Settings::downloadBookings() && Settings::bookingsPeriodically()) {
        bookingsTimer->start(Settings::bookingsInterval() * 60 * 1000);
    }

    bookingsBuffer->open(QBuffer::ReadOnly); // maybe fixes some issues we encounter very rarely
    if(bookingsBuffer->data().isEmpty()) {
        emit hasGuiMessage("No data in Bookings.", GuiMessage::Warning);
        return;
    }

    if(error) {
        emit hasGuiMessage(bookingsDownloader->errorString(), GuiMessage::Warning);
        return;
    }
    emit hasGuiMessage("Processing Bookings", GuiMessage::Persistent, "bookingsProcess");

    WhazzupData newBookingsData(bookingsBuffer, WhazzupData::ATCBOOKINGS);
    bookingsBuffer->close();
    if(!newBookingsData.isNull()) {
        if(newBookingsData.bookingsTimestamp().secsTo(QDateTime::currentDateTime().toUTC()) > 60 * 60 * 3)
            emit hasGuiMessage("Bookings data more than 3 hours old.", GuiMessage::Warning);

        if(newBookingsData.bookingsTimestamp() != data.bookingsTimestamp()) {
            data.updateFrom(newBookingsData);
            qDebug() << "Bookings updated from timestamp\t" << data.bookingsTimestamp().toString();

            QFile out(Settings::applicationDataDirectory(
                    QString("downloaded/%1_%2.bookings")
                    .arg(Settings::downloadNetwork())
                    .arg(data.bookingsTimestamp().toString("yyMMdd-HHmmss"))));
            if (out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                out.write(bookingsBuffer->data());
            } else {
                qDebug() << "Info: Could not write Bookings to disk" << out.fileName();
            }

            emit newData(true);
        } else {
            emit hasGuiMessage(QString("We already have Bookings with that Timestamp: %1")
                               .arg(data.bookingsTimestamp().toString("ddd MM/dd HHmm'z'")));
        }
    }
    emit hasGuiMessage("", GuiMessage::Remove, "bookingsProcess");
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
    if (this->predictedTime != predictedTime) {
        emit hasGuiMessage("Calculating Warp...", GuiMessage::Persistent, "warpProcess");
        this->predictedTime = predictedTime;
        if (Settings::downloadBookings() && !data.bookingsTimestamp().isValid()) {
            emit needBookings();
        }

        WhazzupData newdata = WhazzupData(predictedTime, data);
        predictedData.updateFrom(newdata);
        emit hasGuiMessage("", GuiMessage::Remove, "warpProcess");
        emit newData(true);
    }
}

QList <QPair <QDateTime, QString> > Whazzup::getDownloadedWhazzups() {
    // Process directory
    QStringList list = QDir(Settings::applicationDataDirectory("downloaded/")).entryList(
            QStringList(QString("%1_*.whazzup").arg(Settings::downloadNetwork())),
            QDir::Files | QDir::Readable);
    list.sort();

    QList <QPair <QDateTime, QString> > returnList;
    for (int i = 0; i < list.size(); i++) {
        QRegExp dtRe = QRegExp("\\.*_(\\d{8}-\\d{6})");
        if (dtRe.indexIn(list[i]) > 0) {
            QDateTime dt = QDateTime::fromString(dtRe.cap(1), "yyyyMMdd-HHmmss");
            dt.setTimeSpec(Qt::UTC);
            returnList.append(QPair<QDateTime, QString>(
                    dt,
                    Settings::applicationDataDirectory(QString("downloaded/%1").arg(list[i]))
            ));
        }
    }
    return returnList;
}
