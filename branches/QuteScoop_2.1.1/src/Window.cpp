/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Window.h"

#include "GLWidget.h"
#include "ClientDetails.h"
#include "PilotDetails.h"
#include "ControllerDetails.h"
#include "AirportDetails.h"
#include "LogBrowserDialog.h"
#include "PreferencesDialog.h"
#include "PlanFlightDialog.h"
#include "BookedAtcDialog.h"
#include "ListClientsDialog.h"
#include "Whazzup.h"
#include "Settings.h"
#include "SearchVisitor.h"
#include "MetarSearchVisitor.h"
#include "NavData.h"
#include "FriendsVisitor.h"
#include "helpers.h"
#include "GuiMessage.h"

// singleton instance
Window *windowInstance = 0;

Window* Window::getInstance(bool createIfNoInstance) {
    if(windowInstance == 0)
        if (createIfNoInstance)
            windowInstance = new Window;
    return windowInstance;
}

Window::Window(QWidget *parent) :
    QMainWindow(parent) {
    if(Settings::resetOnNextStart())
        QSettings().clear();

    setupUi(this);
    // restore saved states
    if (!Settings::getSavedSize().isNull())     resize(Settings::getSavedSize());
    if (!Settings::getSavedPosition().isNull()) move(Settings::getSavedPosition());
    if (!Settings::getSavedGeometry().isNull()) restoreGeometry(Settings::getSavedGeometry());
    if (!Settings::getSavedState().isNull())    restoreState(Settings::getSavedState());

    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    setWindowTitle(QString("QuteScoop %1").arg(VERSION_NUMBER));

    //QSettings* settings = new QSettings();

    // apply styleSheet
    if (!Settings::stylesheet().isEmpty()) {
        qDebug() << "Window::Window() applying styleSheet:" << Settings::stylesheet();
        setStyleSheet(Settings::stylesheet());
    }



    //The map
    mapScreen = new MapScreen(this);
    centralwidget->layout()->addWidget(mapScreen);

    connect(mapScreen, SIGNAL(toggleRoutes()), actionShowRoutes, SLOT(trigger()));
    connect(mapScreen, SIGNAL(toggleSectors(bool)), this, SLOT(allSectorsChanged(bool)));
    connect(mapScreen, SIGNAL(toggleRouteWaypoints()), actionShowWaypoints, SLOT(trigger()));
    connect(mapScreen, SIGNAL(toggleInactiveAirports()), actionShowInactiveAirports,SLOT(trigger()));

    // Status- & ProgressBar
    progressBar = new QProgressBar(statusbar);
    progressBar->setMaximumWidth(200);
    progressBar->hide();
    lblStatus = new QLabel(statusbar);
    statusbar->addWidget(lblStatus, 5);
    statusbar->addWidget(progressBar, 3);
    statusbar->addPermanentWidget(tbZoomIn, 0);
    statusbar->addPermanentWidget(tbZoomOut, 0);

    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(actionToggleFullscreen, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));
    connect(actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
    connect(actionPlanFlight, SIGNAL(triggered()), this, SLOT(openPlanFlight()));
    connect(actionBookedAtc, SIGNAL(triggered()), this, SLOT(openBookedAtc()));
    connect(actionListClients, SIGNAL(triggered()), this, SLOT(openListClients()));
    connect(actionSectorview, SIGNAL(triggered()), this, SLOT(openSectorView()));

    qDebug() << "Window::Window() creating Whazzup";
    Whazzup *whazzup = Whazzup::getInstance();
    qDebug() << "Window::Window() creating Whazzup --finished";
    connect(actionDownload, SIGNAL(triggered()), whazzup, SLOT(download()));
    //connect(actionDownload, SIGNAL(triggered()), glWidget, SLOT(updateGL()));

    // these 2 get disconnected and connected again to inhibit unnecessary updates:
    connect(whazzup, SIGNAL(newData(bool)), mapScreen->glWidget, SLOT(newWhazzupData(bool)));
    connect(whazzup, SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));

    if(Settings::downloadOnStartup())
        // download whazzup as soon as whazzup status download is complete
        connect(whazzup, SIGNAL(statusDownloaded()), whazzup, SLOT(download()));

    // Always download status
    whazzup->setStatusLocation(Settings::statusLocation());

    searchResult->setModel(&searchResultModel);
    connect(searchResult, SIGNAL(doubleClicked(const QModelIndex&)),
            &searchResultModel, SLOT(modelDoubleClicked(const QModelIndex&)));
    connect(searchResult, SIGNAL(clicked(const QModelIndex&)),
            &searchResultModel, SLOT(modelClicked(const QModelIndex&)));
    searchResult->sortByColumn(0, Qt::AscendingOrder);

    metarSortModel = new QSortFilterProxyModel;
    metarSortModel->setDynamicSortFilter(true);
    metarSortModel->setSourceModel(&metarModel);
    metarList->setModel(metarSortModel);

    connect(metarList, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(metarDoubleClicked(const QModelIndex&)));
    connect(metarList, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(metarDoubleClicked(const QModelIndex&)));
    connect(metarList->header(), SIGNAL(sectionClicked(int)),
            metarList, SLOT(sortByColumn(int)));
    metarList->sortByColumn(0, Qt::AscendingOrder);

    friendsSortModel = new QSortFilterProxyModel;
    friendsSortModel->setDynamicSortFilter(true);
    friendsSortModel->setSourceModel(&friendsModel);
    friendsList->setModel(friendsSortModel);

    connect(friendsList, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(friendDoubleClicked(const QModelIndex&)));
    connect(friendsList, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(friendClicked(const QModelIndex&)));
    connect(friendsList->header(), SIGNAL(sectionClicked(int)),
            friendsList, SLOT(sortByColumn(int)));
    metarList->sortByColumn(0, Qt::AscendingOrder);

    connect(&searchTimer, SIGNAL(timeout()), this, SLOT(performSearch()));
    connect(&metarTimer, SIGNAL(timeout()), this, SLOT(updateMetars()));
    connect(&downloadWatchdog, SIGNAL(timeout()),
            this, SLOT(downloadWatchdogTriggered()));

    actionDisplayAllSectors->setChecked(Settings::showAllSectors());
    connect(actionDisplayAllSectors, SIGNAL(toggled(bool)), this, SLOT(allSectorsChanged(bool)));
            //mapScreen->glWidget, SLOT(displayAllSectors(bool)));


    actionShowInactiveAirports->setChecked(Settings::showInactiveAirports());
    connect(actionShowInactiveAirports, SIGNAL(toggled(bool)),
            mapScreen->glWidget, SLOT(showInactiveAirports(bool)));

    connect(metarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(metarDockMoved(Qt::DockWidgetArea)));
    connect(searchDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(searchDockMoved(Qt::DockWidgetArea)));
    connect(metarDecoderDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(metarDecoderDockMoved(Qt::DockWidgetArea)));
    metarDecoderDock->hide();

    connect(friendsDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(friendsDockMoved(Qt::DockWidgetArea)));

    pb_highlightFriends->setChecked(Settings::highlightFriends());
    actionHighlight_Friends->setChecked(Settings::highlightFriends());

    versionChecker = 0;
    versionBuffer = 0;
    //if(Settings::checkForUpdates()) // disabled
    //    checkForUpdates();

    // Forecast / Predict settings
    framePredict->hide();
    editPredictTimer.stop();
    connect(&editPredictTimer, SIGNAL(timeout()), this, SLOT(performWarp()));
    runPredictTimer.stop();
    connect(&runPredictTimer, SIGNAL(timeout()), this, SLOT(runPredict()));
    widgetRunPredict->hide();

    QFont font = lblWarpInfo->font();
    font.setPointSize(lblWarpInfo->fontInfo().pointSize() - 1);
    lblWarpInfo->setFont(font); //make it a bit smaller than standard text

    font = cbUseDownloaded->font();
    font.setPointSize(cbUseDownloaded->fontInfo().pointSize() - 1);
    cbUseDownloaded->setFont(font); //make it a bit smaller than standard text

    font = cbOnlyUseDownloaded->font();
    font.setPointSize(cbOnlyUseDownloaded->fontInfo().pointSize() - 1);
    cbOnlyUseDownloaded->setFont(font); //make it a bit smaller than standard text

    setEnableBookedAtc(Settings::downloadBookings());
    actionShowWaypoints->setChecked(Settings::showUsedWaypoints());

    if(windDataDownloader != 0) windDataDownloader = 0;

    windDataDownloader= new QHttp(this);
    QUrl url("http://fsrealwx.rs-transline.de/upperair.txt");
    connect(windDataDownloader, SIGNAL(done(bool)) , this , SLOT(startWindDecoding(bool)));

    windDataBuffer = new QBuffer;
    windDataBuffer->open(QBuffer::ReadWrite);

    windDataDownloader->setHost(url.host());
    windDataDownloader->get(url.path(), windDataBuffer);
    qDebug() << "Window::Window -- WindData download started";

    connect(&cloudTimer, SIGNAL(timeout()), this, SLOT(startCloudDownload()));

    if(Settings::showClouds()){
        startCloudDownload();
    }


    //LogBrowser
#ifdef QT_NO_DEBUG_OUTPUT
    menuView->removeAction(actionDebugLog);
    qDebug() << "Window::Window() Debug Log Browser deactivated due to QT_NO_DEBUG_OUTPUT";
#endif

    qDebug() << "Window::Window() connecting me as GuiMessages listener";
    GuiMessages::getInstance()->addProgressBar(progressBar, true);
    GuiMessages::getInstance()->addStatusLabel(lblStatus, false);
    qDebug() << "Window::Window() --finished";
}

Window::~Window() {
}

void Window::toggleFullscreen() {
    if(isFullScreen()) {
        showNormal();
        statusBar()->show();
    } else {
        statusBar()->hide();
        showFullScreen();
    }
}

void Window::about() {
    QFile readmeFile(qApp->applicationDirPath() + "/README.html");
    readmeFile.open(QIODevice::ReadOnly);
    QMessageBox::about(this, tr("About QuteScoop"), readmeFile.readAll());
}

void Window::whazzupDownloaded(bool isNew) {
    qDebug() << "Window::whazzupDownloaded() isNew =" << isNew;
    const WhazzupData &realdata = Whazzup::getInstance()->realWhazzupData();
    const WhazzupData &data = Whazzup::getInstance()->whazzupData();

    QString msg = QString(tr("%1%2 - %3: %4 clients"))
                  .arg(Settings::downloadNetworkName())
                  .arg(Whazzup::getInstance()->getPredictedTime().isValid()
                       ? " - <b>W A R P E D</b>  to"
                       : ""
                       )
                  .arg(data.whazzupTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
                        ? QString("today %1").arg(data.whazzupTime.time().toString("HHmm'z'"))
                        : data.whazzupTime.toString("ddd MM/dd HHmm'z'"))
                  .arg(data.pilots.size() + data.controllers.size());
    GuiMessages::status(msg, "status");

    msg = QString("Whazzup %1, bookings %2 updated")
                  .arg(realdata.whazzupTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
                        ? QString("today %1").arg(realdata.whazzupTime.time().toString("HHmm'z'"))
                        : (realdata.whazzupTime.isValid()
                           ? realdata.whazzupTime.toString("ddd MM/dd HHmm'z'")
                           : "never")
                        )
                  .arg(realdata.bookingsTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
                        ? QString("today %1").arg(realdata.bookingsTime.time().toString("HHmm'z'"))
                        : (realdata.bookingsTime.isValid()
                           ? realdata.bookingsTime.toString("ddd MM/dd HHmm'z'")
                           : "never")
                        );
    lblWarpInfo->setText(msg);

    if (Whazzup::getInstance()->getPredictedTime().isValid()) {
        framePredict->show();
        dateTimePredict->setDateTime(Whazzup::getInstance()->getPredictedTime());
        if(isNew) {
            // recalculate prediction on new data arrived
            if(data.predictionBasedOnTime != realdata.whazzupTime
                || data.predictionBasedOnBookingsTime != realdata.bookingsTime)
                Whazzup::getInstance()->setPredictedTime(dateTimePredict->dateTime());
        }
    }

    if(isNew) {
        mapScreen->glWidget->clientSelection->close();
        mapScreen->glWidget->clientSelection->clearClients();

        performSearch();

        if (AirportDetails::getInstance(false) != 0) {
            if (AirportDetails::getInstance(true)->isVisible())
                AirportDetails::getInstance(true)->refresh();
            else // not visible -> delete it...
                AirportDetails::getInstance(true)->destroyInstance();
        }
        if (PilotDetails::getInstance(false) != 0) {
            if (PilotDetails::getInstance(true)->isVisible())
                PilotDetails::getInstance(true)->refresh();
            else // not visible -> delete it...
                PilotDetails::getInstance(true)->destroyInstance();
        }
        if (ControllerDetails::getInstance(false) != 0) {
            if (ControllerDetails::getInstance(true)->isVisible())
                ControllerDetails::getInstance(true)->refresh();
            else // not visible -> delete it...
                ControllerDetails::getInstance(true)->destroyInstance();
        }

        if (ListClientsDialog::getInstance(false) != 0) {
            if (ListClientsDialog::getInstance(true)->isVisible())
                ListClientsDialog::getInstance(true)->refresh();
            else // not visible -> delete it...
                ListClientsDialog::getInstance(true)->destroyInstance();
        }

        if(realdata.bookingsTime.isValid()) {
            if (BookedAtcDialog::getInstance(false) != 0) {
                if (BookedAtcDialog::getInstance(true)->isVisible())
                    BookedAtcDialog::getInstance(true)->refresh();
                else // not visible -> delete it...
                    BookedAtcDialog::getInstance(true)->destroyInstance();
            }
        }

        refreshFriends();

        if (Settings::shootScreenshots())
            shootScreenshot();
    }
    downloadWatchdog.stop();
    if(Settings::downloadPeriodically())
        downloadWatchdog.start(Settings::downloadInterval() * 60 * 1000 * 4);

    qDebug() << "Window::whazzupDownloaded() -- finished";
}

void Window::refreshFriends() {
    // update friends list
    FriendsVisitor *visitor = new FriendsVisitor();
    Whazzup::getInstance()->whazzupData().accept(visitor);
    friendsModel.setData(visitor->result());
    delete visitor;
    friendsList->reset();
}

void Window::openPreferences() {
    //if (!Settings::preferencesDialogSize().isNull())     {PreferencesDialog::getInstance(true, this)->resize(Settings::preferencesDialogSize());}
    if (!Settings::preferencesDialogPos().isNull()) PreferencesDialog::getInstance(true)->move(Settings::preferencesDialogPos());
    if (!Settings::preferencesDialogGeometry().isNull()) PreferencesDialog::getInstance(true)->restoreGeometry(Settings::preferencesDialogGeometry());

    PreferencesDialog::getInstance(true, this)->show();
    PreferencesDialog::getInstance(true)->raise();
    PreferencesDialog::getInstance(true)->activateWindow();
    PreferencesDialog::getInstance(true)->setFocus();
}

void Window::openPlanFlight() {
    PlanFlightDialog::getInstance(true, this)->show();
    PlanFlightDialog::getInstance(true)->raise();
    PlanFlightDialog::getInstance(true)->activateWindow();
    PlanFlightDialog::getInstance(true)->setFocus();

    //if (Settings::planFlightDialogSize().isNull() == false) {PlanFlightDialog::getInstance(true)->resize(Settings::planFlightDialogSize());}
    if (Settings::planFlightDialogPos().isNull() == false)  {PlanFlightDialog::getInstance(true)->move(Settings::planFlightDialogPos());}
    if (Settings::planFlightDialogGeometry().isNull() == false)    {PlanFlightDialog::getInstance(true)->restoreGeometry(Settings::planFlightDialogGeometry());}

}

void Window::openBookedAtc() {
    BookedAtcDialog::getInstance(true, this)->show();
    BookedAtcDialog::getInstance(true)->raise();
    BookedAtcDialog::getInstance(true)->activateWindow();
    BookedAtcDialog::getInstance(true)->setFocus();
}

void Window::openListClients()
{
    ListClientsDialog::getInstance(true, this)->show();
    ListClientsDialog::getInstance(true)->raise();
    ListClientsDialog::getInstance(true)->activateWindow();
    //ListClientsDialog::getInstance(true)->setFocus();
}

void Window::on_actionDebugLog_triggered()
{
    LogBrowserDialog::getInstance(true, this)->show();
    LogBrowserDialog::getInstance(true)->raise();
    LogBrowserDialog::getInstance(true)->activateWindow();
    LogBrowserDialog::getInstance(true)->setFocus();
}

void Window::on_searchEdit_textChanged(const QString& text) {
    if(text.length() < 2) {
        searchTimer.stop();
        searchResultModel.setData(QList<MapObject*>());
        searchResult->reset();
        return;
    }

    searchTimer.start(400);
}

void Window::performSearch() {
    if(searchEdit->text().length() < 2)
        return;

    searchTimer.stop();
    SearchVisitor *visitor = new SearchVisitor(searchEdit->text());
    NavData::getInstance()->accept(visitor);
    Whazzup::getInstance()->whazzupData().accept(visitor);

    searchResultModel.setData(visitor->result());
    delete visitor;

    searchResult->reset();
}

void Window::closeEvent(QCloseEvent *event) {
    Settings::saveState(saveState()); //was: (VERSION_INT) but that should not harm
    Settings::saveGeometry(saveGeometry()); // added this 'cause maximized wasn't saved
    Settings::saveSize(size()); // readded as Mac OS had problems with geometry only
    Settings::savePosition(pos());
    on_actionHideAllWindows_triggered();
    mapScreen->glWidget->savePosition();
    event->accept();
}

void Window::on_actionHideAllWindows_triggered() {
    if (PilotDetails::getInstance(false) != 0) PilotDetails::getInstance(true)->close();
    if (ControllerDetails::getInstance(false) != 0) ControllerDetails::getInstance(true)->close();
    if (AirportDetails::getInstance(false) != 0) AirportDetails::getInstance(true)->close();
    if (PreferencesDialog::getInstance(false) != 0) PreferencesDialog::getInstance(true)->close();
    if (PlanFlightDialog::getInstance(false) != 0) PlanFlightDialog::getInstance(true)->close();
    if (BookedAtcDialog::getInstance(false) != 0) BookedAtcDialog::getInstance(true)->close();
    if (ListClientsDialog::getInstance(false) != 0) ListClientsDialog::getInstance(true)->close();
    if (LogBrowserDialog::getInstance(false) != 0) LogBrowserDialog::getInstance(true)->close();

    if(searchDock->isFloating())
        searchDock->hide();
    if(metarDock->isFloating())
        metarDock->hide();
    if(metarDecoderDock->isFloating())
        metarDecoderDock->hide();
    if(friendsDock->isFloating())
        friendsDock->hide();

    mapScreen->glWidget->clientSelection->close();
}

void Window::on_metarEdit_textChanged(const QString& text) {
    if(text.length() < 2) {
        metarTimer.stop();
        metarModel.setData(QList<Airport*>());
        metarList->reset();
        return;
    }

    metarTimer.start(500);
}

void Window::on_btnRefreshMetar_clicked() {
    metarModel.refresh();
}

void Window::updateMetars() {
    if(metarEdit->text().length() < 2)
        return;

    metarTimer.stop();
    MetarSearchVisitor *visitor = new MetarSearchVisitor(metarEdit->text());
    NavData::getInstance()->accept(visitor); // search airports only

    metarModel.setData(visitor->airports());
    delete visitor;

    metarList->reset();
}

void Window::friendClicked(const QModelIndex& index) {
    friendsModel.modelClicked(friendsSortModel->mapToSource(index));
}

void Window::friendDoubleClicked(const QModelIndex& index) {
    friendsModel.modelDoubleClicked(friendsSortModel->mapToSource(index));
}

void Window::metarDoubleClicked(const QModelIndex& index) {
    metarModel.modelClicked(metarSortModel->mapToSource(index));
}

void Window::metarDockMoved(Qt::DockWidgetArea area) {
    updateTitlebarAfterMove(area, metarDock);
}

void Window::searchDockMoved(Qt::DockWidgetArea area) {
    updateTitlebarAfterMove(area, searchDock);
}

void Window::metarDecoderDockMoved(Qt::DockWidgetArea area) {
    updateTitlebarAfterMove(area, metarDecoderDock);
}

void Window::friendsDockMoved(Qt::DockWidgetArea area) {
    updateTitlebarAfterMove(area, friendsDock);
}

void Window::updateTitlebarAfterMove(Qt::DockWidgetArea area, QDockWidget *dock) {
    switch(area) {
    case Qt::LeftDockWidgetArea:
    case Qt::RightDockWidgetArea:
        // set horizontal title bar
        dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        break;
    case Qt::TopDockWidgetArea:
    case Qt::BottomDockWidgetArea:
        // set vertical title bar
        dock->setFeatures(searchDock->features() | QDockWidget::DockWidgetVerticalTitleBar);
        break;
    }
}

//void Window::checkForUpdates() {
//    versionChecker = new QHttp(this);
//    connect(versionChecker, SIGNAL(done(bool)), this, SLOT(versionDownloaded(bool)));

//    QString downloadUrl = "http://qutescoop.svn.sourceforge.net/svnroot/qutescoop/trunk/QuteScoop/version.txt";

//    if(Settings::sendVersionInformation()) {
//        // append platform, version and preferred network information to the download link
//        QString urlArgs = QString("?%1&%2&").arg(VERSION_NUMBER).arg(Settings::downloadNetwork());
//        #ifdef Q_WS_WIN
//            urlArgs += "w";
//        #endif
//        #ifdef Q_WS_MAC
//            urlArgs += "m";
//        #endif
//        #ifdef Q_WS_X11
//            urlArgs += "l";
//        #endif
//        downloadUrl += urlArgs;
//    }

//    qDebug() << "Checking for new version on" << downloadUrl;
//    QUrl url(downloadUrl);
//    versionChecker->setHost(url.host(), url.port() != -1 ? url.port() : 80);
//    Settings::applyProxySetting(versionChecker);

//    if (!url.userName().isEmpty())
//        versionChecker->setUser(url.userName(), url.password());

//    QString querystr = url.path() + "?" + url.encodedQuery();
//    versionBuffer = new QBuffer;
//    versionBuffer->open(QBuffer::ReadWrite);
//    versionChecker->get(querystr, versionBuffer);
//}

//void Window::versionDownloaded(bool error) {
//    if(!error) {
//        // compare downloaded version with my own
//        versionBuffer->seek(0);
//        if(versionBuffer->canReadLine()) {
//            QString newVersion = QString(versionBuffer->readLine()).trimmed();
//            QString myVersion = VERSION_NUMBER;

//            // if downloaded is greater, see if we told about new version
//            if(myVersion < newVersion) {
//                if(newVersion != Settings::updateVersionNumber()) {
//                    // tell user that there is a newer version
//                    QMessageBox::information(this, tr("New Version Available"),
//                            QString("A new version of QuteScoop is available! Visit the ")
//                            + "<a href='http://sourceforge.net/projects/qutescoop'>sourceforge.net/projects/qutescoop</a> for more information.<br><br>"
//                            + "You are using: " + VERSION_NUMBER + "<br>"
//                            + "New version is: " + newVersion);

//                    // remember that we told about new version
//                    Settings::setUpdateVersionNumber(newVersion);
//                }
//            }
//        }
//    }
//}

void Window::updateMetarDecoder(const QString& airport, const QString& decodedText) {
    qDebug() << "Window::updateMetarDecoder()";
    metarDecoderDock->setWindowTitle("METAR for " + airport);
    metarText->setText(decodedText);
    metarDecoderDock->show();
    metarDecoderDock->raise();
    metarDecoderDock->activateWindow(); // ?? it gets on top only after the second click from AirportDialog...
    metarDecoderDock->setFocus(); // Don't understand how I can bring this nasty on top of all other. A simple click on the titlebar and it is done.
    qDebug() << "Window::updateMetarDecoder() -- finished";
}

void Window::downloadWatchdogTriggered() {
    downloadWatchdog.stop();
    Whazzup::getInstance()->setStatusLocation(Settings::statusLocation());
    GuiMessages::errorUserAttention("Failed to download network data for a while. Maybe a Whazzup location went offline. I try to get the Network Status again.",
                                   "Whazzup-download failed.");
}

void Window::setEnableBookedAtc(bool enable) {
    actionBookedAtc->setEnabled(enable);
}

void Window::performWarp(bool forceUseDownloaded) {
    editPredictTimer.stop();

    QDateTime warpToTime = dateTimePredict->dateTime();
    if(cbUseDownloaded->isChecked() || forceUseDownloaded) { // use downloaded Whazzups for (past) replay
        qDebug() << "Window::performWarp() Looking for downloaded Whazzups";
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::getInstance()->getDownloadedWhazzups();
        for (int i = downloaded.size() - 1; i > -1; i--) {
            if((downloaded[i].first <= warpToTime) || (i == 0)) {
                // only if different
                if (downloaded[i].first != Whazzup::getInstance()->realWhazzupData().whazzupTime) {
                    // disconnect to inhibit update because will be updated later
                    disconnect(Whazzup::getInstance(), SIGNAL(newData(bool)), mapScreen->glWidget, SLOT(newWhazzupData(bool)));
                    disconnect(Whazzup::getInstance(), SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));

                    Whazzup::getInstance()->fromFile(downloaded[i].second);

                    connect(Whazzup::getInstance(), SIGNAL(newData(bool)), mapScreen->glWidget, SLOT(newWhazzupData(bool)));
                    connect(Whazzup::getInstance(), SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));
                }
                break;
            }
        }
    }
    Whazzup::getInstance()->setPredictedTime(warpToTime);
}

void Window::on_cbUseDownloaded_toggled(bool checked) {
    qDebug() << "Window::cbUseDownloaded_toggled()" << checked;
    if(!checked) {
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::getInstance()->getDownloadedWhazzups();
        if(!downloaded.isEmpty())
            Whazzup::getInstance()->fromFile(downloaded.last().second);
        cbOnlyUseDownloaded->setChecked(false); // uncheck when not using downloaded at all
    }
    performWarp(true);
}
void Window::on_cbOnlyUseDownloaded_toggled(bool checked) {
    if(checked) // if newly selected, set dateTime to valid Whazzup
        on_dateTimePredict_dateTimeChanged(dateTimePredict->dateTime());
}

void Window::on_tbDisablePredict_clicked() {
    qDebug() << "Window::tbDisablePredict_clicked()";
    this->on_actionPredict_toggled(false);
}

void Window::on_actionPredict_toggled(bool enabled) {
    if(enabled) {
        dateTimePredict_old = QDateTime::currentDateTimeUtc()
                .addSecs(- QDateTime::currentDateTimeUtc().time().second()); // remove seconds
        dateTimePredict->setDateTime(dateTimePredict_old);
        framePredict->show();
    } else {
        tbRunPredict->setChecked(false);
        cbUseDownloaded->setChecked(false);
        Whazzup::getInstance()->setPredictedTime(QDateTime()); // remove time warp
        framePredict->hide();
        widgetRunPredict->hide();
    }
}

void Window::on_tbRunPredict_toggled(bool checked) {
    if(checked) {
        dateTimePredict->setEnabled(false);
        widgetRunPredict->show();
        if(!Whazzup::getInstance()->getPredictedTime().isValid())
            performWarp();
        runPredictTimer.start(1000);
    } else {
        runPredictTimer.stop();
        widgetRunPredict->hide();
        dateTimePredict->setEnabled(true);
    }
}

void Window::runPredict() {
    runPredictTimer.stop();
    QDateTime to;

    if (dsRunPredictStep->value() == 0) // real time selected
        to = QDateTime::currentDateTimeUtc();
    else
        to = Whazzup::getInstance()->getPredictedTime().addSecs(
                static_cast<int>(dsRunPredictStep->value()*60));

    // when only using downloaded Whazzups, select the next available
    if(cbOnlyUseDownloaded->isChecked()) {
        qDebug() << "Window::runPredict() restricting Warp target to downloaded Whazzups";
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::getInstance()->getDownloadedWhazzups();
        for (int i = 0; i < downloaded.size(); i++) {
            if((downloaded[i].first >= to) || (i == downloaded.size() - 1)) {
                to = downloaded[i].first;
                break;
            }
        }
    }

    // setting dateTimePredict without "niceify"
    disconnect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));
    dateTimePredict->setDateTime(to);
    connect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));

    performWarp();
    runPredictTimer.start(static_cast<int>(spinRunPredictInterval->value() * 1000));
}

void Window::on_dateTimePredict_dateTimeChanged(QDateTime dateTime) {
    // some niceify on the default behaviour, making the sections depend on each other
    // + only allow selecting downloaded Whazzups if respective option is selected
    disconnect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));
    editPredictTimer.stop();

    // make year change if M 12+ or 0-
    if ((dateTimePredict_old.date().month() == 12)
        && (dateTime.date().month() == 1))
        dateTime = dateTime.addYears(1);
    if ((dateTimePredict_old.date().month() == 1)
        && (dateTime.date().month() == 12))
        dateTime = dateTime.addYears(-1);

    // make month change if d lastday+ or 0-
    if ((dateTimePredict_old.date().day() == dateTimePredict_old.date().daysInMonth())
        && (dateTime.date().day() == 1)) {
        dateTime = dateTime.addMonths(1);
    }
    if ((dateTimePredict_old.date().day() == 1)
        && (dateTime.date().day() == dateTime.date().daysInMonth())) {
        dateTime = dateTime.addMonths(-1);
        dateTime = dateTime.addDays( // compensate for month lengths
                dateTime.date().daysInMonth()
                - dateTimePredict_old.date().daysInMonth());
    }

    // make day change if h 23+ or 00-
    if ((dateTimePredict_old.time().hour() == 23)
        && (dateTime.time().hour() == 0))
        dateTime = dateTime.addDays(1);
    if ((dateTimePredict_old.time().hour() == 0)
        && (dateTime.time().hour() == 23))
        dateTime = dateTime.addDays(-1);

    // make hour change if m 59+ or 0-
    if ((dateTimePredict_old.time().minute() == 59)
        && (dateTime.time().minute() == 0))
        dateTime = dateTime.addSecs(60 * 60);
    if ((dateTimePredict_old.time().minute() == 0)
        && (dateTime.time().minute() == 59))
        dateTime = dateTime.addSecs(-60 * 60);

    // when only using downloaded Whazzups, select the next available
    if(cbOnlyUseDownloaded->isChecked()) {
        qDebug() << "Window::on_dateTimePredict_dateTimeChanged()"
                 << "restricting Warp target to downloaded Whazzups";
        QList<QPair<QDateTime, QString> > downloaded =
                Whazzup::getInstance()->getDownloadedWhazzups();
        if (dateTime > dateTimePredict_old) { // selecting a later date
            for (int i = 0; i < downloaded.size(); i++) {
                if((downloaded[i].first >= dateTime) || (i == downloaded.size() - 1)) {
                    dateTime = downloaded[i].first;
                    break;
                }
            }
        } else { // selecting an earlier date
            for (int i = downloaded.size() - 1; i > -1; i--) {
                if((downloaded[i].first <= dateTime) || (i == 0)) {
                    dateTime = downloaded[i].first;
                    break;
                }
            }
        }
    }

    dateTimePredict_old = dateTime;

    if(dateTime.isValid() && (dateTime != dateTimePredict->dateTime()))
        dateTimePredict->setDateTime(dateTime);

    connect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));
    editPredictTimer.start(1000);
}

void Window::on_actionRecallMapPosition_triggered(){
    mapScreen->glWidget->restorePosition(1);
}

void Window::on_actionRecallMapPosition7_triggered(){
    mapScreen->glWidget->restorePosition(7);
}

void Window::on_actionRecallMapPosition6_triggered(){
    mapScreen->glWidget->restorePosition(6);
}

void Window::on_actionRecallMapPosition5_triggered(){
    mapScreen->glWidget->restorePosition(5);
}

void Window::on_actionRecallMapPosition4_triggered(){
    mapScreen->glWidget->restorePosition(4);
}

void Window::on_actionRecallMapPosition3_triggered(){
    mapScreen->glWidget->restorePosition(3);
}

void Window::on_actionRecallMapPosition2_triggered(){
    mapScreen->glWidget->restorePosition(2);
}

void Window::on_actionRememberPosition_triggered() {
    mapScreen->glWidget->rememberPosition(1);
}

void Window::on_actionRememberMapPosition7_triggered(){
    mapScreen->glWidget->rememberPosition(7);
}

void Window::on_actionRememberMapPosition6_triggered(){
    mapScreen->glWidget->rememberPosition(6);
}

void Window::on_actionRememberMapPosition5_triggered(){
    mapScreen->glWidget->rememberPosition(5);
}

void Window::on_actionRememberMapPosition4_triggered(){
    mapScreen->glWidget->rememberPosition(4);
}

void Window::on_actionRememberMapPosition3_triggered(){
    mapScreen->glWidget->rememberPosition(3);
}

void Window::on_actionRememberMapPosition2_triggered(){
    mapScreen->glWidget->rememberPosition(2);
}

void Window::on_actionMoveLeft_triggered(){
    mapScreen->glWidget->scrollBy(-1, 0);
}

void Window::on_actionMoveRight_triggered(){
    mapScreen->glWidget->scrollBy(1, 0);
}

void Window::on_actionMoveUp_triggered(){
    mapScreen->glWidget->scrollBy(0, -1);
}

void Window::on_actionMoveDown_triggered(){
    mapScreen->glWidget->scrollBy(0, 1);
}
void Window::on_tbZoomIn_clicked(){
    mapScreen->glWidget->zoomIn(.6);
}
void Window::on_tbZoomOut_clicked(){
    mapScreen->glWidget->zoomIn(-.6);
}
// we use this to catch right-clicks on the buttons
void Window::on_tbZoomOut_customContextMenuRequested(QPoint pos){
    mapScreen->glWidget->zoomTo(2.);
}
void Window::on_tbZoomIn_customContextMenuRequested(QPoint pos){
    mapScreen->glWidget->zoomTo(2.);
}
void Window::on_actionZoomReset_triggered(){
    mapScreen->glWidget->zoomTo(2.);
}

void Window::shootScreenshot() {
    QString filename = Settings::applicationDataDirectory(
            QString("screenshots/%1_%2")
            .arg(Settings::downloadNetwork())
            .arg(Whazzup::getInstance()->whazzupData().whazzupTime.toString("yyyyMMdd-HHmmss")));

    if (Settings::screenshotMethod() == 0)
        QPixmap::grabWindow(mapScreen->glWidget->winId()).save(QString("%1.%2").arg(filename, Settings::screenshotFormat()),
                                                    Settings::screenshotFormat().toAscii());
    else if (Settings::screenshotMethod() == 1)
        mapScreen->glWidget->renderPixmap().save(QString("%1.%2").arg(filename, Settings::screenshotFormat()),
                                      Settings::screenshotFormat().toAscii(), true);
    else if (Settings::screenshotMethod() == 2)
        mapScreen->glWidget->grabFrameBuffer(true).save(QString("%1.%2").arg(filename, Settings::screenshotFormat()),
                                             Settings::screenshotFormat().toAscii());
    qDebug() << "Window::shootScreenshot()" << QString("%1.png").arg(filename); //fixme
}

void Window::on_actionShowRoutes_triggered(bool checked) {
    GuiMessages::message(QString("toggled routes [%1]").arg(checked? "on": "off"), "routeToggle");
    foreach(Airport *a, NavData::getInstance()->airports.values()) // synonym to "toggle routes" on all airports
        a->showFlightLines = checked;
    if (!checked) // when disabled, this shall clear all routes
        foreach (Pilot *p, Whazzup::getInstance()->whazzupData().allPilots())
            p->showDepDestLine = false;

    // adjust the "plot route" tick in dialogs
    if (AirportDetails::getInstance(false) != 0)
        AirportDetails::getInstance(true)->refresh();
    if (PilotDetails::getInstance(false) != 0)
        PilotDetails::getInstance(true)->refresh();

    // map update
    mapScreen->glWidget->createPilotsList();
    mapScreen->glWidget->updateGL();
    //glWidget->newWhazzupData(); // complete update, but (should be) unnecessary
}

void Window::on_actionShowWaypoints_triggered(bool checked) {
    Settings::setShowUsedWaypoints(checked);
    mapScreen->glWidget->createPilotsList();
    mapScreen->glWidget->updateGL();
}

void Window::startWindDecoding(bool error)
{
    qDebug() << "Window::startWindDecoding -- WindData downloaded";
    if(windDataBuffer == 0) return;

    if(error)
    {
        GuiMessages::criticalUserInteraction(windDataDownloader->errorString() , "WindData download");
        return;
    }

    windDataBuffer->seek(0);

    QString data = QString(windDataBuffer->readAll());
    WindData::getInstance()->setRawData(data);

    WindData::getInstance()->decodeData();
}

void Window::allSectorsChanged(bool state)
{
    if(actionDisplayAllSectors->isChecked() !=  state)
    {
        actionDisplayAllSectors->setChecked( state);
    }

    mapScreen->toggleSectorChanged(state);

    Settings::setShowAllSectors(state);
    mapScreen->glWidget->displayAllSectors(state);

}

void Window::startCloudDownload()
{
    qDebug() << "Window::startCloudDownload -- prepare Download";
    cloudTimer.stop();

    /*if(!Settings::downloadClouds()){
        mapScreen->glWidget->cloudsAvaliable = false;
        return;
    }*/

    FileReader file(Settings::applicationDataDirectory("data/cloudmirrors.dat"));

    bool hiResMode = false;
    QList<QString> loResMirrors;
    QList<QString> hiResMirrors;

    while(!file.atEnd())
    {
        QString line = file.nextLine();
        if(line.startsWith(";")) continue;
        if(line.startsWith("[2048px]")) {
            hiResMode = false;
            continue;
        }
        if(line.startsWith("[4096px]")){
            hiResMode = true;
            continue;
        }

        if(!hiResMode) loResMirrors.append(line);
        if(hiResMode) hiResMirrors.append(line);
    }

    QUrl url;
    if(Settings::useHightResClouds()){
        if (!hiResMirrors.isEmpty())
            url.setUrl(hiResMirrors[qrand() % hiResMirrors.size()]);
    }
    else {
        if (!loResMirrors.isEmpty())
            url.setUrl(loResMirrors[qrand() % loResMirrors.size()]);
    }
    if(cloudDownloader != 0) cloudDownloader = 0;
    cloudDownloader = new QHttp(this);

    cloudDownloader->setHost(url.host());
    connect(cloudDownloader, SIGNAL(done(bool)), this, SLOT(cloudDownloadFinished(bool)));

    cloudBuffer = new QBuffer;
    cloudBuffer->open(QBuffer::ReadWrite);

    //cloudDownloader->abort();
    cloudDownloader->get(url.path(), cloudBuffer);

    qDebug() << "Window::startCloudDownload -- Download started from " << url.toString();
}

void Window::cloudDownloadFinished(bool error)
{
    qDebug() << "Window::cloudDownloadFinished -- download finished";
    disconnect(cloudDownloader, SIGNAL(done(bool)), this, SLOT(cloudDownloadFinished(bool)));
    if(cloudBuffer == 0)
        return;

    if(error) {
        GuiMessages::criticalUserInteraction(cloudDownloader->errorString(), "cloudlayer download error:");
        return;
    }

    cloudBuffer->seek(0);
    QImage cloudlayer;
    cloudlayer.load(cloudBuffer, "JPG");
    cloudlayer.save(Settings::applicationDataDirectory("textures/clouds/clouds.jpg"), "JPG");
    qDebug() << "Window::cloudDownloadFinished -- clouds.jpg saved  here:" << Settings::applicationDataDirectory("textures/clouds/");

    cloudTimer.start(12600000); //start download in 3,5 h again
    mapScreen->glWidget->cloudsAvaliable = true;
    mapScreen->glWidget->useClouds();
}

void Window::on_actionHighlight_Friends_tiggered(bool checked){
    Settings::setHighlightFriends(checked);
    pb_highlightFriends->setChecked(checked);
    if(checked == false) mapScreen->glWidget->destroyFriendHightlighter();
    mapScreen->glWidget->updateGL();
}

void Window::on_pb_highlightFriends_toggled(bool checked){
    Settings::setHighlightFriends(checked);
    actionHighlight_Friends->setChecked(checked);
    if(checked == false) mapScreen->glWidget->destroyFriendHightlighter();
    mapScreen->glWidget->updateGL();
}

void Window::openSectorView(){
    // SectorView not committed yet?
//    Sectorview::getInstance(true, this)->show();
//    Sectorview::getInstance(true)->raise();
//    Sectorview::getInstance(true)->activateWindow();
//    Sectorview::getInstance(true)->setFocus();
}





