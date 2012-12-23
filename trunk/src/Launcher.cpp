#include "Launcher.h"

// singleton instance
Launcher *launcherInstance = 0;

Launcher* Launcher::instance(bool createIfNoInstance) {
    if(launcherInstance == 0 && createIfNoInstance)
        launcherInstance = new Launcher();
    return launcherInstance;
}

Launcher::Launcher(QWidget *parent) :
        QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint),
        _navReady(false), _windowReady(false), _windReady(false)
{
    map = QPixmap(":/startup/logo").scaled(600, 600);
    /*qDebug() << "isNull:" << map.isNull();
    qDebug() << "hasAlphaChannel:" << map.hasAlphaChannel();
    qDebug() << "w:" << map.width() << " h:" << map.height();*/

    _image = new QLabel(this);
    _text = new QLabel(this);
    _progress = new QProgressBar(this);

    _image->setPixmap(map);
    _image->resize(map.width(), map.height());

    resize(map.width(), map.height());

    _text->setText("Launcher started");
    _text->setStyleSheet(
        "QLabel {"
            "color: white;"
            "font-weight: bold;"
        "}"
    );
    _text->setAlignment(Qt::AlignCenter);
    _text->setWordWrap(true);
    _text->resize(440, _text->height());
    //qDebug() << "text frames w:" << text->frameSize() << " text h:" << text->height();
    _text->move((map.width() / 2) - 220, (map.height() / 3) * 2 + 30);

    _progress->resize(300, _progress->height());
    _progress->move((map.width() / 2) - 150,
                   (map.height() / 3) * 2 + 30 + _text->height());

    GuiMessages::instance()->addStatusLabel(_text, true);
    GuiMessages::instance()->addProgressBar(_progress, false); // non-autohide

    _image->lower();
    _text->raise();
    _progress->raise();

    setMask(map.mask());
    //qDebug() << "Widget w:" << this->width() << " Widget h:" << this->height();

    _finalTimer.setInterval(250);
    _finalTimer.setSingleShot(false);
    _windowTimer.setInterval(250);
    _windowTimer.setSingleShot(false);

    move(qApp->desktop()->availableGeometry().center()
         - rect().center());
    show();
    qApp->processEvents();
}

Launcher::~Launcher() {
    delete _image;
    delete _text;
}

///////////////////////////
// Events
///////////////////////////
void Launcher::keyReleaseEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape) {
        event->accept();
        qApp->quit();
    }
}

void Launcher::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        _dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void Launcher::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
         move(event->globalPos() - _dragPosition);
         event->accept();
    }
}

///////////////////////////
// General
///////////////////////////

void Launcher::fireUp() {
    qDebug() << "Launcher::fireUp -- started";

    //Check for navdataupdatets & loading navdata
    GuiMessages::status(tr("Checking for navdata updates"), "checknavdata");
    qApp->processEvents();

    //after Update loadNavdata will be started
    if (Settings::checkForUpdates()) {
        connect(this, SIGNAL(navdataUpdated()), SLOT(loadNavdata()));
        checkForDataUpdates();
    } else
        loadNavdata();



    //Download wind
    if (Settings::showWind())
        startWindDownload();


//    //get networksatus & Whazzupdata
//    GuiMessages::status(tr("Getting network status"), "gettingnetworkstatus");
//    qApp->processEvents();

//    qDebug() << "Launcher::fireUp() creating Whazzup";
//    Whazzup *whazzup = Whazzup::instance();
//    qDebug() << "Launcher::fireUp() creating Whazzup --finished";
//    if(Settings::downloadOnStartup()) {
//        // download whazzup as soon as whazzup status download is complete
//        connect(whazzup, SIGNAL(statusDownloaded()), whazzup, SLOT(download()));
//        connect(whazzup, SIGNAL(newData(bool)), this, SLOT(loadWindow()));
//    }


//    // Always download status
//    whazzup->setStatusLocation(Settings::statusLocation());


//    finalTimer.start();
//    while(finalTimer.isActive()) {
//        QCoreApplication::processEvents(QEventLoop::AllEvents, 250);
//        if(windowReady && windReady)
//            finalTimer.stop();
//    }

    close();
    qDebug() << "Launcher::fireUp -- finished";
}

///////////////////////////
//Navdata update & loading
///////////////////////////
void Launcher::checkForDataUpdates() {
    if(_dataVersionsAndFilesDownloader != 0)
        _dataVersionsAndFilesDownloader = 0;
    _dataVersionsAndFilesDownloader = new QHttp(this);
    QUrl url("http://svn.code.sf.net/p/qutescoop/code/trunk/data/dataversions.txt");
    _dataVersionsAndFilesDownloader->setHost(url.host());
    Settings::applyProxySetting(_dataVersionsAndFilesDownloader);

    connect(_dataVersionsAndFilesDownloader, SIGNAL(done(bool)), this, SLOT(dataVersionsDownloaded(bool)));

    _dataVersionsBuffer = new QBuffer;
    _dataVersionsBuffer->open(QBuffer::ReadWrite);

    _dataVersionsAndFilesDownloader->get(url.path(), _dataVersionsBuffer);
    qDebug() << "Launcher:: Checking for datafile versions:" << url.toString();
}

void Launcher::dataVersionsDownloaded(bool error) {
    disconnect(_dataVersionsAndFilesDownloader, SIGNAL(done(bool)), this, SLOT(dataVersionsDownloaded(bool)));
    if(_dataVersionsBuffer == 0) {
        emit navdataUpdated();
        return;
    }

    if(error) {
        GuiMessages::criticalUserInteraction(_dataVersionsAndFilesDownloader->errorString(), "Datafile download");
        emit navdataUpdated();
        return;
    }
    qDebug() << "Launcher::dataVersionsDownloaded";
    QList< QPair< QString, int> > serverDataVersionsList, localDataVersionsList;

    _dataVersionsBuffer->seek(0);
    while(!_dataVersionsBuffer->atEnd()) {
        QStringList splitLine = QString(_dataVersionsBuffer->readLine()).split("%%");
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
    connect(_dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
            this, SLOT(dataFilesRequestFinished(int,bool)));
    connect(_dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
            this, SLOT(dataFilesDownloaded(bool)));
    for(int i = 0; i < serverDataVersionsList.size(); i++) {
        // download also files that are locally not available
        if(serverDataVersionsList[i].second >
           localDataVersionsList.value(i, QPair< QString, int>(QString(), 0)).second)
        {
            _dataFilesToDownload.append(new QFile(Settings::applicationDataDirectory("data/%1.newFromServer")
                                                 .arg(serverDataVersionsList[i].first)));
            QUrl url(QString("http://svn.code.sf.net/p/qutescoop/code/trunk/data/%1")
                 .arg(serverDataVersionsList[i].first));
            _dataVersionsAndFilesDownloader->get(url.path(), _dataFilesToDownload.last());
            //qDebug() << "Downloading datafile" << url.toString();
        }
    }
    if (!_dataFilesToDownload.isEmpty()) {
        GuiMessages::informationUserAttention(
                QString("New sector-/ airport- or geography-files are available. They will be downloaded now."),
                "New datafiles");
    } else {
        disconnect(_dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
                this, SLOT(dataFilesRequestFinished(int,bool)));
        disconnect(_dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
                this, SLOT(dataFilesDownloaded(bool)));
        _dataVersionsAndFilesDownloader->abort();
        delete _dataVersionsAndFilesDownloader;
        delete _dataVersionsBuffer;
        emit navdataUpdated();
    }
}

void Launcher::dataFilesRequestFinished(int id, bool error) {
    Q_UNUSED(id);
    if (error) {
        GuiMessages::criticalUserInteraction(QString("Error downloading %1:\n%2")
                                            .arg(_dataVersionsAndFilesDownloader->currentRequest().path())
                                            .arg(_dataVersionsAndFilesDownloader->errorString()),
                                            "New datafiles");
        return;
    }
    GuiMessages::informationUserAttention(QString("Downloaded %1")
                                         .arg(_dataVersionsAndFilesDownloader->currentRequest().path()),
                                         "New datafiles");
}

void Launcher::dataFilesDownloaded(bool error) {
    disconnect(_dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
            this, SLOT(dataFilesRequestFinished(int,bool)));
    disconnect(_dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
            this, SLOT(dataFilesDownloaded(bool)));
    if (_dataVersionsBuffer == 0) {
        emit navdataUpdated();
        return;}

    if (error) {
        GuiMessages::criticalUserInteraction(
                    QString("New sector- / airport- / geography-files could not be downloaded.\n%1")
                        .arg(_dataVersionsAndFilesDownloader->errorString()),
                    "New datafiles");
        emit navdataUpdated();
        return;
    }

    GuiMessages::informationUserAttention(
                "All scheduled files have been downloaded.\n"
                "These changes will take effect on the next start of QuteScoop.",
                "New datafiles");

    int errors = 0;
    for(int i = 0; i < _dataFilesToDownload.size(); i++) {
        _dataFilesToDownload[i]->flush();
        _dataFilesToDownload[i]->close();

        if(_dataFilesToDownload[i]->exists()) {
            QString datafileFilePath = _dataFilesToDownload[i]->fileName().remove(".newFromServer");
            if (QFile::exists(datafileFilePath) && !QFile::remove(datafileFilePath)) {
                GuiMessages::criticalUserInteraction(QString("Unable to delete\n%1")
                                                    .arg(datafileFilePath), "New datafiles");
                errors++;
            }
            if (!_dataFilesToDownload[i]->rename(datafileFilePath)) {
                GuiMessages::criticalUserInteraction(QString("Unable to move downloaded file to\n%1")
                                                    .arg(datafileFilePath), "New datafiles");
                errors++;
            }
        }

        delete _dataFilesToDownload[i];
    }

    if (errors == 0) {
        QFile localDataVersionsFile(Settings::applicationDataDirectory("data/dataversions.txt"));
        if (localDataVersionsFile.open(QIODevice::WriteOnly))
            localDataVersionsFile.write(_dataVersionsBuffer->data());
        else
            GuiMessages::criticalUserInteraction(
                        QString("Error writing %1").arg(localDataVersionsFile.fileName()),
                        "New datafiles");
    } else
        GuiMessages::criticalUserInteraction(
                    QString("Errors occured. All datafiles will be redownloaded on next launch of QuteScoop."),
                    "New datafiles");

    _dataVersionsBuffer->close();
    delete _dataVersionsBuffer;
    _dataVersionsAndFilesDownloader->abort();
    delete _dataVersionsAndFilesDownloader;
    _dataFilesToDownload.clear();

    emit navdataUpdated();
}

void Launcher::loadNavdata() {
    qDebug() << "Launcher:loadNavdata -- Started";

    GuiMessages::remove("checknavdata");
    GuiMessages::status(tr("Loading Navdata"), "loadnavdata");
    qApp->processEvents();

    NavData::instance();
    Airac::instance();
    _navReady = true;
    GuiMessages::remove("loadnavdata");
    qApp->processEvents();
    qDebug() << "Launcher:loadNavdata -- Finished";

}



//////////////////////////
//Whazzup & Window
//////////////////////////

void Launcher::loadWindow() {
    _windowTimer.start();
    while(_windowTimer.isActive()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 250);
        if(_navReady) _windowTimer.stop();
    }
    qDebug() << "Launcher::loadWindow -- starting loading";

    GuiMessages::remove("gettingnetworkstatus");
    GuiMessages::status(tr("Loading window"), "loadwindow");

    Window::instance(true)->whazzupDownloaded(true);
    Window::instance(true)->mapScreen->glWidget->newWhazzupData(true);
    _windowReady = true;
    GuiMessages::remove("loadwindow");
    close();
}

/////////////////////////
// Wind
/////////////////////////

void Launcher::startWindDownload() {
    if(_windDataDownloader != 0) _windDataDownloader = 0;

    _windDataDownloader= new QHttp(this);
    QUrl url("http://fsrealwx.rs-transline.de/upperair.txt");
    connect(_windDataDownloader, SIGNAL(done(bool)) , this , SLOT(startWindDecoding(bool)));
    connect(_windDataDownloader, SIGNAL(dataReadProgress(int,int)) , this , SLOT(windProgress(int, int)));

    GuiMessages::progress("loadwind",
                          QString("Downloading wind data from %1...").arg(url.toString(QUrl::RemoveUserInfo)));
    _windDataBuffer = new QBuffer;
    _windDataBuffer->open(QBuffer::ReadWrite);

    _windDataDownloader->setHost(url.host());
    _windDataDownloader->get(url.path(), _windDataBuffer);
    qDebug() << "Launcher::startWindDownload" << url;
}

void Launcher::windProgress(int prog, int total) {
    GuiMessages::progress("loadwind", prog, total);
}

void Launcher::startWindDecoding(bool error) {
    qDebug() << "Launcher::startWindDecoding -- WindData downloaded";

    GuiMessages::remove("loadwind");
    if(_windDataBuffer == 0) return;

    if(error) {
        GuiMessages::criticalUserInteraction(_windDataDownloader->errorString() , "WindData download");
        return;
    }

    _windDataBuffer->seek(0);

    QString data = QString(_windDataBuffer->readAll());

    GuiMessages::status("Decoding wind data...", "decodewind");
    WindData::instance()->setRawData(data);
    WindData::instance()->decodeData();
    GuiMessages::remove("decodewind");

    _windReady = true;
}
