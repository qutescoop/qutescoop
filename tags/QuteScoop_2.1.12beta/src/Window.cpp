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
#include "SectorView.h"

// singleton instance
Window *windowInstance = 0;

Window* Window::instance(bool createIfNoInstance) {
    if(windowInstance == 0 && createIfNoInstance)
        windowInstance = new Window();
    return windowInstance;
}

Window::Window(QWidget *parent) :
    QMainWindow(parent) {
    if(Settings::resetOnNextStart())
        QSettings().clear();

    setupUi(this);

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
    _progressBar = new QProgressBar(statusbar);
    _progressBar->setMaximumWidth(200);
    _progressBar->hide();
    _lblStatus = new QLabel(statusbar);
    statusbar->addWidget(_lblStatus, 5);
    statusbar->addWidget(_progressBar, 3);
    statusbar->addPermanentWidget(tbZoomIn, 0);
    statusbar->addPermanentWidget(tbZoomOut, 0);

    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(actionToggleFullscreen, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));
    connect(actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
    connect(actionPlanFlight, SIGNAL(triggered()), this, SLOT(openPlanFlight()));
    connect(actionBookedAtc, SIGNAL(triggered()), this, SLOT(openBookedAtc()));
    connect(actionListClients, SIGNAL(triggered()), this, SLOT(openListClients()));
    connect(actionSectorview, SIGNAL(triggered()), this, SLOT(openSectorView()));

    Whazzup *whazzup = Whazzup::instance();
    connect(actionDownload, SIGNAL(triggered()), whazzup, SLOT(download()));

    // these 2 get disconnected and connected again to inhibit unnecessary updates:
    connect(whazzup, SIGNAL(newData(bool)), mapScreen->glWidget, SLOT(newWhazzupData(bool)));
    connect(whazzup, SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));

    searchResult->setModel(&_modelSearchResult);
    connect(searchResult, SIGNAL(doubleClicked(const QModelIndex&)),
            &_modelSearchResult, SLOT(modelDoubleClicked(const QModelIndex&)));
    connect(searchResult, SIGNAL(clicked(const QModelIndex&)),
            &_modelSearchResult, SLOT(modelClicked(const QModelIndex&)));
    searchResult->sortByColumn(0, Qt::AscendingOrder);

    _sortmodelMetar = new QSortFilterProxyModel;
    _sortmodelMetar->setDynamicSortFilter(true);
    _sortmodelMetar->setSourceModel(&_metarModel);
    metarList->setModel(_sortmodelMetar);

    connect(metarList, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(metarDoubleClicked(const QModelIndex&)));
    connect(metarList, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(metarDoubleClicked(const QModelIndex&)));
    connect(metarList->header(), SIGNAL(sectionClicked(int)),
            metarList, SLOT(sortByColumn(int)));
    metarList->sortByColumn(0, Qt::AscendingOrder);

    _sortmodelFriends = new QSortFilterProxyModel;
    _sortmodelFriends->setDynamicSortFilter(true);
    _sortmodelFriends->setSourceModel(&_modelFriends);
    friendsList->setModel(_sortmodelFriends);

    connect(friendsList, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(friendDoubleClicked(const QModelIndex&)));
    connect(friendsList, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(friendClicked(const QModelIndex&)));
    connect(friendsList->header(), SIGNAL(sectionClicked(int)),
            friendsList, SLOT(sortByColumn(int)));
    metarList->sortByColumn(0, Qt::AscendingOrder);

    connect(&_timerSearch, SIGNAL(timeout()), this, SLOT(performSearch()));
    connect(&_timerMetar, SIGNAL(timeout()), this, SLOT(updateMetars()));
    connect(&_timerWhazzup, SIGNAL(timeout()),
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

    // Forecast / Predict settings
    framePredict->hide();
    _timerEditPredict.stop();
    connect(&_timerEditPredict, SIGNAL(timeout()), this, SLOT(performWarp()));
    _timerRunPredict.stop();
    connect(&_timerRunPredict, SIGNAL(timeout()), this, SLOT(runPredict()));
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


    connect(&_timerCloud, SIGNAL(timeout()), this, SLOT(startCloudDownload()));

    if(Settings::showClouds())
        startCloudDownload();

    //LogBrowser
#ifdef QT_NO_DEBUG_OUTPUT
    menuView->removeAction(actionDebugLog);
    qDebug() << "Window::Window() Debug Log Browser deactivated due to QT_NO_DEBUG_OUTPUT";
#endif

    qDebug() << "Window::Window() connecting me as GuiMessages listener";
    GuiMessages::instance()->addProgressBar(_progressBar, true);
    GuiMessages::instance()->addStatusLabel(_lblStatus, false);
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
    QFile file(qApp->applicationDirPath() + "/README.html");
    if (file.open(QIODevice::ReadOnly))
        QMessageBox::about(this, tr("About QuteScoop"), file.readAll());
    file.close();
}

void Window::whazzupDownloaded(bool isNew) {
    qDebug() << "Window::whazzupDownloaded() isNew =" << isNew;
    const WhazzupData &realdata = Whazzup::instance()->realWhazzupData();
    const WhazzupData &data = Whazzup::instance()->whazzupData();

    QString msg = QString(tr("%1%2 - %3: %4 clients"))
                  .arg(Settings::downloadNetworkName())
                  .arg(Whazzup::instance()->predictedTime().isValid()
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

    if (Whazzup::instance()->predictedTime().isValid()) {
        framePredict->show();
        dateTimePredict->setDateTime(Whazzup::instance()->predictedTime());
        if(isNew) {
            // recalculate prediction on new data arrived
            if(data.predictionBasedOnTime != realdata.whazzupTime
                || data.predictionBasedOnBookingsTime != realdata.bookingsTime)
                Whazzup::instance()->setPredictedTime(dateTimePredict->dateTime());
        }
    }

    if(isNew) {
        mapScreen->glWidget->clientSelection->close();
        mapScreen->glWidget->clientSelection->clearObjects();

        performSearch();

        if (AirportDetails::instance(false) != 0) {
            if (AirportDetails::instance()->isVisible())
                AirportDetails::instance()->refresh();
            else // not visible -> delete it...
                AirportDetails::instance()->destroyInstance();
        }
        if (PilotDetails::instance(false) != 0) {
            if (PilotDetails::instance()->isVisible())
                PilotDetails::instance()->refresh();
            else // not visible -> delete it...
                PilotDetails::instance()->destroyInstance();
        }
        if (ControllerDetails::instance(false) != 0) {
            if (ControllerDetails::instance()->isVisible())
                ControllerDetails::instance()->refresh();
            else // not visible -> delete it...
                ControllerDetails::instance()->destroyInstance();
        }

        if (ListClientsDialog::instance(false) != 0) {
            if (ListClientsDialog::instance()->isVisible())
                ListClientsDialog::instance()->refresh();
            else // not visible -> delete it...
                ListClientsDialog::instance()->destroyInstance();
        }

        if(realdata.bookingsTime.isValid()) {
            if (BookedAtcDialog::instance(false) != 0) {
                if (BookedAtcDialog::instance()->isVisible())
                    BookedAtcDialog::instance()->refresh();
                else // not visible -> delete it...
                    BookedAtcDialog::instance()->destroyInstance();
            }
        }

        refreshFriends();

        if (Settings::shootScreenshots())
            shootScreenshot();
    }
    _timerWhazzup.stop();
    if(Settings::downloadPeriodically())
        _timerWhazzup.start(Settings::downloadInterval() * 60 * 1000 * 4);

    qDebug() << "Window::whazzupDownloaded() -- finished";
}

/**
 restore saved states
 **/
void Window::restore() {
    if (!Settings::savedSize().isNull())     resize(Settings::savedSize());
    if (!Settings::savedPosition().isNull()) move(Settings::savedPosition());
    if (!Settings::savedGeometry().isNull()) restoreGeometry(Settings::savedGeometry());
    if (!Settings::savedState().isNull())    restoreState(Settings::savedState());
    if (Settings::maximized())               showMaximized();
    show();
}

void Window::refreshFriends() {
    // update friends list
    FriendsVisitor *visitor = new FriendsVisitor();
    Whazzup::instance()->whazzupData().accept(visitor);
    _modelFriends.setData(visitor->result());
    delete visitor;
    friendsList->reset();
}

void Window::openPreferences() {
    PreferencesDialog::instance(true, this);
//    if (!Settings::preferencesDialogSize().isNull())
//        PreferencesDialog::instance()->resize(Settings::preferencesDialogSize());
    if (!Settings::preferencesDialogPos().isNull())
        PreferencesDialog::instance()->move(Settings::preferencesDialogPos());
    if (!Settings::preferencesDialogGeometry().isNull())
        PreferencesDialog::instance()->restoreGeometry(Settings::preferencesDialogGeometry());

    PreferencesDialog::instance()->show();
    PreferencesDialog::instance()->raise();
    PreferencesDialog::instance()->activateWindow();
    PreferencesDialog::instance()->setFocus();
}

void Window::openPlanFlight() {
    PlanFlightDialog::instance(true, this)->show();
    PlanFlightDialog::instance()->raise();
    PlanFlightDialog::instance()->activateWindow();
    PlanFlightDialog::instance()->setFocus();

//    if (!Settings::planFlightDialogSize().isNull())
//        PlanFlightDialog::instance()->resize(Settings::planFlightDialogSize());
    if (!Settings::planFlightDialogPos().isNull())
        PlanFlightDialog::instance()->move(Settings::planFlightDialogPos());
    if (!Settings::planFlightDialogGeometry().isNull())
        PlanFlightDialog::instance()->restoreGeometry(Settings::planFlightDialogGeometry());
}

void Window::openBookedAtc() {
    BookedAtcDialog::instance(true, this)->show();
    BookedAtcDialog::instance()->raise();
    BookedAtcDialog::instance()->activateWindow();
    BookedAtcDialog::instance()->setFocus();
}

void Window::openListClients() {
    ListClientsDialog::instance(true, this)->show();
    ListClientsDialog::instance()->raise();
    ListClientsDialog::instance()->activateWindow();
    //ListClientsDialog::instance()->setFocus();
}

void Window::on_actionDebugLog_triggered() {
    LogBrowserDialog::instance(true, this)->show();
    LogBrowserDialog::instance()->raise();
    LogBrowserDialog::instance()->activateWindow();
    LogBrowserDialog::instance()->setFocus();
}

void Window::on_searchEdit_textChanged(const QString& text) {
    if(text.length() < 2) {
        _timerSearch.stop();
        _modelSearchResult.setData(QList<MapObject*>());
        searchResult->reset();
        return;
    }

    _timerSearch.start(400);
}

void Window::performSearch() {
    if(searchEdit->text().length() < 2)
        return;

    _timerSearch.stop();
    SearchVisitor *visitor = new SearchVisitor(searchEdit->text());
    NavData::instance()->accept(visitor);
    Whazzup::instance()->whazzupData().accept(visitor);

    _modelSearchResult.setData(visitor->result());
    delete visitor;

    searchResult->reset();
}

void Window::closeEvent(QCloseEvent *event) {
    Settings::saveState(saveState());
    Settings::saveGeometry(saveGeometry()); // added this 'cause maximized wasn't saved
    Settings::saveSize(size()); // readded as Mac OS had problems with geometry only
    Settings::savePosition(pos());
    Settings::saveMaximized(isMaximized());
    on_actionHideAllWindows_triggered();
    if (Settings::rememberMapPositionOnClose())
        mapScreen->glWidget->rememberPosition(9);
    event->accept();
}

void Window::on_actionHideAllWindows_triggered() {
    if (PilotDetails::instance(false) != 0) PilotDetails::instance()->close();
    if (ControllerDetails::instance(false) != 0) ControllerDetails::instance()->close();
    if (AirportDetails::instance(false) != 0) AirportDetails::instance()->close();
    if (PreferencesDialog::instance(false) != 0) PreferencesDialog::instance()->close();
    if (PlanFlightDialog::instance(false) != 0) PlanFlightDialog::instance()->close();
    if (BookedAtcDialog::instance(false) != 0) BookedAtcDialog::instance()->close();
    if (ListClientsDialog::instance(false) != 0) ListClientsDialog::instance()->close();
    if (LogBrowserDialog::instance(false) != 0) LogBrowserDialog::instance()->close();

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
        _timerMetar.stop();
        _metarModel.setData(QList<Airport*>());
        metarList->reset();
        return;
    }
    _timerMetar.start(500);
}

void Window::on_btnRefreshMetar_clicked() {
    _metarModel.refresh();
}

void Window::updateMetars() {
    if(metarEdit->text().length() < 2)
        return;

    _timerMetar.stop();
    MetarSearchVisitor *visitor = new MetarSearchVisitor(metarEdit->text());
    NavData::instance()->accept(visitor); // search airports only

    _metarModel.setData(visitor->airports());
    delete visitor;

    metarList->reset();
}

void Window::friendClicked(const QModelIndex& index) {
    _modelFriends.modelClicked(_sortmodelFriends->mapToSource(index));
}

void Window::friendDoubleClicked(const QModelIndex& index) {
    _modelFriends.modelDoubleClicked(_sortmodelFriends->mapToSource(index));
}

void Window::metarDoubleClicked(const QModelIndex& index) {
    _metarModel.modelClicked(_sortmodelMetar->mapToSource(index));
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
    default: {}
    }
}

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
    _timerWhazzup.stop();
    Whazzup::instance()->setStatusLocation(Settings::statusLocation());
    GuiMessages::errorUserAttention("Failed to download network data for a while. Maybe a Whazzup location went offline. I try to get the Network Status again.",
                                   "Whazzup-download failed.");
}

void Window::setEnableBookedAtc(bool enable) {
    actionBookedAtc->setEnabled(enable);
}

void Window::performWarp(bool forceUseDownloaded) {
    _timerEditPredict.stop();

    QDateTime warpToTime = dateTimePredict->dateTime();
    if(cbUseDownloaded->isChecked() || forceUseDownloaded) { // use downloaded Whazzups for (past) replay
        qDebug() << "Window::performWarp() Looking for downloaded Whazzups";
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::instance()->downloadedWhazzups();
        for (int i = downloaded.size() - 1; i > -1; i--) {
            if((downloaded[i].first <= warpToTime) || (i == 0)) {
                // only if different
                if (downloaded[i].first != Whazzup::instance()->realWhazzupData().whazzupTime) {
                    // disconnect to inhibit update because will be updated later
                    disconnect(Whazzup::instance(), SIGNAL(newData(bool)), mapScreen->glWidget, SLOT(newWhazzupData(bool)));
                    disconnect(Whazzup::instance(), SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));

                    Whazzup::instance()->fromFile(downloaded[i].second);

                    connect(Whazzup::instance(), SIGNAL(newData(bool)), mapScreen->glWidget, SLOT(newWhazzupData(bool)));
                    connect(Whazzup::instance(), SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));
                }
                break;
            }
        }
    }
    Whazzup::instance()->setPredictedTime(warpToTime);
}

void Window::on_cbUseDownloaded_toggled(bool checked) {
    qDebug() << "Window::cbUseDownloaded_toggled()" << checked;
    if(!checked) {
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::instance()->downloadedWhazzups();
        if(!downloaded.isEmpty())
            Whazzup::instance()->fromFile(downloaded.last().second);
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
    //this->on_actionPredict_toggled(false); // we do this by toggling the menu item
    actionPredict->setChecked(false);
}

void Window::on_actionPredict_toggled(bool enabled) {
    if(enabled) {
        _dateTimePredict_old = QDateTime::currentDateTimeUtc()
                .addSecs(- QDateTime::currentDateTimeUtc().time().second()); // remove seconds
        dateTimePredict->setDateTime(_dateTimePredict_old);
        framePredict->show();
    } else {
        tbRunPredict->setChecked(false);
        cbUseDownloaded->setChecked(false);
        Whazzup::instance()->setPredictedTime(QDateTime()); // remove time warp
        framePredict->hide();
        widgetRunPredict->hide();
    }
}

void Window::on_tbRunPredict_toggled(bool checked) {
    if(checked) {
        dateTimePredict->setEnabled(false);
        widgetRunPredict->show();
        if(!Whazzup::instance()->predictedTime().isValid())
            performWarp();
        _timerRunPredict.start(1000);
    } else {
        _timerRunPredict.stop();
        widgetRunPredict->hide();
        dateTimePredict->setEnabled(true);
    }
}

void Window::runPredict() {
    _timerRunPredict.stop();
    QDateTime to;

    if (dsRunPredictStep->value() == 0) // real time selected
        to = QDateTime::currentDateTimeUtc();
    else
        to = Whazzup::instance()->predictedTime().addSecs(
                static_cast<int>(dsRunPredictStep->value()*60));

    // when only using downloaded Whazzups, select the next available
    if(cbOnlyUseDownloaded->isChecked()) {
        qDebug() << "Window::runPredict() restricting Warp target to downloaded Whazzups";
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::instance()->downloadedWhazzups();
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
    _timerRunPredict.start(static_cast<int>(spinRunPredictInterval->value() * 1000));
}

void Window::on_dateTimePredict_dateTimeChanged(QDateTime dateTime) {
    // some niceify on the default behaviour, making the sections depend on each other
    // + only allow selecting downloaded Whazzups if respective option is selected
    disconnect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));
    _timerEditPredict.stop();

    // make year change if M 12+ or 0-
    if ((_dateTimePredict_old.date().month() == 12)
        && (dateTime.date().month() == 1))
        dateTime = dateTime.addYears(1);
    if ((_dateTimePredict_old.date().month() == 1)
        && (dateTime.date().month() == 12))
        dateTime = dateTime.addYears(-1);

    // make month change if d lastday+ or 0-
    if ((_dateTimePredict_old.date().day() == _dateTimePredict_old.date().daysInMonth())
        && (dateTime.date().day() == 1)) {
        dateTime = dateTime.addMonths(1);
    }
    if ((_dateTimePredict_old.date().day() == 1)
        && (dateTime.date().day() == dateTime.date().daysInMonth())) {
        dateTime = dateTime.addMonths(-1);
        dateTime = dateTime.addDays( // compensate for month lengths
                dateTime.date().daysInMonth()
                - _dateTimePredict_old.date().daysInMonth());
    }

    // make day change if h 23+ or 00-
    if ((_dateTimePredict_old.time().hour() == 23)
        && (dateTime.time().hour() == 0))
        dateTime = dateTime.addDays(1);
    if ((_dateTimePredict_old.time().hour() == 0)
        && (dateTime.time().hour() == 23))
        dateTime = dateTime.addDays(-1);

    // make hour change if m 59+ or 0-
    if ((_dateTimePredict_old.time().minute() == 59)
        && (dateTime.time().minute() == 0))
        dateTime = dateTime.addSecs(60 * 60);
    if ((_dateTimePredict_old.time().minute() == 0)
        && (dateTime.time().minute() == 59))
        dateTime = dateTime.addSecs(-60 * 60);

    // when only using downloaded Whazzups, select the next available
    if(cbOnlyUseDownloaded->isChecked()) {
        qDebug() << "Window::on_dateTimePredict_dateTimeChanged()"
                 << "restricting Warp target to downloaded Whazzups";
        QList<QPair<QDateTime, QString> > downloaded =
                Whazzup::instance()->downloadedWhazzups();
        if (dateTime > _dateTimePredict_old) { // selecting a later date
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

    _dateTimePredict_old = dateTime;

    if(dateTime.isValid() && (dateTime != dateTimePredict->dateTime()))
        dateTimePredict->setDateTime(dateTime);

    connect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));
    _timerEditPredict.start(1000);
}

void Window::on_actionRecallMapPosition9_triggered() {
    mapScreen->glWidget->restorePosition(9);
}

void Window::on_actionRecallMapPosition1_triggered() {
    mapScreen->glWidget->restorePosition(1);
}

void Window::on_actionRecallMapPosition7_triggered() {
    mapScreen->glWidget->restorePosition(7);
}

void Window::on_actionRecallMapPosition6_triggered() {
    mapScreen->glWidget->restorePosition(6);
}

void Window::on_actionRecallMapPosition5_triggered() {
    mapScreen->glWidget->restorePosition(5);
}

void Window::on_actionRecallMapPosition4_triggered() {
    mapScreen->glWidget->restorePosition(4);
}

void Window::on_actionRecallMapPosition3_triggered() {
    mapScreen->glWidget->restorePosition(3);
}

void Window::on_actionRecallMapPosition2_triggered() {
    mapScreen->glWidget->restorePosition(2);
}

void Window::on_actionRememberMapPosition9_triggered() {
    if (Settings::rememberMapPositionOnClose()) {
        if (QMessageBox::question(this, "Startup position will be overridden on close",
                              "You have just set the the startup map position. "
                              "Anyhow this will be overridden on close because "
                              "you have set 'remember startup map position on close' "
                              "in preferences.\n\n"
                              "Do you want to disable 'remember startup map position "
                              "on close' now?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
                == QMessageBox::Yes) {
            Settings::setRememberMapPositionOnClose(false);
            if (PreferencesDialog::instance(false) != 0)
                    PreferencesDialog::instance()->
                            cbRememberMapPositionOnClose->setChecked(false);
        }
    }
    mapScreen->glWidget->rememberPosition(9);
}

void Window::on_actionRememberMapPosition1_triggered() {
    mapScreen->glWidget->rememberPosition(1);
}

void Window::on_actionRememberMapPosition7_triggered() {
    mapScreen->glWidget->rememberPosition(7);
}

void Window::on_actionRememberMapPosition6_triggered() {
    mapScreen->glWidget->rememberPosition(6);
}

void Window::on_actionRememberMapPosition5_triggered() {
    mapScreen->glWidget->rememberPosition(5);
}

void Window::on_actionRememberMapPosition4_triggered() {
    mapScreen->glWidget->rememberPosition(4);
}

void Window::on_actionRememberMapPosition3_triggered() {
    mapScreen->glWidget->rememberPosition(3);
}

void Window::on_actionRememberMapPosition2_triggered() {
    mapScreen->glWidget->rememberPosition(2);
}

void Window::on_actionMoveLeft_triggered() {
    mapScreen->glWidget->scrollBy(-1, 0);
}

void Window::on_actionMoveRight_triggered() {
    mapScreen->glWidget->scrollBy(1, 0);
}

void Window::on_actionMoveUp_triggered() {
    mapScreen->glWidget->scrollBy(0, -1);
}

void Window::on_actionMoveDown_triggered() {
    mapScreen->glWidget->scrollBy(0, 1);
}
void Window::on_tbZoomIn_clicked() {
    mapScreen->glWidget->zoomIn(.6);
}
void Window::on_tbZoomOut_clicked() {
    mapScreen->glWidget->zoomIn(-.6);
}
// we use this to catch right-clicks on the buttons
void Window::on_tbZoomOut_customContextMenuRequested(QPoint pos) {
    Q_UNUSED(pos);
    mapScreen->glWidget->zoomTo(2.);
}
void Window::on_tbZoomIn_customContextMenuRequested(QPoint pos) {
    Q_UNUSED(pos);
    mapScreen->glWidget->zoomTo(2.);
}
void Window::on_actionZoomReset_triggered() {
    mapScreen->glWidget->zoomTo(2.);
}

void Window::shootScreenshot() {
    QString filename = Settings::applicationDataDirectory(
            QString("screenshots/%1_%2")
            .arg(Settings::downloadNetwork())
            .arg(Whazzup::instance()->whazzupData().whazzupTime.toString("yyyyMMdd-HHmmss")));

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
    foreach(Airport *a, NavData::instance()->airports.values()) // synonym to "toggle routes" on all airports
        a->showFlightLines = checked;
    if (!checked) // when disabled, this shall clear all routes
        foreach (Pilot *p, Whazzup::instance()->whazzupData().allPilots())
            p->showDepDestLine = false;

    // adjust the "plot route" tick in dialogs
    if (AirportDetails::instance(false) != 0)
        AirportDetails::instance()->refresh();
    if (PilotDetails::instance(false) != 0)
        PilotDetails::instance()->refresh();

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

void Window::allSectorsChanged(bool state) {
    if(actionDisplayAllSectors->isChecked() !=  state)
    {
        actionDisplayAllSectors->setChecked( state);
    }

    mapScreen->on_sectorsAll_changed(state);

    Settings::setShowAllSectors(state);
    mapScreen->glWidget->displayAllSectors(state);

}

void Window::startCloudDownload() {
    qDebug() << "Window::startCloudDownload -- prepare Download";
    _timerCloud.stop();

    /*if(!Settings::downloadClouds()) {
        mapScreen->glWidget->cloudsAvaliable = false;
        return;
    }*/

    FileReader file(Settings::applicationDataDirectory("data/cloudmirrors.dat"));

    bool hiResMode = false;
    QList<QString> loResMirrors;
    QList<QString> hiResMirrors;

    while(!file.atEnd()) {
        QString line = file.nextLine();
        if(line.startsWith(";")) continue;
        if(line.startsWith("[2048px]")) {
            hiResMode = false;
            continue;
        }
        if(line.startsWith("[4096px]")) {
            hiResMode = true;
            continue;
        }

        if(!hiResMode) loResMirrors.append(line);
        if(hiResMode) hiResMirrors.append(line);
    }

    QUrl url;
    if(Settings::useHightResClouds()) {
        if (!hiResMirrors.isEmpty())
            url.setUrl(hiResMirrors[qrand() % hiResMirrors.size()]);
    } else {
        if (!loResMirrors.isEmpty())
            url.setUrl(loResMirrors[qrand() % loResMirrors.size()]);
    }
    if(_cloudDownloader != 0) _cloudDownloader = 0;
    _cloudDownloader = new QHttp(this);

    _cloudDownloader->setHost(url.host());
    connect(_cloudDownloader, SIGNAL(done(bool)), this, SLOT(cloudDownloadFinished(bool)));

    _cloudBuffer = new QBuffer;
    _cloudBuffer->open(QBuffer::ReadWrite);

    //cloudDownloader->abort();
    _cloudDownloader->get(url.path(), _cloudBuffer);

    qDebug() << "Window::startCloudDownload -- Download started from " << url.toString();
}

void Window::cloudDownloadFinished(bool error) {
    qDebug() << "Window::cloudDownloadFinished -- download finished";
    disconnect(_cloudDownloader, SIGNAL(done(bool)), this, SLOT(cloudDownloadFinished(bool)));
    if(_cloudBuffer == 0)
        return;

    if(error) {
        GuiMessages::criticalUserInteraction(_cloudDownloader->errorString(), "cloudlayer download error:");
        return;
    }

    _cloudBuffer->seek(0);
    QImage cloudlayer;
    cloudlayer.load(_cloudBuffer, "JPG");
    cloudlayer.save(Settings::applicationDataDirectory("textures/clouds/clouds.jpg"), "JPG");
    qDebug() << "Window::cloudDownloadFinished -- clouds.jpg saved  here:"
             << Settings::applicationDataDirectory("textures/clouds/");

    _timerCloud.start(12600000); //start download in 3,5 h again
    mapScreen->glWidget->useClouds();
}

void Window::on_actionHighlight_Friends_triggered(bool checked) {
    Settings::setHighlightFriends(checked);
    pb_highlightFriends->setChecked(checked);
    if (!checked) mapScreen->glWidget->destroyFriendHightlighter();
    mapScreen->glWidget->updateGL();
}

void Window::on_pb_highlightFriends_toggled(bool checked) {
    actionHighlight_Friends->setChecked(checked);
    on_actionHighlight_Friends_triggered(checked);
}

void Window::openSectorView() {
    Sectorview::instance(true, this)->show();
    Sectorview::instance()->raise();
    Sectorview::instance()->activateWindow();
    Sectorview::instance()->setFocus();
}

void Window::on_actionChangelog_triggered() {
    QFile file(qApp->applicationDirPath() + "/CHANGELOG");
    if (file.open(QIODevice::ReadOnly)) {
        QDialog *dlg = new QDialog(this);
        QTextEdit *lbl = new QTextEdit(dlg);
        QDialogButtonBox *btns = new QDialogButtonBox(
                    QDialogButtonBox::Ok,
                    Qt::Horizontal,
                    dlg
        );

        dlg->connect(btns, SIGNAL(accepted()), SLOT(close()));
        lbl->setReadOnly(true);
        lbl->setText(file.readAll());

        dlg->setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));
        dlg->setWindowTitle("Changelog");

        dlg->setLayout(new QVBoxLayout);
        dlg->layout()->addWidget(lbl);
        dlg->layout()->addWidget(btns);

        dlg->setModal(true);
        dlg->showMaximized();
    }
    file.close();
}
