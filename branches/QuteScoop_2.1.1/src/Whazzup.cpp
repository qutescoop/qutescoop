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
    statusDownloader = new QHttp;
    whazzupDownloader = new QHttp;
    bookingsDownloader = new QHttp;

    statusBuffer = 0;
    whazzupBuffer = 0;
    bookingsBuffer = 0;
    connect(statusDownloader, SIGNAL(done(bool)), this, SLOT(statusDownloaded(bool)));

    srand(QDateTime::currentDateTime().toTime_t()); // init random seed to switch between URLs

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
        GuiMessages::warning(statusDownloader->errorString());
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

    delete statusBuffer;
    statusBuffer = 0;
    lastDownloadTime = QTime();

    qDebug() << "Whazzup::statusDownloaded() Got" << urls.size() << "Whazzup URLs";

    if(urls.size() == 0)
        GuiMessages::warning("No Whazzup-URLs found. Try again later.");
    else
        emit statusDownloaded();
}

void Whazzup::fromFile(QString filename) {
    qDebug() << "Whazzup::fromFile()" << filename;
    QFile *file = new QFile(filename);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        GuiMessages::errorUserAttention(QString("Error opening %1").arg(filename));
        return;
    }
    GuiMessages::progress("whazzupDownload", "Loading Whazzup from file...");

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
        GuiMessages::errorUserAttention("No Whazzup URLs in network status. Trying to get a new network status.");
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

    GuiMessages::progress("whazzupDownload", QString("Updating whazzup from %1").
                         arg(url.toString(QUrl::RemoveUserInfo)));

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
    GuiMessages::progress("whazzupDownload", prog, tot);
}

void Whazzup::whazzupDownloaded(bool error) {
    GuiMessages::remove("whazzupDownload");
    if(whazzupBuffer == 0) {
        GuiMessages::errorUserAttention("Download Error. Buffer unavailable.");
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }

    if(whazzupBuffer->data().isEmpty()) {
        GuiMessages::warning("No data in Whazzup.");
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }

    if(error) {
        GuiMessages::warning(whazzupDownloader->errorString());
        downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }
    GuiMessages::status("Processing Whazzup", "whazzupProcess");

    whazzupBuffer->open(QBuffer::ReadOnly); // maybe fixes some issues we encounter very rarely
    whazzupBuffer->seek(0);
    WhazzupData newWhazzupData(whazzupBuffer, WhazzupData::WHAZZUP);

    whazzupBuffer->close();
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
                    out.write(whazzupBuffer->data());
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
}

void Whazzup::downloadBookings() {
    QUrl url(Settings::bookingsLocation());

    bookingsTimer->stop();

    GuiMessages::progress("bookingsDownload",
                              QString("Updating ATC Bookings from %1").arg(url.toString(QUrl::RemoveUserInfo)));

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
    GuiMessages::progress("bookingsDownload", prog, tot);
}

void Whazzup::bookingsDownloaded(bool error) {
    GuiMessages::remove("bookingsDownload");
    if(bookingsBuffer == 0) {
        GuiMessages::errorUserAttention("Download Error. Buffer unavailable.");
        return;
    }

    if(Settings::downloadBookings() && Settings::bookingsPeriodically())
        bookingsTimer->start(Settings::bookingsInterval() * 60 * 1000);

    bookingsBuffer->open(QBuffer::ReadOnly); // maybe fixes some issues we encounter very rarely
    if(bookingsBuffer->data().isEmpty()) {
        GuiMessages::warning("No data in Bookings.");
        return;
    }

    if(error) {
        GuiMessages::warning(bookingsDownloader->errorString());
        return;
    }
    GuiMessages::status("Processing Bookings", "bookingsProcess");

    WhazzupData newBookingsData(bookingsBuffer, WhazzupData::ATCBOOKINGS);
    bookingsBuffer->close();
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
                out.write(bookingsBuffer->data());
            else
                qDebug() << "Info: Could not write Bookings to disk" << out.fileName();

            emit newData(true);
        } else
            GuiMessages::message(QString("We already have bookings with that timestamp: %1")
                               .arg(data.bookingsTime.toString("ddd MM/dd HHmm'z'")));
    }
    GuiMessages::remove("bookingsProcess");
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
