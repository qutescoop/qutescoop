#include "Whazzup.h"

#include "Client.h"
#include "GuiMessage.h"
#include "Net.h"
#include "Settings.h"
#include "dialogs/Window.h"

Whazzup* whazzupInstance = 0;

Whazzup* Whazzup::instance() {
    if(whazzupInstance == 0) {
        whazzupInstance = new Whazzup();
    }
    return whazzupInstance;
}

Whazzup::Whazzup() :
    _replyStatus(0), _replyWhazzup(0), _replyBookings(0) {
    _downloadTimer = new QTimer(this);
    _bookingsTimer = new QTimer(this);
    connect(_downloadTimer, &QTimer::timeout, this, &Whazzup::downloadJson3);
    connect(_bookingsTimer, &QTimer::timeout, this, &Whazzup::downloadBookings);

    connect(this, &Whazzup::needBookings, this, &Whazzup::downloadBookings);
}

Whazzup::~Whazzup() {
    if(_downloadTimer != 0) {
        delete _downloadTimer;
    }
    if(_bookingsTimer != 0) {
        delete _bookingsTimer;
    }
    if(_replyStatus != 0) {
        delete _replyStatus;
    }
    if(_replyWhazzup != 0) {
        delete _replyWhazzup;
    }
    if(_replyBookings != 0) {
        delete _replyBookings;
    }
}

void Whazzup::setStatusLocation(const QString& statusLocation) {
    qDebug() << "Downloading network status from\t" << statusLocation;
    GuiMessages::progress("statusdownload", "Getting network status...");

    _replyStatus = Net::g(statusLocation);
    connect(_replyStatus, &QNetworkReply::finished, this, &Whazzup::processStatus);
}

void Whazzup::processStatus() {
    qDebug() << "Whazzup::processStatus()";
    disconnect(_replyStatus, &QNetworkReply::finished, this, &Whazzup::processStatus);
    _replyStatus->deleteLater();

    // status.vatsim.net uses redirection
    QUrl urlRedirect = _replyStatus->attribute(
        QNetworkRequest::RedirectionTargetAttribute
    ).toUrl();
    if(!urlRedirect.isEmpty() && urlRedirect != _replyStatus->url()) {
        qDebug() << "Whazzup::processStatus() redirected to" << urlRedirect;
        // send new request
        _replyStatus = Net::g(urlRedirect);
        connect(_replyStatus, &QNetworkReply::finished, this, &Whazzup::processStatus);
        return;
    }

    if(_replyStatus->error() != QNetworkReply::NoError) {
        GuiMessages::warning(_replyStatus->errorString());
    }

    if(_replyStatus->bytesAvailable() == 0) {
        GuiMessages::warning("Statusfile is empty");
    }

    _json3Urls.clear();
    _metar0Url = "";
    _user0Url = "";

    QJsonDocument data = QJsonDocument::fromJson(_replyStatus->readAll());

    if(data.isNull()) {
        GuiMessages::warning("Couldn't parse status. Does the status URL return the old format?");
    } else {
        QJsonObject json = data.object();
        if(json.contains("data") && json["data"].isObject()) {
            QJsonObject vatsimDataSources = json["data"].toObject();
            if(vatsimDataSources.contains("v3") && vatsimDataSources["v3"].isArray()) {
                QJsonArray v3Urls = vatsimDataSources["v3"].toArray();
                for(int i = 0; i < v3Urls.size(); ++i) {
                    if(v3Urls[i].isString()) {
                        _json3Urls.append(v3Urls[i].toString());
                    }
                }
            }
        }

        if(json.contains("user") && json["user"].isArray()) {
            QJsonArray userUrls = json["user"].toArray();
            if(userUrls.first() != QJsonValue::Undefined) {
                _user0Url = userUrls.first().toString();
            }
        }

        if(json.contains("metar") && json["metar"].isArray()) {
            QJsonArray metarUrls = json["metar"].toArray();
            if(metarUrls.first() != QJsonValue::Undefined) {
                _metar0Url = metarUrls.first().toString();
            }
        }
    }

    _lastDownloadTime = QTime();

    GuiMessages::remove("statusdownload");
    qDebug() << "Whazzup::statusDownloaded() data.v3[]:" << _json3Urls;
    qDebug() << "Whazzup::statusDownloaded() metar.0:" << _metar0Url;
    qDebug() << "Whazzup::statusDownloaded() user.0:" << _user0Url;

    if(_json3Urls.size() == 0) {
        GuiMessages::warning("No Whazzup-URLs found. Try again later.");
    } else {
        downloadJson3();
    }
}

void Whazzup::fromFile(QString filename) {
    qDebug() << "Whazzup::fromFile()" << filename;
    GuiMessages::progress("whazzupDownload", "Loading Whazzup from file...");

    _replyWhazzup = Net::g(QUrl::fromLocalFile(filename));
    connect(_replyWhazzup, &QNetworkReply::finished, this, &Whazzup::processWhazzup);
}

void Whazzup::downloadJson3() {
    if(_json3Urls.size() == 0) {
        setStatusLocation(Settings::statusLocation());
        return;
    }

    _downloadTimer->stop();
    QTime now = QTime::currentTime();

    if(!_lastDownloadTime.isNull() && _lastDownloadTime.secsTo(now) < 30) {
        GuiMessages::message(
            QString("Whazzup checked %1s (less than 30s) ago. Download scheduled.")
            .arg(_lastDownloadTime.secsTo(now))
        );
        _downloadTimer->start((30 - _lastDownloadTime.secsTo(now)) * 1000);
        return; // don't allow download intervals < 30s
    }
    _lastDownloadTime = now;

    QUrl url(_json3Urls[QRandomGenerator::global()->bounded(quint32(_json3Urls.size() - 1))]);

    GuiMessages::progress(
        "whazzupDownload", QString("Updating whazzup from %1...").
        arg(url.toString(QUrl::RemoveUserInfo))
    );

    _replyWhazzup = Net::g(url);
    connect(_replyWhazzup, &QNetworkReply::finished, this, &Whazzup::processWhazzup);
    connect(_replyWhazzup, &QNetworkReply::downloadProgress, this, &Whazzup::whazzupProgress);
}

void Whazzup::whazzupProgress(qint64 prog, qint64 tot) {
    GuiMessages::progress("whazzupDownload", prog, tot);
}

void Whazzup::processWhazzup() {
    GuiMessages::remove("whazzupDownload");
    emit whazzupDownloaded();
    disconnect(_replyWhazzup, &QNetworkReply::finished, this, &Whazzup::processWhazzup);
    disconnect(_replyWhazzup, &QNetworkReply::downloadProgress, this, &Whazzup::whazzupProgress);
    _replyWhazzup->deleteLater();
    if(_replyWhazzup == 0) {
        GuiMessages::criticalUserInteraction(
            "Buffer unavailable.",
            "Whazzup download Error"
        );
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

    QByteArray* bytes = new QByteArray(_replyWhazzup->readAll());
    WhazzupData newWhazzupData(bytes, WhazzupData::WHAZZUP);

    if(!newWhazzupData.isNull()) {
        if(
            !predictedTime.isValid()
            && newWhazzupData.whazzupTime.secsTo(QDateTime::currentDateTimeUtc()) > 60 * 30
        )
        {
            GuiMessages::warning("Whazzup data more than 30 minutes old.");
        }

        if(newWhazzupData.whazzupTime != _data.whazzupTime) {
            _data.updateFrom(newWhazzupData);
            qDebug() << "Whazzup::whazzupDownloaded() Whazzup updated from timestamp" << _data.whazzupTime;
            emit newData(true);

            if(Settings::saveWhazzupData()) {
                // write out Whazzup to a file
                QFile out(Settings::dataDirectory(
                        QString("downloaded/%1_%2.whazzup")
                        .arg(Settings::downloadNetwork())
                        .arg(_data.whazzupTime.toString("yyyyMMdd-HHmmss"))
                ));
                if(!out.exists() && out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    qDebug() << "Whazzup::processWhazzup writing Whazzup to" << out.fileName();
                    out.write(bytes->constData());
                    out.close();
                } else {
                    qWarning() << "Whazzup::processWhazzup: Could not write Whazzup to disk" << out.fileName();
                }
            }
        } else {
            GuiMessages::message(
                QString("We already have Whazzup with that Timestamp: %1")
                .arg(_data.whazzupTime.toString("ddd MM/dd HHmm'z'"))
            );
        }
    }

    if(Settings::downloadPeriodically()) {
        const int serverNextUpdateInSec = QDateTime::currentDateTimeUtc().secsTo(_data.updateEarliest);
        if(
            _data.updateEarliest.isValid()
            && (Settings::downloadInterval() * 60 < serverNextUpdateInSec)
        )
        {
            _downloadTimer->start(serverNextUpdateInSec * 1000 + 60000); // 1min after later than reported
                                                                         // from server
                                                                         // - seems to report 0000z when
                                                                         // actually
                                                                         // updates at 0000:59z
            qDebug() << "Whazzup::whazzupDownloaded() correcting next update, will update in"
                     << serverNextUpdateInSec << "s to respect the server's minimum interval";
        } else {
            _downloadTimer->start(Settings::downloadInterval() * 60 * 1000);
        }
    }

    GuiMessages::remove("whazzupProcess");
}

void Whazzup::downloadBookings() {
    if(!Settings::downloadBookings()) {
        return;
    }
    QUrl url(Settings::bookingsLocation());

    _bookingsTimer->stop();

    GuiMessages::progress(
        "bookingsDownload",
        QString("Updating ATC Bookings from %1...")
        .arg(url.toString(QUrl::RemoveUserInfo))
    );

    _replyBookings = Net::g(url);
    connect(_replyBookings, &QNetworkReply::finished, this, &Whazzup::processBookings);
    connect(_replyBookings, &QNetworkReply::downloadProgress, this, &Whazzup::bookingsProgress);
}

void Whazzup::bookingsProgress(qint64 prog, qint64 tot) {
    GuiMessages::progress("bookingsDownload", prog, tot);
}

void Whazzup::processBookings() {
    qDebug() << "Whazzup::processBookings()";
    GuiMessages::remove("bookingsDownload");
    disconnect(_replyBookings, &QNetworkReply::finished, this, &Whazzup::processBookings);
    disconnect(_replyBookings, &QNetworkReply::downloadProgress, this, &Whazzup::bookingsProgress);
    qDebug() << "Whazzup::processBookings() deleting buffer";
    _replyBookings->deleteLater();
    if(_replyBookings == 0) {
        GuiMessages::criticalUserInteraction(
            "Buffer unavailable.",
            "Booking download Error"
        );
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

    if(Settings::downloadBookings() && Settings::bookingsPeriodically()) {
        _bookingsTimer->start(Settings::bookingsInterval() * 60 * 1000);
    }

    GuiMessages::progress("bookingsProcess", "Processing Bookings...");

    QByteArray* bytes = new QByteArray(_replyBookings->readAll());
    WhazzupData newBookingsData(bytes, WhazzupData::ATCBOOKINGS);
    if(!newBookingsData.isNull()) {
        qDebug() << "Whazzup::processBookings() step 2";
        if(newBookingsData.bookingsTime.secsTo(QDateTime::currentDateTimeUtc()) > 60 * 60 * 3) {
            GuiMessages::warning("Bookings data more than 3 hours old.");
        }

        if(newBookingsData.bookingsTime != _data.bookingsTime) {
            qDebug() << "Whazzup::processBookings() will call updateFrom()";
            _data.updateFrom(newBookingsData);
            qDebug() << "Whazzup::processBookings() Bookings updated from timestamp"
                     << _data.bookingsTime;

            QFile out(Settings::dataDirectory(
                    QString("downloaded/%1_%2.bookings")
                    .arg(Settings::downloadNetwork())
                    .arg(_data.bookingsTime.toString("yyyyMMdd-HHmmss"))
            ));
            if(!out.exists() && out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qDebug() << "Whazzup::processBookings writing bookings to" << out.fileName();
                out.write(bytes->constData());
                out.close();
            } else {
                qWarning() << "Whazzup::processBookings: Could not write Bookings to disk"
                           << out.fileName();
            }

            // from now on we want to redownload when the user triggers a network update
            connect(Window::instance()->actionDownload, &QAction::triggered, this, &Whazzup::downloadBookings);
            emit newData(true);
        } else {
            GuiMessages::message(
                QString("We already have bookings with that timestamp: %1")
                .arg(_data.bookingsTime.toString("ddd MM/dd HHmm'z'"))
            );
        }
    }
    GuiMessages::remove("bookingsProcess");
    qDebug() << "Whazzup::processBookings() -- finished";
}


QString Whazzup::userUrl(const QString& id) const {
    if(_user0Url.isEmpty() || !Client::isValidID(id)) {
        return QString();
    }
    return _user0Url + "?id=" + id;
}

QString Whazzup::metarUrl(const QString& id) const {
    if(_metar0Url.isEmpty()) {
        return QString();
    }
    return _metar0Url + "?id=" + id;
}

void Whazzup::setPredictedTime(QDateTime predictedTime) {
    if(this->predictedTime != predictedTime) {
        qDebug() << "Whazzup::setPredictedTime() predictedTime=" << predictedTime
                 << "data.whazzupTime=" << _data.whazzupTime;
        GuiMessages::progress("warpProcess", "Calculating Warp...");
        this->predictedTime = predictedTime;
        if(Settings::downloadBookings() && !_data.bookingsTime.isValid()) {
            emit needBookings();
        }
        if(predictedTime == _data.whazzupTime) {
            qDebug() << "Whazzup::setPredictedTime() predictedTime == data.whazzupTime"
                     << "(no need to predict, we have it already :) )";
            _predictedData = _data;
        } else {
            _predictedData.updateFrom(WhazzupData(predictedTime, _data));
        }
        GuiMessages::remove("warpProcess");
        emit newData(true);
    }
}

QList <QPair <QDateTime, QString> > Whazzup::downloadedWhazzups() const {
    // Process directory
    QStringList list = QDir(Settings::dataDirectory("downloaded/")).entryList(
        QStringList(QString("%1_*.whazzup").arg(Settings::downloadNetwork())),
        QDir::Files | QDir::Readable
    ); // getting all *.whazzup
    list.sort();

    QList <QPair <QDateTime, QString> > returnList;
    foreach(const QString filename, list) {
        QRegExp dtRe = QRegExp("\\.*_(\\d{8}-\\d{6})"); // dateTime: 20110301-191050
        if(dtRe.indexIn(filename) > 0) {
            QDateTime dt = QDateTime::fromString(dtRe.cap(1), "yyyyMMdd-HHmmss");
            dt.setTimeSpec(Qt::UTC);
            returnList.append(
                QPair<QDateTime, QString>(
                    dt,
                    Settings::dataDirectory(
                        QString("downloaded/%1").
                        arg(filename)
                    )
                )
            );
        }
    }
    return returnList;
}
