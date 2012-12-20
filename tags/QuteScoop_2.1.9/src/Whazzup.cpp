/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Whazzup.h"

#include "_pch.h"

#include "Settings.h"
#include "Window.h"

Whazzup *whazzupInstance = 0;

Whazzup* Whazzup::getInstance() {
    if(whazzupInstance == 0)
        whazzupInstance = new Whazzup();
    return whazzupInstance;
}

Whazzup::Whazzup() {
    srand(QDateTime::currentDateTime().toTime_t()); // init random seed to switch between URLs

    downloadTimer = new QTimer(this);
    bookingsTimer = new QTimer(this);
    connect(downloadTimer, SIGNAL(timeout()), this, SLOT(download()));
    connect(bookingsTimer, SIGNAL(timeout()), this, SLOT(downloadBookings()));

    connect(this, SIGNAL(needBookings()), this, SLOT(downloadBookings()));
}

Whazzup::~Whazzup() {
    if(downloadTimer != 0) delete downloadTimer;
    if(bookingsTimer != 0) delete bookingsTimer;
}

void Whazzup::setStatusLocation(const QString& statusLocation) {
    qDebug() << "Downloading network status from\t" << statusLocation;
    GuiMessages::message("Getting network status...");

    QUrl url(statusLocation);

    connect(NetworkManager::getInstance(),
            SIGNAL(requestFinished(QNetworkReply*)),
            this, SLOT(processStatus(QNetworkReply*)));
    NetworkManager::getInstance()->httpRequest(QNetworkRequest(url));
}

void Whazzup::processStatus(QNetworkReply* reply) {
    disconnect(NetworkManager::getInstance(),
               SIGNAL(requestFinished(QNetworkReply*)),
               this, SLOT(processStatus(QNetworkReply*)));

    if(reply == 0)
        return;

    if(reply->error() != QNetworkReply::NoError) {
        GuiMessages::warning(reply->errorString());
        return;
    }

    if(reply->bytesAvailable() == 0) {
        GuiMessages::warning("Statusfile is empty");
    }

    urls.clear();
    gzurls.clear();
    metarUrl = "";
    tafUrl = "";
    shorttafUrl = "";
    atisLink = "";
    userLink = "";


    reply->seek(0);
    while(reply->canReadLine()) {
        QString line = QString(reply->readLine()).trimmed();
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
        else if("moveto0" == list[0]) // this URL is obsolete. Try that one instead.
            // do something with it?
            qDebug() << "status.txt suggested to use" << list[0] <<
                    "!!! We do not handle that automatically, please update the status-URL by hand.";
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
        GuiMessages::warning(message);

    lastDownloadTime = QTime();

    qDebug() << "Whazzup::statusDownloaded() Got" << urls.size() << "Whazzup URLs";
    reply->deleteLater();

    if(urls.size() == 0)
        GuiMessages::warning("No Whazzup-URLs found. Try again later.");
    else
        download();
}

void Whazzup::fromFile(QString filename) {
    qDebug() << "Whazzup::fromFile()" << filename;
    QUrl url = QUrl::fromLocalFile(filename);

    GuiMessages::progress("whazzupDownload", "Loading Whazzup from file...");

    connect(NetworkManager::getInstance(),
            SIGNAL(requestFinished(QNetworkReply*)),
            this, SLOT(processWhazzup(QNetworkReply*)));
    NetworkManager::getInstance()->httpRequest(QNetworkRequest(url));
}

void Whazzup::download() {
    if(urls.size() == 0) {
        setStatusLocation(Settings::statusLocation());
        return;
    }

    downloadTimer->stop();
    QTime now = QTime::currentTime();

    if(lastDownloadTime.secsTo(now) < 30) {
        GuiMessages::message(QString("Whazzup checked %1s (less than 30s) ago. Download scheduled.")
                            .arg(lastDownloadTime.secsTo(now)));
        downloadTimer->start((30 - lastDownloadTime.secsTo(now)) * 1000);
        return; // don't allow download intervals < 30s
    }
    lastDownloadTime = now;

    QUrl url(urls[qrand() % urls.size()]);

    GuiMessages::progress("whazzupDownload", QString("Updating whazzup from %1...").
                         arg(url.toString(QUrl::RemoveUserInfo)));

    connect(NetworkManager::getInstance(),
            SIGNAL(requestFinished(QNetworkReply*)),
            this, SLOT(processWhazzup(QNetworkReply*)));
    QNetworkReply *reply = NetworkManager::getInstance()->httpRequest(QNetworkRequest(url));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(whazzupProgress(qint64,qint64)));
}

void Whazzup::whazzupProgress(qint64 prog, qint64 tot) {
    GuiMessages::progress("whazzupDownload", prog, tot);
}

void Whazzup::processWhazzup(QNetworkReply* reply) {
    GuiMessages::remove("whazzupDownload");
    disconnect(NetworkManager::getInstance(),
               SIGNAL(requestFinished(QNetworkReply*)),
               this, SLOT(processWhazzup(QNetworkReply*)));

    if(reply == 0) {
        GuiMessages::errorUserAttention("Download Error. Buffer unavailable.");
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }

    if(reply->bytesAvailable() == 0) {
        GuiMessages::warning("No data in Whazzup.");
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }

    if(reply->error() != QNetworkReply::NoError) {
        GuiMessages::warning(reply->errorString());
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }
    GuiMessages::status("Processing Whazzup", "whazzupProcess");

    reply->seek(0);
    WhazzupData newWhazzupData(reply, WhazzupData::WHAZZUP);

    reply->close();
    if(!newWhazzupData.isNull()) {
        if(!predictedTime.isValid() &&
                newWhazzupData.whazzupTime.secsTo(QDateTime::currentDateTimeUtc()) > 60 * 30)
            GuiMessages::warning("Whazzup data more than 30 minutes old.");

        if(newWhazzupData.whazzupTime != data.whazzupTime) {
            data.updateFrom(newWhazzupData);
            qDebug() << "Whazzup::whazzupDownloaded() Whazzup updated from timestamp" << data.whazzupTime;
            emit newData(true);

            if(Settings::saveWhazzupData()){
                // write out Whazzup to a file
                QString filename = Settings::applicationDataDirectory(
                        QString("downloaded/%1_%2.whazzup")
                        .arg(Settings::downloadNetwork())
                        .arg(data.whazzupTime.toString("yyyyMMdd-HHmmss")));
                QFile out(filename);
                if (out.open(QIODevice::WriteOnly | QIODevice::Text))
                    out.write(reply->readAll());
                else
                    qWarning() << "Info: Could not write Whazzup to disk" << out.fileName();
            }
        } else
            GuiMessages::message(QString("We already have Whazzup with that Timestamp: %1")
                                .arg(data.whazzupTime.toString("ddd MM/dd HHmm'z'")));
    }

    if(Settings::downloadPeriodically()) {
        const int serverNextUpdateInSec = QDateTime::currentDateTimeUtc().secsTo(data.updateEarliest);
        if (data.updateEarliest.isValid() &&
            (Settings::downloadInterval() * 60 < serverNextUpdateInSec)) {
            downloadTimer->start(serverNextUpdateInSec * 1000 + 60000); // 1min after later than reported from server
                                                                        // - seems to report 0000z when actually updates at 0000:59z
            qDebug() << "Whazzup::whazzupDownloaded() correcting next update, will update in"
                    << serverNextUpdateInSec << "s to respect the server's minimum interval";
        } else
            downloadTimer->start(Settings::downloadInterval() * 60 * 1000);
    }

    GuiMessages::remove("whazzupProcess");
    reply->deleteLater();
}

void Whazzup::downloadBookings() {
    QUrl url(Settings::bookingsLocation());

    bookingsTimer->stop();

    GuiMessages::progress("bookingsDownload",
                              QString("Updating ATC Bookings from %1...")
                          .arg(url.toString(QUrl::RemoveUserInfo)));
    connect(NetworkManager::getInstance(),
            SIGNAL(requestFinished(QNetworkReply*)),
            this, SLOT(processBookings(QNetworkReply*)));
    QNetworkReply *reply = NetworkManager::getInstance()->httpRequest(QNetworkRequest(url));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this,
            SLOT(bookingsProgress(qint64,qint64)));
}

void Whazzup::bookingsProgress(qint64 prog, qint64 tot) {
    GuiMessages::progress("bookingsDownload", prog, tot);
}

void Whazzup::processBookings(QNetworkReply* reply) {
    GuiMessages::remove("bookingsDownload");
    disconnect(NetworkManager::getInstance(),
               SIGNAL(requestFinished(QNetworkReply*)),
               this, SLOT(processWhazzup(QNetworkReply*)));
    if(reply == 0) {
        GuiMessages::errorUserAttention("Download Error. Buffer unavailable.");
        return;
    }
    if(reply->bytesAvailable() == 0) {
        GuiMessages::warning("No data in Bookings.");
        return;
    }
    if(reply->error() != QNetworkReply::NoError) {
        GuiMessages::warning(reply->errorString());
        return;
    }

    if(Settings::downloadBookings() && Settings::bookingsPeriodically())
        bookingsTimer->start(Settings::bookingsInterval() * 60 * 1000);

    reply->open(QBuffer::ReadOnly); // maybe fixes some issues we encounter very rarely
    GuiMessages::status("Processing Bookings", "bookingsProcess");
    WhazzupData newBookingsData(reply, WhazzupData::ATCBOOKINGS);
    reply->close();
    if(!newBookingsData.isNull()) {
        if(newBookingsData.bookingsTime.secsTo(QDateTime::currentDateTimeUtc()) > 60 * 60 * 3)
            GuiMessages::warning("Bookings data more than 3 hours old.");

        if(newBookingsData.bookingsTime != data.bookingsTime) {
            data.updateFrom(newBookingsData);
            qDebug() << "Whazzup::bookingsDownloaded() Bookings updated from timestamp" << data.bookingsTime;

            QFile out(Settings::applicationDataDirectory(
                    QString("downloaded/%1_%2.bookings")
                    .arg(Settings::downloadNetwork())
                    .arg(data.bookingsTime.toString("yyMMdd-HHmmss"))));
            if (out.open(QIODevice::WriteOnly | QIODevice::Text))
                out.write(reply->readAll());
            else
                qDebug() << "Info: Could not write Bookings to disk" << out.fileName();

            emit newData(true);
        } else
            GuiMessages::message(QString("We already have bookings with that timestamp: %1")
                               .arg(data.bookingsTime.toString("ddd MM/dd HHmm'z'")));
    }
    GuiMessages::remove("bookingsProcess");
    reply->deleteLater();
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
    if(this->predictedTime != predictedTime) {
        qDebug() << "Whazzup::setPredictedTime() predictedTime=" << predictedTime
                 << "data.whazzupTime=" << data.whazzupTime;
        GuiMessages::status("Calculating Warp...", "warpProcess");
        this->predictedTime = predictedTime;
        if (Settings::downloadBookings() && !data.bookingsTime.isValid())
            emit needBookings();
        if(predictedTime == data.whazzupTime) {
            qDebug() << "Whazzup::setPredictedTime() predictedTime == data.whazzupTime"
                     << "(no need to predict, we have it already :) )";
            predictedData = data;
        } else
            predictedData.updateFrom(WhazzupData(predictedTime, data));
        GuiMessages::remove("warpProcess");
        emit newData(true);
    }
}

QList <QPair <QDateTime, QString> > Whazzup::getDownloadedWhazzups() const {
    // Process directory
    QStringList list = QDir(Settings::applicationDataDirectory("downloaded/")).entryList(
            QStringList(QString("%1_*.whazzup").arg(Settings::downloadNetwork())),
            QDir::Files | QDir::Readable); // getting all *.whazzup
    list.sort();

    QList <QPair <QDateTime, QString> > returnList;
    foreach(const QString filename, list) {
        QRegExp dtRe = QRegExp("\\.*_(\\d{8}-\\d{6})"); // dateTime: 20110301-191050
        if (dtRe.indexIn(filename) > 0) {
            QDateTime dt = QDateTime::fromString(dtRe.cap(1), "yyyyMMdd-HHmmss");
            dt.setTimeSpec(Qt::UTC);
            returnList.append(QPair<QDateTime, QString>(
                    dt,
                    Settings::applicationDataDirectory(QString("downloaded/%1").
                                                       arg(filename))
            ));
        }
    }
    return returnList;
}
