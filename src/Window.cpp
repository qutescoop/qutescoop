/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Window.h"

#include "GLWidget.h"
#include "Net.h"
#include "PilotDetails.h"
#include "ControllerDetails.h"
#include "AirportDetails.h"
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
#include "GuiMessage.h"
#include "SectorView.h"
#include "Platform.h"
#include "MetarDelegate.h"

#include <QModelIndex>

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

    GuiMessages::progress("mainwindow", "Setting up main window...");
    setupUi(this);

    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    setWindowTitle(QString("QuteScoop %1").arg(Platform::version()));

    // apply styleSheet
    if (!Settings::stylesheet().isEmpty()) {
        qDebug() << "Window::() applying styleSheet:" << Settings::stylesheet();
        setStyleSheet(Settings::stylesheet());
    }

    // map (GLWidget)
    mapScreen = new MapScreen(this);
    centralwidget->layout()->addWidget(mapScreen);

    connect(mapScreen, &MapScreen::toggleRoutes, actionShowRoutes, &QAction::trigger);
    connect(mapScreen, &MapScreen::toggleSectors, this, &Window::allSectorsChanged);
    connect(mapScreen, &MapScreen::toggleRouteWaypoints, actionShowWaypoints, &QAction::trigger);
    connect(mapScreen, &MapScreen::toggleInactiveAirports, actionShowInactiveAirports,&QAction::trigger);

    // Status- & ProgressBar
    _progressBar = new QProgressBar(statusbar);
    _progressBar->setMaximumWidth(200);
    _progressBar->hide();
    _lblStatus = new QLabel(statusbar);
    statusbar->addWidget(_lblStatus, 5);
    statusbar->addWidget(_progressBar, 3);
    statusbar->addPermanentWidget(tbZoomIn, 0);
    statusbar->addPermanentWidget(tbZoomOut, 0);

    // actions
    connect(actionAbout, &QAction::triggered, this, &Window::about);
    connect(actionToggleFullscreen, &QAction::triggered, this, &Window::toggleFullscreen);
    connect(actionPreferences, &QAction::triggered, this, &Window::openPreferences);
    connect(actionPlanFlight, &QAction::triggered, this, &Window::openPlanFlight);
    connect(actionBookedAtc, &QAction::triggered, this, &Window::openBookedAtc);
    connect(actionListClients, &QAction::triggered, this, &Window::openListClients);
    connect(actionSectorview, &QAction::triggered, this, &Window::openSectorView);
    actionDisplayAllSectors->setChecked(Settings::showAllSectors());
    connect(actionDisplayAllSectors, &QAction::toggled, this, &Window::allSectorsChanged);
    actionShowInactiveAirports->setChecked(Settings::showInactiveAirports());
    connect(actionShowInactiveAirports, &QAction::toggled, mapScreen->glWidget, &GLWidget::showInactiveAirports);
    pb_highlightFriends->setChecked(Settings::highlightFriends());
    actionHighlight_Friends->setChecked(Settings::highlightFriends());
    setEnableBookedAtc(Settings::downloadBookings());
    actionShowWaypoints->setChecked(Settings::showUsedWaypoints());

    Whazzup *whazzup = Whazzup::instance();
    connect(actionDownload, &QAction::triggered, whazzup, &Whazzup::downloadJson3);

    // these 2 get disconnected and connected again to inhibit unnecessary updates:
    connect(whazzup, &Whazzup::newData, mapScreen->glWidget, &GLWidget::newWhazzupData);
    connect(whazzup, &Whazzup::newData, this, &Window::processWhazzup);

    // search result widget
    searchResult->setModel(&_modelSearchResult);
    connect(searchResult, &QAbstractItemView::clicked, &_modelSearchResult, &SearchResultModel::modelClicked);
    searchResult->sortByColumn(0, Qt::AscendingOrder);

    // METAR widget
    _sortmodelMetar = new QSortFilterProxyModel;
    _sortmodelMetar->setDynamicSortFilter(true);
    _sortmodelMetar->setSourceModel(&_metarModel);

    metarList->setModel(_sortmodelMetar);
    MetarDelegate *metarDelegate = new MetarDelegate();
    metarDelegate->setParent(metarList);
    metarList->setItemDelegate(metarDelegate);
    //metarList->setWordWrap(true); // some say this causes sizeHint() to be called on resize, but it does not

    connect(metarList, &QAbstractItemView::clicked, this, &Window::metarClicked);
    metarList->sortByColumn(0, Qt::AscendingOrder);

    // friends widget
    _sortmodelFriends = new QSortFilterProxyModel;
    _sortmodelFriends->setDynamicSortFilter(true);
    _sortmodelFriends->setSourceModel(&_modelFriends);
    friendsList->setModel(_sortmodelFriends);

    connect(friendsList, &QAbstractItemView::clicked, this, &Window::friendClicked);
    friendsList->sortByColumn(0, Qt::AscendingOrder);

    // debounce input timers
    connect(&_timerSearch, &QTimer::timeout, this, &Window::performSearch);
    connect(&_timerMetar, &QTimer::timeout, this, &Window::updateMetars);

    // Whazzup download timer
    connect(&_timerWhazzup, &QTimer::timeout, this, &Window::downloadWatchdogTriggered);

    // Cloud download timer
    connect(&_timerCloud, &QTimer::timeout, this, &Window::downloadCloud);

    // dock layout
    connect(metarDock, &QDockWidget::dockLocationChanged,
            this, &Window::metarDockMoved);
    connect(searchDock, &QDockWidget::dockLocationChanged,
            this, &Window::searchDockMoved);
    connect(friendsDock, &QDockWidget::dockLocationChanged,
            this, &Window::friendsDockMoved);

    // Forecast / Predict settings
    framePredict->hide();
    _timerEditPredict.stop();
    connect(&_timerEditPredict, &QTimer::timeout, this, &Window::performWarp);
    _timerRunPredict.stop();
    connect(&_timerRunPredict, &QTimer::timeout, this, &Window::runPredict);
    widgetRunPredict->hide();

    connect(dateTimePredict, &QDateTimeEdit::dateTimeChanged, this, &Window::dateTimePredict_dateTimeChanged);

    QFont font = lblWarpInfo->font();
    font.setPointSize(lblWarpInfo->fontInfo().pointSize() - 1);
    lblWarpInfo->setFont(font); //make it a bit smaller than standard text

    font = cbUseDownloaded->font();
    font.setPointSize(cbUseDownloaded->fontInfo().pointSize() - 1);
    cbUseDownloaded->setFont(font); //make it a bit smaller than standard text

    font = cbOnlyUseDownloaded->font();
    font.setPointSize(cbOnlyUseDownloaded->fontInfo().pointSize() - 1);
    cbOnlyUseDownloaded->setFont(font); //make it a bit smaller than standard text

    // GuiMessages
    GuiMessages::instance()->addProgressBar(_progressBar, true);
    GuiMessages::instance()->addStatusLabel(_lblStatus, false);

    GuiMessages::remove("mainwindow");
    qDebug() << "Window::() --finished";
}

Window::~Window() {
}

void Window::restore() {
    qDebug() << "Window::restore() restoring window state, geometry and position";

    if (!Settings::savedSize().isNull())     resize(Settings::savedSize());
    if (!Settings::savedPosition().isNull()) move(Settings::savedPosition());
    if (!Settings::savedGeometry().isNull()) restoreGeometry(Settings::savedGeometry());
    if (!Settings::savedState().isNull())    restoreState(Settings::savedState());
    if (Settings::maximized())               showMaximized();
    else                                     show();

    emit restored();
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
    QMessageBox::about(
        this,
        tr("About QuteScoop"),
        tr(
R"html(
<h3>QuteScoop</h3>

<p>
    QuteScoop is free (libre) software.
    Find downloads, get help, voice your wishes, contribute and join the discussion here:
    <a href="https://github.com/qutescoop/qutescoop">QuteScoop on Github</a>.
</p>

<h3>License</h3>
<p>
    QuteScoop is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
</p>

<p>
    QuteScoop is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
</p>
)html"
        )
    );
}

void Window::processWhazzup(bool isNew) {
    qDebug() << "Window::whazzupDownloaded() isNew =" << isNew;
    const WhazzupData &realdata = Whazzup::instance()->realWhazzupData();
    const WhazzupData &data = Whazzup::instance()->whazzupData();

    QString msg = QString(tr("%1%2 - %3: %4 clients"))
        .arg(
            Settings::downloadNetworkName(),
            Whazzup::instance()->predictedTime.isValid() ? " - <b>W A R P E D</b>  to" : ""
        )
        .arg(data.whazzupTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
            ? QString("today %1").arg(data.whazzupTime.time().toString("HHmm'z'"))
            : data.whazzupTime.toString("ddd MM/dd HHmm'z'"))
        .arg(data.pilots.size() + data.controllers.size());
    GuiMessages::status(msg, "status");

    msg = QString("Whazzup %1, bookings %2 updated")
        .arg(
            realdata.whazzupTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
                ? QString("today %1").arg(realdata.whazzupTime.time().toString("HHmm'z'"))
                : (realdata.whazzupTime.isValid()
                   ? realdata.whazzupTime.toString("ddd MM/dd HHmm'z'")
                   : "never"
                 ),
            realdata.bookingsTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
                ? QString("today %1").arg(realdata.bookingsTime.time().toString("HHmm'z'"))
                : (realdata.bookingsTime.isValid()
                   ? realdata.bookingsTime.toString("ddd MM/dd HHmm'z'")
                   : "never"
                 )
        );
    lblWarpInfo->setText(msg);

    if (Whazzup::instance()->predictedTime.isValid()) {
        framePredict->show();
        dateTimePredict->setDateTime(Whazzup::instance()->predictedTime);
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

void Window::refreshFriends() {
    // update friends list
    FriendsVisitor *visitor = new FriendsVisitor();
    Whazzup::instance()->whazzupData().accept(visitor);
    _modelFriends.setSearchResults(visitor->result());
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
    if (actionPredict->isChecked()) {
        BookedAtcDialog::instance()->setDateTime(dateTimePredict->dateTime().toUTC());
    }
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

void Window::on_searchEdit_textChanged(const QString& text) {
    if(text.length() < 1) {
        _timerSearch.stop();
        _modelSearchResult.setSearchResults(QList<MapObject*>());
        searchResult->reset();
        return;
    }

    _timerSearch.start(400);
}

void Window::performSearch() {
    if(searchEdit->text().length() < 1)
        return;

    _timerSearch.stop();
    SearchVisitor *visitor = new SearchVisitor(searchEdit->text());
    NavData::instance()->accept(visitor);
    Whazzup::instance()->whazzupData().accept(visitor);

    _modelSearchResult.setSearchResults(visitor->result());
    delete visitor;

    searchResult->reset();
}

void Window::closeEvent(QCloseEvent *event) {
    // save window statii
    Settings::saveState(saveState());
    Settings::saveGeometry(saveGeometry()); // added this 'cause maximized wasn't saved
    Settings::saveSize(size()); // readded as Mac OS had problems with geometry only
    Settings::savePosition(pos());
    Settings::saveMaximized(isMaximized());
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

    if(searchDock->isFloating())
        searchDock->hide();
    if(metarDock->isFloating())
        metarDock->hide();
    if(friendsDock->isFloating())
        friendsDock->hide();

    mapScreen->glWidget->clientSelection->close();
}

void Window::on_metarEdit_textChanged(const QString& text) {
    if(text.length() < 1) {
        _timerMetar.stop();
        _metarModel.setAirports(QList<Airport*>());
        metarList->reset();
        return;
    }
    _timerMetar.start(500);
}

void Window::on_btnRefreshMetar_clicked() {
    _metarModel.refresh();
}

void Window::updateMetars() {
    if(metarEdit->text().length() < 1)
        return;

    _timerMetar.stop();
    MetarSearchVisitor *visitor = new MetarSearchVisitor(metarEdit->text());
    NavData::instance()->accept(visitor); // search airports only

    _metarModel.setAirports(visitor->airports());
    delete visitor;

    metarList->reset();
}

void Window::friendClicked(const QModelIndex& index) {
    _modelFriends.modelClicked(_sortmodelFriends->mapToSource(index));
}

void Window::metarClicked(const QModelIndex& index) {
    _metarModel.modelClicked(_sortmodelMetar->mapToSource(index));
}

void Window::metarDockMoved(Qt::DockWidgetArea area) {
    updateTitlebarAfterMove(area, metarDock);
}

void Window::searchDockMoved(Qt::DockWidgetArea area) {
    updateTitlebarAfterMove(area, searchDock);
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

void Window::downloadWatchdogTriggered() {
    _timerWhazzup.stop();
    Whazzup::instance()->setStatusLocation(Settings::statusLocation());
    GuiMessages::errorUserAttention("Failed to download network data for a while. "
                                    "Maybe a Whazzup location went offline. "
                                    "I try to get the Network Status again.",
                                    "Whazzup-download failed.");
}

void Window::setEnableBookedAtc(bool enable) {
    actionBookedAtc->setEnabled(enable);
}

void Window::performWarp() {
    _timerEditPredict.stop();

    QDateTime warpToTime = dateTimePredict->dateTime();
    auto realWhazzupTime = Whazzup::instance()->realWhazzupData().whazzupTime;
    qDebug() << "Window::performWarp() warpToTime=" << warpToTime << " realWhazzupTime=" << realWhazzupTime;
    if(cbUseDownloaded->isChecked() && warpToTime < realWhazzupTime) {
        qDebug() << "Window::performWarp() Looking for downloaded Whazzups";
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::instance()->downloadedWhazzups();
        for (int i = downloaded.size() - 1; i > -1; i--) {
            if((downloaded[i].first <= warpToTime && realWhazzupTime < downloaded[i].first) || (i == 0)) {
                // only if different
                if (downloaded[i].first != realWhazzupTime) {
                    // disconnect to inhibit update because will be updated later
                    disconnect(Whazzup::instance(), &Whazzup::newData, mapScreen->glWidget, &GLWidget::newWhazzupData);
                    disconnect(Whazzup::instance(), &Whazzup::newData, this, &Window::processWhazzup);

                    Whazzup::instance()->fromFile(downloaded[i].second);

                    connect(Whazzup::instance(), &Whazzup::newData, mapScreen->glWidget, &GLWidget::newWhazzupData);
                    connect(Whazzup::instance(), &Whazzup::newData, this, &Window::processWhazzup);
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
        // I currently don't understand why we had this
        //        QList<QPair<QDateTime, QString> > downloaded = Whazzup::instance()->downloadedWhazzups();
        //        if(!downloaded.isEmpty())
        //            Whazzup::instance()->fromFile(downloaded.last().second);
        cbOnlyUseDownloaded->setChecked(false);
    }
    performWarp();
}
void Window::on_cbOnlyUseDownloaded_toggled(bool checked) {
    if(checked) { // if newly selected, set dateTime to valid Whazzup
        dateTimePredict_dateTimeChanged(dateTimePredict->dateTime());
    }
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
        if(!Whazzup::instance()->predictedTime.isValid()) {
            performWarp();
        }
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

    if (dsRunPredictStep->value() == 0) { // real time selected
        to = QDateTime::currentDateTimeUtc();
    } else {
        to = Whazzup::instance()->predictedTime.addSecs(static_cast<int>(dsRunPredictStep->value()*60));
    }

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

    qDebug() << to;

    // setting dateTimePredict without "niceify"
    disconnect(dateTimePredict, &QDateTimeEdit::dateTimeChanged, this, &Window::dateTimePredict_dateTimeChanged);
    dateTimePredict->setDateTime(to);
    connect(dateTimePredict, &QDateTimeEdit::dateTimeChanged, this, &Window::dateTimePredict_dateTimeChanged);

    qDebug() << dateTimePredict->dateTime();

    performWarp();
    _timerRunPredict.start(static_cast<int>(spinRunPredictInterval->value() * 1000));
}

void Window::dateTimePredict_dateTimeChanged(QDateTime dateTime) {
    // some niceify on the default behaviour, making the sections depend on each other
    // + only allow selecting downloaded Whazzups if respective option is selected
    disconnect(dateTimePredict, &QDateTimeEdit::dateTimeChanged, this, &Window::dateTimePredict_dateTimeChanged);
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
        qDebug() << "Window::dateTimePredict_dateTimeChanged()"
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

    connect(dateTimePredict, &QDateTimeEdit::dateTimeChanged, this, &Window::dateTimePredict_dateTimeChanged);
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
                              "You have just set the startup map position. "
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
    QString filename = Settings::dataDirectory(
            QString("screenshots/%1_%2")
            .arg(Settings::downloadNetwork())
            .arg(Whazzup::instance()->whazzupData().whazzupTime.toString("yyyyMMdd-HHmmss")));

    if (Settings::screenshotMethod() == 0)
        qApp->screens().first()->grabWindow(mapScreen->glWidget->winId()).save(QString("%1.%2").arg(filename, Settings::screenshotFormat()),
                                                    Settings::screenshotFormat().toLatin1());
    else if (Settings::screenshotMethod() == 1)
        mapScreen->glWidget->renderPixmap().save(QString("%1.%2").arg(filename, Settings::screenshotFormat()),
                                      Settings::screenshotFormat().toLatin1(), true);
    else if (Settings::screenshotMethod() == 2)
        mapScreen->glWidget->grabFrameBuffer(true).save(QString("%1.%2").arg(filename, Settings::screenshotFormat()),
                                             Settings::screenshotFormat().toLatin1());
    qDebug() << "Window::shootScreenshot()" << QString("%1.png").arg(filename); //fixme
}

void Window::on_actionShowRoutes_triggered(bool checked) {
    qDebug() << "Window::on_actionShowRoutes_triggered()" << checked;
    GuiMessages::message(QString("toggled routes [%1]").arg(checked? "on": "off"), "routeToggle");
    foreach(Airport *a, NavData::instance()->airports.values()) // synonym to "toggle routes" on all airports
        a->showFlightLines = checked;
    if (!checked) { // when disabled, this shall clear all routes
        foreach (Pilot *p, Whazzup::instance()->whazzupData().allPilots())
            p->showDepDestLine = false;
    }

    // adjust the "plot route" tick in dialogs
    if (AirportDetails::instance(false) != 0)
        AirportDetails::instance()->refresh();
    if (PilotDetails::instance(false) != 0)
        PilotDetails::instance()->refresh();

    // map update
    mapScreen->glWidget->createPilotsList();
    mapScreen->glWidget->updateGL();
    //glWidget->newWhazzupData(); // complete update, but (should be) unnecessary
    qDebug() << "Window::on_actionShowRoutes_triggered() -- finished";
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

    Settings::setShowAllSectors(state);
    mapScreen->glWidget->displayAllSectors(state);
}

void Window::downloadCloud() {
    qDebug() << "Window::startCloudDownload -- prepare Download";
    _timerCloud.stop();

    /*if(!Settings::downloadClouds()) {
        mapScreen->glWidget->cloudsAvaliable = false;
        return;
    }*/

    FileReader file(Settings::dataDirectory("data/cloudmirrors.dat"));

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
    if(Settings::useHighResClouds()) {
        if (!hiResMirrors.isEmpty())
            url.setUrl(hiResMirrors[QRandomGenerator::global()->bounded(quint32(hiResMirrors.size() - 1))]);
    } else {
        if (!loResMirrors.isEmpty())
            url.setUrl(loResMirrors[QRandomGenerator::global()->bounded(quint32(loResMirrors.size() - 1))]);
    }
    if(_cloudDownloadReply != 0) _cloudDownloadReply = 0;
    _cloudDownloadReply = Net::g(url);
    connect(_cloudDownloadReply, &QNetworkReply::finished, this, &Window::cloudDownloadFinished);

    qDebug() << "Window::startCloudDownload -- Download started from " << url.toString();
}

void Window::cloudDownloadFinished() {
    qDebug() << "Window::cloudDownloadFinished";
    emit cloudDownloaded();
    disconnect(_cloudDownloadReply, SIGNAL(finished()), this, SLOT(downloaded()));
    _cloudDownloadReply->deleteLater();

    if(_cloudDownloadReply->error() != QNetworkReply::NoError) {
        GuiMessages::criticalUserInteraction(_cloudDownloadReply->errorString(), "cloudlayer download error:");
        return;
    }

    QImage cloudlayer;
    cloudlayer.load(_cloudDownloadReply->readAll(), "JPG");
    cloudlayer.save(Settings::dataDirectory("textures/clouds/clouds.jpg"), "JPG");
    qDebug() << "Window::cloudDownloadFinished -- clouds.jpg saved  here:"
             << Settings::dataDirectory("textures/clouds/");

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
