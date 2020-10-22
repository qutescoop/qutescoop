/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Whazzup.h"

#include "_pch.h"

#include "Settings.h"
#include "Window.h"

Whazzup *whazzupInstance = 0;

Whazzup* Whazzup::instance() {
    if(whazzupInstance == 0)
        whazzupInstance = new Whazzup();
    return whazzupInstance;
}

Whazzup::Whazzup():
        _replyStatus(0), _replyWhazzup(0), _replyBookings(0) {
    // init random seed to switch between URLs
    qsrand(QDateTime::currentDateTime().toTime_t());

    _downloadTimer = new QTimer(this);
    _bookingsTimer = new QTimer(this);
    connect(_downloadTimer, SIGNAL(timeout()), SLOT(download()));
    connect(_bookingsTimer, SIGNAL(timeout()), SLOT(downloadBookings()));

    connect(this, SIGNAL(needBookings()), SLOT(downloadBookings()));
}

Whazzup::~Whazzup() {
    if(_downloadTimer != 0) delete _downloadTimer;
    if(_bookingsTimer != 0) delete _bookingsTimer;
    if(_replyStatus != 0) delete _replyStatus;
    if(_replyWhazzup != 0) delete _replyWhazzup;
    if(_replyBookings != 0) delete _replyBookings;
}

void Whazzup::setStatusLocation(const QString& statusLocation) {
    qDebug() << "Downloading network status from\t" << statusLocation;
    GuiMessages::progress("statusdownload", "Getting network status...");

    _replyStatus = Net::g(statusLocation);
    connect(_replyStatus, SIGNAL(finished()), SLOT(processStatus()));
}

void Whazzup::processStatus() {
    qDebug() << "Whazzup::processStatus()";
    disconnect(_replyStatus, SIGNAL(finished()), this, SLOT(processStatus()));
    _replyStatus->deleteLater();

    // status.vatsim.net uses redirection
    QUrl urlRedirect = _replyStatus->attribute(
                QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if(!urlRedirect.isEmpty() && urlRedirect != _replyStatus->url()) {
        qDebug() << "Whazzup::processStatus() redirected to" << urlRedirect;
        // send new request
        _replyStatus = Net::g(urlRedirect);
        connect(_replyStatus, SIGNAL(finished()), SLOT(processStatus()));
        return;
    }

    if(_replyStatus->error() != QNetworkReply::NoError)
        GuiMessages::warning(_replyStatus->errorString());

    if(_replyStatus->bytesAvailable() == 0)
        GuiMessages::warning("Statusfile is empty");

    _urls.clear();
    _gzurls.clear();
    _metarUrl = "";
    _tafUrl = "";
    _shorttafUrl = "";
    _atisLink = "";
    _userLink = "";

    while(_replyStatus->canReadLine()) {
        QString line = QString(_replyStatus->readLine()).trimmed();
        if(line.startsWith(";")) // ignore comments
            continue;

        QStringList list = line.split('=');

        if("msg0" == list[0]) {
            _message = line.right(line.length() - QString("msg0=").length());
            continue;
        }

        if(list.size() != 2) continue;

        if("url0" == list[0])
            _urls.append(list[1]);
        else if("gzurl0" == list[0])
            _gzurls.append(list[1]);
        else if("moveto0" == list[0]) // this URL is obsolete. Try that one instead.
            // do something with it?
            qDebug() << "status.txt suggested to use" << list[1]
                     << "!!! We do not handle that automatically,"
                     << "please update the status-URL by hand.";
        else if("metar0" == list[0])
            _metarUrl = list[1];
        else if("taf0" == list[0])
            _tafUrl = list[1];
        else if("shorttaf0" == list[0])
            _shorttafUrl = list[1];
        else if("user0" == list[0])
            _userLink = list[1];
        else if("atis0" == list[0])
            _atisLink = list[1];
    }

    if(!_message.isEmpty())
        GuiMessages::warning(_message);

    _lastDownloadTime = QTime();

    GuiMessages::remove("statusdownload");
    qDebug() << "Whazzup::statusDownloaded() Got" << _urls.size() << "Whazzup URLs";

    if(_urls.size() == 0)
        GuiMessages::warning("No Whazzup-URLs found. Try again later.");
    else
        download();
}

void Whazzup::fromFile(QString filename) {
    qDebug() << "Whazzup::fromFile()" << filename;
    GuiMessages::progress("whazzupDownload", "Loading Whazzup from file...");

    _replyWhazzup = Net::g(QUrl::fromLocalFile(filename));
    connect(_replyWhazzup, SIGNAL(finished()), SLOT(processWhazzup()));
}

void Whazzup::download() {
    if(_urls.size() == 0) {
        setStatusLocation(Settings::statusLocation());
        return;
    }

    _downloadTimer->stop();
    QTime now = QTime::currentTime();

    if(!_lastDownloadTime.isNull() && _lastDownloadTime.secsTo(now) < 30) {
        GuiMessages::message(QString("Whazzup checked %1s (less than 30s) ago. Download scheduled.")
                            .arg(_lastDownloadTime.secsTo(now)));
        _downloadTimer->start((30 - _lastDownloadTime.secsTo(now)) * 1000);
        return; // don't allow download intervals < 30s
    }
    _lastDownloadTime = now;

    QUrl url(_urls[qrand() % _urls.size()]);

    GuiMessages::progress("whazzupDownload", QString("Updating whazzup from %1...").
                         arg(url.toString(QUrl::RemoveUserInfo)));

    _replyWhazzup = Net::g(url);
    connect(_replyWhazzup, SIGNAL(finished()), SLOT(processWhazzup()));
    connect(_replyWhazzup, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(whazzupProgress(qint64,qint64)));
}

void Whazzup::whazzupProgress(qint64 prog, qint64 tot) {
    GuiMessages::progress("whazzupDownload", prog, tot);
}

void Whazzup::processWhazzup() {
    GuiMessages::remove("whazzupDownload");
    emit whazzupDownloaded();
    disconnect(_replyWhazzup, SIGNAL(finished()), this, SLOT(processWhazzup()));
    disconnect(_replyWhazzup, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(whazzupProgress(qint64,qint64)));
    _replyWhazzup->deleteLater();
    if(_replyWhazzup == 0) {
        GuiMessages::criticalUserInteraction("Buffer unavailable.",
                                             "Whazzup download Error");
        _downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }
    if(_replyWhazzup->bytesAvailable() == 0) {
        GuiMessages::warning("No data in Whazzup.");
        _downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }
    if(_replyWhazzup->error() != QNetworkReply::NoError) {
        GuiMessages::warning(_replyWhazzup->errorString());
        _downloadTimer->start(30 * 1000); // try again in 30s
        return;
    }
    GuiMessages::progress("whazzupProcess", "Processing Whazzup...");

    QByteArray *bytes = new QByteArray(_replyWhazzup->readAll());
    WhazzupData newWhazzupData(bytes, WhazzupData::WHAZZUP);

    if(!newWhazzupData.isNull()) {
        if(!predictedTime.isValid() &&
                newWhazzupData.whazzupTime.secsTo(QDateTime::currentDateTimeUtc()) > 60 * 30)
            GuiMessages::warning("Whazzup data more than 30 minutes old.");

        if(newWhazzupData.whazzupTime != _data.whazzupTime) {
            _data.updateFrom(newWhazzupData);
            qDebug() << "Whazzup::whazzupDownloaded() Whazzup updated from timestamp" << _data.whazzupTime;
            emit newData(true);

            if(Settings::saveWhazzupData()) {
                // write out Whazzup to a file
                QFile out(Settings::applicationDataDirectory(
                          QString("downloaded/%1_%2.whazzup")
                          .arg(Settings::downloadNetwork())
                          .arg(_data.whazzupTime.toString("yyyyMMdd-HHmmss"))));
                if (!out.exists() && out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    qDebug() << "Whazzup::processWhazzup writing Whazzup to" << out.fileName();
                    out.write(bytes->constData());
                    out.close();
                } else
                    qWarning() << "Whazzup::processWhazzup: Could not write Whazzup to disk" << out.fileName();
            }
        } else
            GuiMessages::message(QString("We already have Whazzup with that Timestamp: %1")
                                .arg(_data.whazzupTime.toString("ddd MM/dd HHmm'z'")));
    }

    if(Settings::downloadPeriodically()) {
        const int serverNextUpdateInSec = QDateTime::currentDateTimeUtc().secsTo(_data.updateEarliest);
        if (_data.updateEarliest.isValid() &&
            (Settings::downloadInterval() * 60 < serverNextUpdateInSec)) {
            _downloadTimer->start(serverNextUpdateInSec * 1000 + 60000); // 1min after later than reported from server
                                                                        // - seems to report 0000z when actually updates at 0000:59z
            qDebug() << "Whazzup::whazzupDownloaded() correcting next update, will update in"
                    << serverNextUpdateInSec << "s to respect the server's minimum interval";
        } else
            _downloadTimer->start(Settings::downloadInterval() * 60 * 1000);
    }

    GuiMessages::remove("whazzupProcess");
}

void Whazzup::downloadBookings() {
    if (!Settings::downloadBookings())
        return;
    QUrl url(Settings::bookingsLocation());

    _bookingsTimer->stop();

    GuiMessages::progress("bookingsDownload",
                              QString("Updating ATC Bookings from %1...")
                          .arg(url.toString(QUrl::RemoveUserInfo)));

    _replyBookings = Net::g(url);
    connect(_replyBookings, SIGNAL(finished()), SLOT(processBookings()));
    connect(_replyBookings, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(bookingsProgress(qint64,qint64)));
}

void Whazzup::bookingsProgress(qint64 prog, qint64 tot) {
    GuiMessages::progress("bookingsDownload", prog, tot);
}

void Whazzup::processBookings() {
    qDebug() << "Whazzup::processBookings()";
    GuiMessages::remove("bookingsDownload");
    disconnect(_replyBookings, SIGNAL(finished()), this, SLOT(processBookings()));
    disconnect(_replyBookings, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(bookingsProgress(qint64,qint64)));
    qDebug() << "Whazzup::processBookings() deleting buffer";
    _replyBookings->deleteLater();
    if(_replyBookings == 0) {
        GuiMessages::criticalUserInteraction("Buffer unavailable.",
                                             "Booking download Error");
        return;
    }
    if(_replyBookings->bytesAvailable() == 0) {
        GuiMessages::warning("No data in Bookings.");
        return;
    }
    if(_replyBookings->error() != QNetworkReply::NoError) {
        GuiMessages::warning(_replyBookings->errorString());
        return;
    }

    if(Settings::downloadBookings() && Settings::bookingsPeriodically())
        _bookingsTimer->start(Settings::bookingsInterval() * 60 * 1000);

    GuiMessages::progress("bookingsProcess", "Processing Bookings...");

    QByteArray *bytes = new QByteArray(_replyBookings->readAll());
    WhazzupData newBookingsData(bytes, WhazzupData::ATCBOOKINGS);
    if (!newBookingsData.isNull()) {
        qDebug() << "Whazzup::processBookings() step 2";
        if (newBookingsData.bookingsTime.secsTo(QDateTime::currentDateTimeUtc()) > 60 * 60 * 3)
            GuiMessages::warning("Bookings data more than 3 hours old.");

        if (newBookingsData.bookingsTime != _data.bookingsTime) {
            qDebug() << "Whazzup::processBookings() will call updateFrom()";
            _data.updateFrom(newBookingsData);
            qDebug() << "Whazzup::processBookings() Bookings updated from timestamp"
                     << _data.bookingsTime;

            QFile out(Settings::applicationDataDirectory(
                    QString("downloaded/%1_%2.bookings")
                    .arg(Settings::downloadNetwork())
                    .arg(_data.bookingsTime.toString("yyyyMMdd-HHmmss"))));
            if (!out.exists() && out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qDebug() << "Whazzup::processBookings writing bookings to" << out.fileName();
                out.write(bytes->constData());
                out.close();
            } else
                qWarning() << "Whazzup::processBookings: Could not write Bookings to disk"
                           << out.fileName();

            // from now on we want to redownload when the user triggers a network update
            connect(Window::instance()->actionDownload, SIGNAL(triggered()),
                    this, SLOT(downloadBookings()));
            emit newData(true);
        } else
            GuiMessages::message(QString("We already have bookings with that timestamp: %1")
                               .arg(_data.bookingsTime.toString("ddd MM/dd HHmm'z'")));
    }
    GuiMessages::remove("bookingsProcess");
    qDebug() << "Whazzup::processBookings() -- finished";
}


QString Whazzup::userLink(const QString& id) const {
    if(_userLink.isEmpty())
        return QString();
    return _userLink + "?id=" + id;
}

QString Whazzup::atisLink(const QString& id) const {
    if(_metarUrl.isEmpty())
        return QString();
    return _metarUrl + "?id=" + id;
}

void Whazzup::setPredictedTime(QDateTime predictedTime) {
    if(this->predictedTime != predictedTime) {
        qDebug() << "Whazzup::setPredictedTime() predictedTime=" << predictedTime
                 << "data.whazzupTime=" << _data.whazzupTime;
        GuiMessages::progress("warpProcess", "Calculating Warp...");
        this->predictedTime = predictedTime;
        if (Settings::downloadBookings() && !_data.bookingsTime.isValid())
            emit needBookings();
        if(predictedTime == _data.whazzupTime) {
            qDebug() << "Whazzup::setPredictedTime() predictedTime == data.whazzupTime"
                     << "(no need to predict, we have it already :) )";
            _predictedData = _data;
        } else
            _predictedData.updateFrom(WhazzupData(predictedTime, _data));
        GuiMessages::remove("warpProcess");
        emit newData(true);
    }
}

QList <QPair <QDateTime, QString> > Whazzup::downloadedWhazzups() const {
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
