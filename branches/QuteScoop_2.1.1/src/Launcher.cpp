#include "Launcher.h"


Launcher::Launcher(QWidget *parent) :
        QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint),
        windowReady(false),
        navReady(false),
        windReady(false)
{
    map = QPixmap(":/startup/logo").scaled(600,600);
    /*qDebug() << "isNull:" << map.isNull();
    qDebug() << "hasAlphaChannel:" << map.hasAlphaChannel();
    qDebug() << "w:" << map.width() << " h:" << map.height();*/

    image = new QLabel(this);
    text = new QLabel(this);
    progress = new QProgressBar(this);

    image->setPixmap(map);
    image->resize(map.width(), map.height());

    resize(map.width(), map.height());

    text->setText("Launcher started");
    text->setStyleSheet(
        "QLabel {"
            "color: white;"
            "font-weight: bold;"
        "}"
    );
    text->setAlignment(Qt::AlignCenter);
    text->setWordWrap(true);
    text->resize(440, text->height());
    //qDebug() << "text frames w:" << text->frameSize() << " text h:" << text->height();
    text->move((map.width() / 2) - 220, (map.height() / 3) * 2 + 30);

    progress->resize(300, progress->height());
    progress->move((map.width() / 2) - 150,
                   (map.height() / 3) * 2 + 30 + text->height());

    GuiMessages::getInstance()->addStatusLabel(text, true);
    GuiMessages::getInstance()->addProgressBar(progress, false); // non-autohide

    image->lower();
    text->raise();
    progress->raise();

    setMask(map.mask());
    //qDebug() << "Widget w:" << this->width() << " Widget h:" << this->height();

    finalTimer.setInterval(250);
    finalTimer.setSingleShot(false);
    windowTimer.setInterval(250);
    windowTimer.setSingleShot(false);

    move(qApp->desktop()->availableGeometry().center()
         - rect().center());
    show();
    qApp->processEvents();
}

Launcher::~Launcher() {
    GuiMessages::getInstance()->removeStatusLabel(text);
    GuiMessages::getInstance()->removeProgressBar(progress);
    delete image, text;
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
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void Launcher::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
         move(event->globalPos() - dragPosition);
         event->accept();
    }
}

///////////////////////////
// General
///////////////////////////

void Launcher::fireUp() {
    qDebug() << "Launcher:fireUp -- started";

    //Check for navdataupdatets & loading navdata
    GuiMessages::status(tr("Checking for navdata updates"), "checknavdata");
    qApp->processEvents();

    //after Update loadNavdata will be started
    connect(this, SIGNAL(navdataUpdated()), this, SLOT(loadNavdata()));
    if(Settings::checkForUpdates())
        checkForDataUpdates();



    //Download wind
    startWindDownload();


    //get networksatus & Whazzupdata
    GuiMessages::status(tr("Getting network status"), "gettingnetworkstatus");
    qApp->processEvents();

    qDebug() << "Launcher::fireUp() creating Whazzup";
    Whazzup *whazzup = Whazzup::getInstance();
    qDebug() << "Launcher::fireUp() creating Whazzup --finished";
    if(Settings::downloadOnStartup()) {
        // download whazzup as soon as whazzup status download is complete
        connect(whazzup, SIGNAL(statusDownloaded()), whazzup, SLOT(download()));
        connect(whazzup, SIGNAL(newData(bool)), this, SLOT(loadWindow()));
    }


    // Always download status
    whazzup->setStatusLocation(Settings::statusLocation());


    finalTimer.start();
    while(finalTimer.isActive()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 250);
        if(windowReady && windReady)
            finalTimer.stop();
    }


    qDebug() << "Launcher::fireUp -- finished";
}

///////////////////////////
//Navdata update & loading
///////////////////////////
void Launcher::checkForDataUpdates() {
    if(dataVersionsAndFilesDownloader != 0)
        dataVersionsAndFilesDownloader = 0;
    dataVersionsAndFilesDownloader = new QHttp(this);
    QUrl url("http://svn.code.sf.net/p/qutescoop/code/trunk/QuteScoop/data/dataversions.txt");
    dataVersionsAndFilesDownloader->setHost(url.host());
    Settings::applyProxySetting(dataVersionsAndFilesDownloader);

    connect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)), this, SLOT(dataVersionsDownloaded(bool)));

    dataVersionsBuffer = new QBuffer;
    dataVersionsBuffer->open(QBuffer::ReadWrite);

    dataVersionsAndFilesDownloader->get(url.path(), dataVersionsBuffer);
    qDebug() << "Launcher:: Checking for datafile versions:" << url.toString();
}

void Launcher::dataVersionsDownloaded(bool error) {
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)), this, SLOT(dataVersionsDownloaded(bool)));
    if(dataVersionsBuffer == 0) {
        emit navdataUpdated();
        return;
    }

    if(error) {
        GuiMessages::criticalUserInteraction(dataVersionsAndFilesDownloader->errorString(), "Datafile download");
        emit navdataUpdated();
        return;
    }
    qDebug() << "Launcher::dataVersionsDownloaded";
    QList< QPair< QString, int> > serverDataVersionsList, localDataVersionsList;

    dataVersionsBuffer->seek(0);
    while(!dataVersionsBuffer->atEnd()) {
        QStringList splitLine = QString(dataVersionsBuffer->readLine()).split("%%");
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
    connect(dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
            this, SLOT(dataFilesRequestFinished(int,bool)));
    connect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
            this, SLOT(dataFilesDownloaded(bool)));
    for(int i = 0; i < serverDataVersionsList.size(); i++) {
        // download also files that are locally not available
        if(serverDataVersionsList[i].second >
           localDataVersionsList.value(i, QPair< QString, int>(QString(), 0)).second)
        {
            dataFilesToDownload.append(new QFile(Settings::applicationDataDirectory("data/%1.newFromServer")
                                                 .arg(serverDataVersionsList[i].first)));
            QUrl url(QString("http://svn.code.sf.net/p/qutescoop/code/trunk/QuteScoop/data/%1")
                 .arg(serverDataVersionsList[i].first));
            dataVersionsAndFilesDownloader->get(url.path(), dataFilesToDownload.last());
            //qDebug() << "Downloading datafile" << url.toString();
        }
    }
    if (!dataFilesToDownload.isEmpty()) {
        GuiMessages::informationUserAttention(
                QString("New sector-/ airport- or geography-files are available. They will be downloaded now."),
                "New datafiles");
    } else {
        disconnect(dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
                this, SLOT(dataFilesRequestFinished(int,bool)));
        disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
                this, SLOT(dataFilesDownloaded(bool)));
        dataVersionsAndFilesDownloader->abort();
        delete dataVersionsAndFilesDownloader;
        delete dataVersionsBuffer;
        emit navdataUpdated();
    }
}

void Launcher::dataFilesRequestFinished(int id, bool error) {
    if (error) {
        GuiMessages::criticalUserInteraction(QString("Error downloading %1:\n%2")
                                            .arg(dataVersionsAndFilesDownloader->currentRequest().path())
                                            .arg(dataVersionsAndFilesDownloader->errorString()),
                                            "New datafiles");
        return;
    }
    GuiMessages::informationUserAttention(QString("Downloaded %1")
                                         .arg(dataVersionsAndFilesDownloader->currentRequest().path()),
                                         "New datafiles");
}

void Launcher::dataFilesDownloaded(bool error) {
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
            this, SLOT(dataFilesRequestFinished(int,bool)));
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
            this, SLOT(dataFilesDownloaded(bool)));
    if (dataVersionsBuffer == 0){
        emit navdataUpdated();
        return;}

    if (error) {
        GuiMessages::criticalUserInteraction(
                    QString("New sector- / airport- / geography-files could not be downloaded.\n%1")
                        .arg(dataVersionsAndFilesDownloader->errorString()),
                    "New datafiles");
        emit navdataUpdated();
        return;
    }

    GuiMessages::informationUserAttention(
                "All scheduled files have been downloaded.\n"
                "These changes will take effect on the next start of QuteScoop.",
                "New datafiles");

    int errors = 0;
    for(int i = 0; i < dataFilesToDownload.size(); i++) {
        dataFilesToDownload[i]->flush();
        dataFilesToDownload[i]->close();

        if(dataFilesToDownload[i]->exists()) {
            QString datafileFilePath = dataFilesToDownload[i]->fileName().remove(".newFromServer");
            if (QFile::exists(datafileFilePath) && !QFile::remove(datafileFilePath)) {
                GuiMessages::criticalUserInteraction(QString("Unable to delete\n%1")
                                                    .arg(datafileFilePath), "New datafiles");
                errors++;
            }
            if (!dataFilesToDownload[i]->rename(datafileFilePath)) {
                GuiMessages::criticalUserInteraction(QString("Unable to move downloaded file to\n%1")
                                                    .arg(datafileFilePath), "New datafiles");
                errors++;
            }
        }

        delete dataFilesToDownload[i];
    }

    if (errors == 0) {
        QFile localDataVersionsFile(Settings::applicationDataDirectory("data/dataversions.txt"));
        if (localDataVersionsFile.open(QIODevice::WriteOnly))
            localDataVersionsFile.write(dataVersionsBuffer->data());
        else
            GuiMessages::criticalUserInteraction(
                        QString("Error writing %1").arg(localDataVersionsFile.fileName()),
                        "New datafiles");
    } else
        GuiMessages::criticalUserInteraction(
                    QString("Errors occured. All datafiles will be redownloaded on next launch of QuteScoop."),
                    "New datafiles");

    dataVersionsBuffer->close();
    delete dataVersionsBuffer;
    dataVersionsAndFilesDownloader->abort();
    delete dataVersionsAndFilesDownloader;
    dataFilesToDownload.clear();

    emit navdataUpdated();
}

void Launcher::loadNavdata()
{
    qDebug() << "Launcher:loadNavdata -- Started";

    GuiMessages::remove("checknavdata");
    GuiMessages::status(tr("Loading Navdata"), "loadnavdata");
    qApp->processEvents();

    NavData::getInstance();
    Airac::getInstance();
    navReady = true;
    GuiMessages::remove("loadnavdata");
    qApp->processEvents();
    qDebug() << "Launcher:loadNavdata -- Finished";

}



//////////////////////////
//Whatzupp & Window
//////////////////////////

void Launcher::loadWindow()
{
    windowTimer.start();
    while(windowTimer.isActive()){
        QCoreApplication::processEvents(QEventLoop::AllEvents, 250);
        if(navReady) windowTimer.stop();
    }
    qDebug() << "Launcher::loadWindow -- starting loading";

    GuiMessages::remove("gettingnetworkstatus");
    GuiMessages::status(tr("Loading window"), "loadwindow");

    Window *window = Window::getInstance(true);
    window->whazzupDownloaded(true);
    window->mapScreen->glWidget->newWhazzupData(true);
    windowReady = true;
    GuiMessages::remove("loadwindow");
}

/////////////////////////
// Wind
/////////////////////////

void Launcher::startWindDownload(){
    if(windDataDownloader != 0) windDataDownloader = 0;

    GuiMessages::status(tr("Download wind data"), "loadwind");

    windDataDownloader= new QHttp(this);
    QUrl url("http://fsrealwx.rs-transline.de/upperair.txt");
    connect(windDataDownloader, SIGNAL(done(bool)) , this , SLOT(startWindDecoding(bool)));

    windDataBuffer = new QBuffer;
    windDataBuffer->open(QBuffer::ReadWrite);

    windDataDownloader->setHost(url.host());
    windDataDownloader->get(url.path(), windDataBuffer);
    qDebug() << "Launcher::startWindDownload -- WindData download started";
}

void Launcher::startWindDecoding(bool error)
{
    qDebug() << "Launcher::startWindDecoding -- WindData downloaded";
    if(windDataBuffer == 0) return;

    if(error) {
        GuiMessages::criticalUserInteraction(windDataDownloader->errorString() , "WindData download");
        return;
    }

    windDataBuffer->seek(0);

    QString data = QString(windDataBuffer->readAll());

    GuiMessages::remove("loadwind");
    GuiMessages::status(tr("Decode wind data"), "decodewind");

    WindData::getInstance()->setRawData(data);

    WindData::getInstance()->decodeData();

    GuiMessages::remove("decodewind");
    windReady = true;
}
