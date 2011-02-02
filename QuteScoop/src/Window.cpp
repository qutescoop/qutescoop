/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include <QtGui>
#include <QApplication>
//#include <QTemporaryFile>
#include "GLWidget.h"
#include "Window.h"
#include "ClientDetails.h"
#include "helpers.h"
#include "Settings.h"
#include "SearchResultModel.h"
#include "SearchVisitor.h"
#include "MetarSearchVisitor.h"
#include "NavData.h"
#include "PilotDetails.h"
#include "ControllerDetails.h"
#include "AirportDetails.h"
#include "FriendsVisitor.h"
#include "LogBrowserDialog.h"

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
    setupUi(this);

    if(Settings::resetOnNextStart())
        QSettings().clear();

    setWindowTitle(QString("QuteScoop %1").arg(VERSION_NUMBER));

    // Playing with toolbar
    //addToolBar(toolBar = new QToolBar());
    //toolBar->addWidget(new QLabel("test", toolBar));

    QSettings* settings = new QSettings();
    QGLFormat fmt;

    // apply styleSheet
    qDebug() << "applying styleSheet:" << Settings::stylesheet();
    setStyleSheet(Settings::stylesheet());

    // Can please somebody comment on which settings are useful and which
    // should go to the preferences window? Thanks, jonas.
    fmt.setDirectRendering(settings->value("gl/directrendering", fmt.defaultFormat().directRendering()).toBool());
    fmt.setDoubleBuffer(settings->value("gl/doublebuffer", fmt.defaultFormat().doubleBuffer()).toBool());
    fmt.setStencil(settings->value("gl/stencilbuffer", fmt.defaultFormat().stencil()).toBool());
    if (fmt.defaultFormat().stencilBufferSize() > 0)
        fmt.setStencilBufferSize(settings->value("gl/stencilsize", fmt.defaultFormat().stencilBufferSize()).toInt());
    fmt.setDepth(settings->value("gl/depthbuffer", fmt.defaultFormat().depth()).toBool());
    if (fmt.defaultFormat().depthBufferSize() > 0)
        fmt.setDepthBufferSize(settings->value("gl/depthsize", fmt.defaultFormat().depthBufferSize()).toInt());
    fmt.setAlpha(settings->value("gl/alphabuffer", fmt.defaultFormat().alpha()).toBool());
    if (fmt.defaultFormat().alphaBufferSize() > 0)
        fmt.setAlphaBufferSize(settings->value("gl/alphasize", fmt.defaultFormat().alphaBufferSize()).toInt());
    fmt.setSampleBuffers(settings->value("gl/samplebuffers", fmt.defaultFormat().sampleBuffers()).toBool());
    if (fmt.defaultFormat().samples() > 0)
        fmt.setSamples(settings->value("gl/samples", fmt.defaultFormat().samples()).toInt());
    fmt.setAccum(settings->value("gl/accumbuffer", fmt.defaultFormat().accum()).toBool());
    if (fmt.defaultFormat().accumBufferSize() > 0)
        fmt.setAccumBufferSize(settings->value("gl/accumsize", fmt.defaultFormat().accumBufferSize()).toInt());
    //fmt.setRgba(true);
    glWidget = new GLWidget(fmt);

    // have fun :)
    //setAttribute(Qt::WA_TranslucentBackground, true);
    //glWidget->setAttribute(Qt::WA_TranslucentBackground, true);

    setAttribute(Qt::WA_AlwaysShowToolTips, true);

    centralwidget->layout()->addWidget(glWidget);
    qDebug() << "OpenGL support: " << glWidget->format().hasOpenGL()
            << "\t| version: " << glWidget->format().openGLVersionFlags()
            << "\nActually applied options (config has options to overwrite):"
            << "\ngl/directrendering" << glWidget->format().directRendering()
            << "\t| gl/doublebuffer" << glWidget->format().doubleBuffer()
            << "\t| gl/stencilbuffer" << glWidget->format().stencil()
            << "\t| gl/stencilsize" << glWidget->format().stencilBufferSize()
            << "\ngl/depthbuffer" << glWidget->format().depth()
            << "\t| gl/depthsize" << glWidget->format().depthBufferSize()
            << "\t| gl/alphabuffer" << glWidget->format().alpha()
            << "\t| gl/alphasize" << glWidget->format().alphaBufferSize()
            << "\ngl/samplebuffers" << glWidget->format().sampleBuffers()
            << "\t| gl/samples" << glWidget->format().samples()
            << "\t| gl/accumbuffer" << glWidget->format().accum()
            << "\t| gl/accumsize" << glWidget->format().accumBufferSize();

    clientSelection = new ClientSelectionWidget();

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

    Whazzup *whazzup = Whazzup::getInstance();
    connect(whazzup, SIGNAL(hasGuiMessage(QString,GuiMessage::GuiMessageType,QString,int,int)),
            this, SLOT(showGuiMessage(QString,GuiMessage::GuiMessageType,QString,int,int)));
    connect(actionDownload, SIGNAL(triggered()), whazzup, SLOT(download()));
    //connect(actionDownload, SIGNAL(triggered()), glWidget, SLOT(updateGL()));

    // these 2 get disconnected and connected again to inhibit unnecessary updates:
    connect(whazzup, SIGNAL(newData(bool)), glWidget, SLOT(newWhazzupData(bool)));
    connect(whazzup, SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));

    connect(glWidget, SIGNAL(mapClicked(int, int, QPoint)), this, SLOT(mapClicked(int, int, QPoint)));

    if(Settings::downloadOnStartup()) {
        // download whazzup as soon as whazzup status download is complete
        connect(whazzup, SIGNAL(statusDownloaded()), whazzup, SLOT(download()));
    }
    // Always download status
    whazzup->setStatusLocation(Settings::statusLocation());

    searchResult->setModel(&searchResultModel);
    connect(searchResult, SIGNAL(doubleClicked(const QModelIndex&)), &searchResultModel, SLOT(modelDoubleClicked(const QModelIndex&)));
    connect(searchResult, SIGNAL(clicked(const QModelIndex&)), &searchResultModel, SLOT(modelClicked(const QModelIndex&)));
    connect(searchResult->header(), SIGNAL(sectionClicked(int)), searchResult, SLOT(sortByColumn(int)));
    searchResult->sortByColumn(0, Qt::AscendingOrder);

    metarSortModel = new QSortFilterProxyModel;
    metarSortModel->setDynamicSortFilter(true);
    metarSortModel->setSourceModel(&metarModel);
    metarList->setModel(metarSortModel);

    connect(metarList, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(metarDoubleClicked(const QModelIndex&)));
    connect(metarList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(metarDoubleClicked(const QModelIndex&)));
    connect(metarList->header(), SIGNAL(sectionClicked(int)), metarList, SLOT(sortByColumn(int)));
    metarList->sortByColumn(0, Qt::AscendingOrder);

    friendsSortModel = new QSortFilterProxyModel;
    friendsSortModel->setDynamicSortFilter(true);
    friendsSortModel->setSourceModel(&friendsModel);
    friendsList->setModel(friendsSortModel);

    connect(friendsList, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(friendDoubleClicked(const QModelIndex&)));
    connect(friendsList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(friendClicked(const QModelIndex&)));
    connect(friendsList->header(), SIGNAL(sectionClicked(int)), friendsList, SLOT(sortByColumn(int)));
    metarList->sortByColumn(0, Qt::AscendingOrder);

    connect(&searchTimer, SIGNAL(timeout()), this, SLOT(performSearch()));
    connect(&metarTimer, SIGNAL(timeout()), this, SLOT(updateMetars()));
    connect(&downloadWatchdog, SIGNAL(timeout()), this, SLOT(downloadWatchdogTriggered()));

    connect(actionZoomIn, SIGNAL(triggered()), glWidget, SLOT(zoomIn()));
    connect(actionZoomOut, SIGNAL(triggered()), glWidget, SLOT(zoomOut()));
    connect(actionDisplayAllSectors, SIGNAL(toggled(bool)), glWidget, SLOT(displayAllSectors(bool)));
    connect(actionShowInactiveAirports, SIGNAL(toggled(bool)), glWidget, SLOT(showInactiveAirports(bool)));
    actionShowInactiveAirports->setChecked(Settings::showInactiveAirports());

    connect(metarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(metarDockMoved(Qt::DockWidgetArea)));
    connect(searchDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(searchDockMoved(Qt::DockWidgetArea)));
    connect(metarDecoderDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(metarDecoderDockMoved(Qt::DockWidgetArea)));
    metarDecoderDock->hide();

    connect(friendsDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(friendsDockMoved(Qt::DockWidgetArea)));

    versionChecker = 0;
    versionBuffer = 0;
    if(Settings::checkForUpdates()){
        checkForUpdates();
        checkForDataUpdates();
    }

    // restore saved states
    glWidget->restorePosition(1);
    if(restoreState(Settings::getSavedState())) { //was: VERSION_INT
        QSize savedSize = Settings::getSavedSize();
        if(!savedSize.isNull()) resize(savedSize);

        QPoint savedPos = Settings::getSavedPosition();
        if(!savedPos.isNull()) move(savedPos);
    }

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

    setEnableBookedAtc(Settings::downloadBookings());

    //LogBrowser
#ifdef QT_NO_DEBUG_OUTPUT
    menuView->removeAction(actionDebugLog);
#endif
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

void Window::showGuiMessage(QString msg, GuiMessage::GuiMessageType msgType, QString id, int progress, int total) {
    switch (msgType) {
    case GuiMessage::Splash:
        qDebug() << "guiMessage[Splash]" << id << msg;
        QMessageBox::information(this, id, msg);
        break;
    case GuiMessage::ProgressBar:
        //qDebug() << "guiMessage[ProgressBar]" << id << msg << progress << total;
        if (!msg.isEmpty()) lblStatus->setText(msg);
        progressBar->setWhatsThis(id);
        progressBar->show();
        progressBar->setMaximum(total);
        progressBar->setValue(progress);
        break;
    case GuiMessage::Temporary:
        qDebug() << "guiMessage[Temporary]" << id << msg;
        lblStatus->setText(msg);
        new GuiMessage(this, msg, msgType, id, 0, 0, 3000);
        break;
    case GuiMessage::InformationUserAttention:
        qDebug() << "guiMessage[Information]" << id << msg;
        QMessageBox::information(this, id, msg);
        break;
    case GuiMessage::Persistent:
        qDebug() << "guiMessage[Persistent]" << id << msg;
        lblStatus->setWhatsThis(msg);
        lblStatus->setText(msg);
        break;
    case GuiMessage::Warning:
        lblStatus->setText(msg);
        new GuiMessage(this, msg, msgType, id, 0, 0, 8000);
        qWarning() << "guiMessage[Warning]" << id << msg;
        break;
    case GuiMessage::ErrorUserAttention:
        qWarning() << "guiMessage[Error]" << id << msg;
        QMessageBox::warning(this, id, msg);
        break;
    case GuiMessage::CriticalUserInteraction:
        qCritical() << "guiMessage[Critical]" << id << msg;
        QMessageBox::critical(this, id, msg);
        break;
    case GuiMessage::FatalUserInteraction:
        qFatal("guiMessage[Fatal]" + id.toAscii() + " " + msg.toAscii());
        QMessageBox::critical(this, id, msg);
        break;
    case GuiMessage::Remove:
        qDebug() << "guiMessage[Remove]" << id;
        // remove Persistent
        if (msg == lblStatus->whatsThis()) {
            lblStatus->setText(QString());
            lblStatus->setWhatsThis(QString());
        }
        // remove Temporary: restore Presistent
        if (msg == lblStatus->text()) lblStatus->setText(lblStatus->whatsThis());
        // remove ProgressBar
        if (id == progressBar->whatsThis()) {
            progressBar->hide();
            lblStatus->setText(lblStatus->whatsThis());
        }
        break;
    case GuiMessage::_Update: // worker function
        qDebug() << "guiMessage[Update]";
        /*QMapIterator<QDateTime, GuiMessage> i(guiMessages);
        while (i.hasNext()) {
            i.next();
            GuiMessage gM = i.value();
            qDebug() << i.key() << gM.msg;
        }*/
    }
}

void Window::whazzupDownloaded(bool isNew) {
    qDebug() << "whazzupDownloaded()";
    const WhazzupData &realdata = Whazzup::getInstance()->realWhazzupData();
    const WhazzupData &data = Whazzup::getInstance()->whazzupData();

    QString msg = QString(tr("%1%2 - %3: %4 clients"))
                  .arg(Settings::downloadNetworkName())
                  .arg(Whazzup::getInstance()->getPredictedTime().isValid()
                       ? " - <b>W A R P E D</b>  to"
                       : ""
                       )
                  .arg(data.timestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
                        ? QString("today %1").arg(data.timestamp().time().toString("HHmm'z'"))
                        : data.timestamp().toString("ddd MM/dd HHmm'z'"))
                  .arg(data.clients());
    showGuiMessage(msg, GuiMessage::Persistent, "status");

    msg = QString("Whazzup %1, bookings %2 updated")
                  .arg(realdata.timestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
                        ? QString("today %1").arg(realdata.timestamp().time().toString("HHmm'z'"))
                        : (realdata.timestamp().isValid()
                           ? realdata.timestamp().toString("ddd MM/dd HHmm'z'")
                           : "never")
                        )
                  .arg(realdata.bookingsTimestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
                        ? QString("today %1").arg(realdata.bookingsTimestamp().time().toString("HHmm'z'"))
                        : (realdata.bookingsTimestamp().isValid()
                           ? realdata.bookingsTimestamp().toString("ddd MM/dd HHmm'z'")
                           : "never")
                        );
    lblWarpInfo->setText(msg);

    if (Whazzup::getInstance()->getPredictedTime().isValid()) {
        framePredict->show();
        dateTimePredict->setDateTime(Whazzup::getInstance()->getPredictedTime());
        if(isNew) {
            // recalculate prediction on new data arrived
            if(data.predictionBasedOnTimestamp() != realdata.timestamp()
                || data.predictionBasedOnBookingsTimestamp() != realdata.bookingsTimestamp()) {
                Whazzup::getInstance()->setPredictedTime(dateTimePredict->dateTime());
            }
        }
    }

    if(isNew) {
        clientSelection->clearClients();
        clientSelection->close();

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

        if(realdata.bookingsTimestamp().isValid()) {
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

    qDebug() << "whazzupDownloaded() -- finished";
}

void Window::refreshFriends() {
    // update friends list
    FriendsVisitor *visitor = new FriendsVisitor();
    Whazzup::getInstance()->whazzupData().accept(visitor);
    friendsModel.setData(visitor->result());
    delete visitor;
    friendsList->reset();

    // update if visible
    if (ListClientsDialog::getInstance(false) != 0) {
        if (ListClientsDialog::getInstance(true)->isVisible())
            ListClientsDialog::getInstance(true)->refresh();
    }
}

void Window::mapClicked(int x, int y, QPoint absolutePos) {
    QList<MapObject*> objects = glWidget->objectsAt(x, y);
    if(objects.size() == 0) {
        // closing all Windows when clicking on an empty spot?
        //on_actionHideAllWindows_triggered();
        clientSelection->clearClients();
        clientSelection->close();
        return;
    }

    if(objects.size() == 1) {
        objects[0]->showDetailsDialog();
    } else {
        clientSelection->setObjects(objects);
        clientSelection->move(absolutePos);
        clientSelection->show();
        //clientSelection->raise();
        //clientSelection->activateWindow();
        //clientSelection->setFocus();
    }
}

void Window::showOnMap(double lat, double lon) {
    // exclude prefiled flights - do not mapcenter on Atlantic between Brazil and Angola (N0/E0)
    if ((lat != 0) || (lon != 0))
        glWidget->setMapPosition(lat, lon, 0.1);
}

void Window::openPreferences() {
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
    ListClientsDialog::getInstance(true)->setFocus();
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
    Settings::saveSize(size());
    Settings::savePosition(pos());

    on_actionHideAllWindows_triggered();

    QMainWindow::closeEvent(event);
    exit(0);
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

    clientSelection->close();
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
    default:
        break;
    }
}

void Window::checkForUpdates() {
    /* DISABLED
    versionChecker = new QHttp(this);
    connect(versionChecker, SIGNAL(done(bool)), this, SLOT(versionDownloaded(bool)));

    QString downloadUrl = "http://qutescoop.svn.sourceforge.net/svnroot/qutescoop/trunk/QuteScoop/version.txt";

    if(Settings::sendVersionInformation()) {
        // append platform, version and preferred network information to the download link
        QString urlArgs = QString("?%1&%2&").arg(VERSION_NUMBER).arg(Settings::downloadNetwork());
        #ifdef Q_WS_WIN
            urlArgs += "w";
        #endif
        #ifdef Q_WS_MAC
            urlArgs += "m";
        #endif
        #ifdef Q_WS_X11
            urlArgs += "l";
        #endif
        downloadUrl += urlArgs;
    }

    qDebug() << "Checking for new version on" << downloadUrl;
    QUrl url(downloadUrl);
    versionChecker->setHost(url.host(), url.port() != -1 ? url.port() : 80);
    Settings::applyProxySetting(versionChecker);

    if (!url.userName().isEmpty())
        versionChecker->setUser(url.userName(), url.password());

    QString querystr = url.path() + "?" + url.encodedQuery();
    versionBuffer = new QBuffer;
    versionBuffer->open(QBuffer::ReadWrite);
    versionChecker->get(querystr, versionBuffer);
    */
}

void Window::versionDownloaded(bool error) {
    /* DISABLED
    if(!error) {
        // compare downloaded version with my own
        versionBuffer->seek(0);
        if(versionBuffer->canReadLine()) {
            QString newVersion = QString(versionBuffer->readLine()).trimmed();
            QString myVersion = VERSION_NUMBER;

            // if downloaded is greater, see if we told about new version
            if(myVersion < newVersion) {
                if(newVersion != Settings::updateVersionNumber()) {
                    // tell user that there is a newer version
                    QMessageBox::information(this, tr("New Version Available"),
                            QString("A new version of QuteScoop is available! Visit the ")
                            + "<a href='http://sourceforge.net/projects/qutescoop'>sourceforge.net/projects/qutescoop</a> for more information.<br><br>"
                            + "You are using: " + VERSION_NUMBER + "<br>"
                            + "New version is: " + newVersion);

                    // remember that we told about new version
                    Settings::setUpdateVersionNumber(newVersion);
                }
            }
        }
    }
    */
}

void Window::checkForDataUpdates() {
    if(dataVersionsAndFilesDownloader != 0) {
        dataVersionsAndFilesDownloader = 0;
    }
    dataVersionsAndFilesDownloader = new QHttp(this);
    QUrl url("http://qutescoop.svn.sourceforge.net/svnroot/qutescoop/trunk/QuteScoop/data/dataversions.txt");
    dataVersionsAndFilesDownloader->setHost(url.host());
    Settings::applyProxySetting(dataVersionsAndFilesDownloader);

    connect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)), this, SLOT(dataVersionsDownloaded(bool)));

    dataVersionsBuffer = new QBuffer;
    dataVersionsBuffer->open(QBuffer::ReadWrite);

    dataVersionsAndFilesDownloader->get(url.path(), dataVersionsBuffer);
    qDebug() << "Checking for datafile versions:" << url.toString();
}

void Window::dataVersionsDownloaded(bool error) {
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)), this, SLOT(dataVersionsDownloaded(bool)));
    if(dataVersionsBuffer == 0)
        return;

    if(error) {
        showGuiMessage(dataVersionsAndFilesDownloader->errorString(), GuiMessage::Warning, "Datafile download");
        return;
    }
    QList< QPair< QString, int> > serverDataVersionsList, localDataVersionsList;

    dataVersionsBuffer->seek(0);
    while(dataVersionsBuffer->canReadLine()) {
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
        showGuiMessage(QString("Could not read %1.\nThus we are updating all datafiles.")
                       .arg(localVersionsFile.fileName()),
                       GuiMessage::InformationUserAttention, "Complete datafiles update necessary");
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
           localDataVersionsList.value(i, QPair< QString, int>(QString(), 0)).second){
            dataFilesToDownload.append(new QFile(Settings::applicationDataDirectory("data/%1.newFromServer")
                                                 .arg(serverDataVersionsList[i].first)));
            QUrl url(QString("http://qutescoop.svn.sourceforge.net/svnroot/qutescoop/trunk/QuteScoop/data/%1")
                 .arg(serverDataVersionsList[i].first));
            dataVersionsAndFilesDownloader->get(url.path(), dataFilesToDownload.last());
            //qDebug() << "Downloading datafile" << url.toString();
        }
    }
    if (!dataFilesToDownload.isEmpty())
        showGuiMessage(QString("New sector-/ airport- or geography-files are available. They will be downloaded now."),
                       GuiMessage::InformationUserAttention, "New datafiles");
    else {
        disconnect(dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
                this, SLOT(dataFilesRequestFinished(int,bool)));
        disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
                this, SLOT(dataFilesDownloaded(bool)));
        dataVersionsAndFilesDownloader->abort();
        delete dataVersionsAndFilesDownloader;
        delete dataVersionsBuffer;
    }
}

void Window::dataFilesRequestFinished(int id, bool error) {
    if (error) {
        showGuiMessage(QString("Error downloading %1:\n%2")
                       .arg(dataVersionsAndFilesDownloader->currentRequest().path())
                       .arg(dataVersionsAndFilesDownloader->errorString()),
                       GuiMessage::CriticalUserInteraction, "New datafiles");
        return;
    }
    showGuiMessage(QString("Downloaded %1")
                   .arg(dataVersionsAndFilesDownloader->currentRequest().path()),
                   GuiMessage::InformationUserAttention, "New datafiles");
}

void Window::dataFilesDownloaded(bool error) {
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(requestFinished(int,bool)),
            this, SLOT(dataFilesRequestFinished(int,bool)));
    disconnect(dataVersionsAndFilesDownloader, SIGNAL(done(bool)),
            this, SLOT(dataFilesDownloaded(bool)));
    if(dataVersionsBuffer == 0)
        return;

    if(error) {
        showGuiMessage(QString("New sector- / airport- / geography-files could not be downloaded.\n%1")
                       .arg(dataVersionsAndFilesDownloader->errorString()),
                       GuiMessage::CriticalUserInteraction, "New datafiles");
        return;
    }

    showGuiMessage("All scheduled files have been downloaded.\nThese changes will take effect on the next start of QuteScoop.",
                   GuiMessage::InformationUserAttention, "New datafiles");

    int errors = 0;
    for(int i = 0; i < dataFilesToDownload.size(); i++) {
        dataFilesToDownload[i]->flush();
        dataFilesToDownload[i]->close();

        if(dataFilesToDownload[i]->exists()) {
            QString datafileFilePath = dataFilesToDownload[i]->fileName().remove(".newFromServer");
            if (QFile::exists(datafileFilePath) && !QFile::remove(datafileFilePath)) {
                showGuiMessage(QString("Unable to delete\n%1")
                               .arg(datafileFilePath), GuiMessage::CriticalUserInteraction, "New datafiles");
                errors++;
            }
            if (!dataFilesToDownload[i]->rename(datafileFilePath)) {
                showGuiMessage(QString("Unable to move downloaded file to\n%1")
                               .arg(datafileFilePath), GuiMessage::CriticalUserInteraction, "New datafiles");
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
            showGuiMessage(QString("Error writing %1").arg(localDataVersionsFile.fileName()),
                           GuiMessage::CriticalUserInteraction, "New datafiles");
    } else
        showGuiMessage(QString("Errors occured. All datafiles will be redownloaded on next launch of QuteScoop."),
                       GuiMessage::CriticalUserInteraction, "New datafiles");

    dataVersionsBuffer->close();
    delete dataVersionsBuffer;
    dataVersionsAndFilesDownloader->abort();
    delete dataVersionsAndFilesDownloader;
    dataFilesToDownload.clear();
}

void Window::updateMetarDecoder(const QString& airport, const QString& decodedText) {
    metarDecoderDock->setWindowTitle("METAR for " + airport);
    metarText->setText(decodedText);
    metarDecoderDock->show();
    metarDecoderDock->raise();
    metarDecoderDock->activateWindow(); // ?? it gets on top only after the second click from AirportDialog...
    metarDecoderDock->setFocus(); // Don't understand how I can bring this nasty on top of all other. A simple click on the titlebar and it is done.
}

void Window::downloadWatchdogTriggered() {
    downloadWatchdog.stop();
    showGuiMessage("Failed to download network data for a while. Maybe a Whazzup location went offline. I try to get the Network Status again.",
                   GuiMessage::ErrorUserAttention, "Whazzup-download failed.");
    Whazzup::getInstance()->setStatusLocation(Settings::statusLocation());
}


void Window::setEnableBookedAtc(bool enable) {
    actionBookedAtc->setEnabled(enable);
}

void Window::performWarp(bool forceUseDownloaded)
{
    editPredictTimer.stop();

    QDateTime warpToTime = dateTimePredict->dateTime();
    if(cbUseDownloaded->isChecked() || forceUseDownloaded) { // use downloaded Whazzups for (past) replay
        qDebug() << "Using downloaded Whazzups";
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::getInstance()->getDownloadedWhazzups();
        for (int i = downloaded.size()-1; i > -1; i--) {
            if((downloaded[i].first <= warpToTime) || (i == 0)) {
                // only if different
                if (downloaded[i].first != Whazzup::getInstance()->realWhazzupData().timestamp()) {
                    // disconnect to inhibit update because will be updated later
                    disconnect(Whazzup::getInstance(), SIGNAL(newData(bool)), glWidget, SLOT(newWhazzupData(bool)));
                    disconnect(Whazzup::getInstance(), SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));

                    Whazzup::getInstance()->fromFile(downloaded[i].second);

                    // keep GUI responsive - leads to hangups?
                    //qApp->processEvents();
                    connect(Whazzup::getInstance(), SIGNAL(newData(bool)), glWidget, SLOT(newWhazzupData(bool)));
                    connect(Whazzup::getInstance(), SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));
                }
                break;
            }
        }
    }
    Whazzup::getInstance()->setPredictedTime(warpToTime);
}

void Window::on_cbUseDownloaded_toggled(bool checked)
{
    qDebug() << "cbUseDownloaded_toggled()" << checked;
    if (!checked) {
        QList<QPair<QDateTime, QString> > downloaded = Whazzup::getInstance()->getDownloadedWhazzups();
        if (!downloaded.isEmpty()) {
            // disconnect to inhibit update because will be updated later
            disconnect(Whazzup::getInstance(), SIGNAL(newData(bool)), glWidget, SLOT(newWhazzupData(bool)));
            disconnect(Whazzup::getInstance(), SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));

            Whazzup::getInstance()->fromFile(downloaded.last().second);

            // keep GUI responsive - leads to hangups?
            //qApp->processEvents();
            connect(Whazzup::getInstance(), SIGNAL(newData(bool)), glWidget, SLOT(newWhazzupData(bool)));
            connect(Whazzup::getInstance(), SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));
        }
    }
    performWarp(true);
}

void Window::on_tbDisablePredict_clicked()
{
    qDebug() << "Window::tbDisablePredict_clicked()";
    actionPredict->setChecked(false);
}

void Window::on_actionPredict_toggled(bool enabled)
{
    qDebug() << "Window::actionPredict_toggled()" << enabled;
    if(enabled) {
        dateTimePredict->setDateTime(
                QDateTime::currentDateTime().toUTC()
                .addSecs(- QDateTime::currentDateTime().toUTC().time().second())); // remove seconds
        framePredict->show();
    } else {
        tbRunPredict->setChecked(false);
        runPredictTimer.stop();

        Whazzup::getInstance()->setPredictedTime(QDateTime()); // remove time warp
        cbUseDownloaded->setChecked(false);
        framePredict->hide();
        widgetRunPredict->hide();
    }
}

void Window::on_tbRunPredict_toggled(bool checked)
{
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
    if (dsRunPredictStep->value() == 0) { // real time selected
        to = QDateTime::currentDateTime().toUTC();
    } else {
        to = Whazzup::getInstance()->getPredictedTime().addSecs(
                static_cast<int>(dsRunPredictStep->value()*60));
    }

    // setting dateTimePredict without "niceify"
    disconnect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));
    dateTimePredict->setDateTime(to);
    connect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));

    performWarp();
    runPredictTimer.start(static_cast<int>(spinRunPredictInterval->value() * 1000));
}

void Window::on_dateTimePredict_dateTimeChanged(QDateTime dateTime)
{
    // some niceify on the default behaviour, making the sections depend on each other
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

    dateTimePredict_old = dateTime;

    if(dateTime.isValid()
        && (dateTime != dateTimePredict->dateTime())) {
        dateTimePredict->setDateTime(dateTime);
    }

    connect(dateTimePredict, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_dateTimePredict_dateTimeChanged(QDateTime)));
    editPredictTimer.start(1000);
}

void Window::on_actionRecallMapPosition_triggered()
{
    glWidget->restorePosition(1);
}

void Window::on_actionRecallMapPosition7_triggered()
{
    glWidget->restorePosition(7);
}

void Window::on_actionRecallMapPosition6_triggered()
{
    glWidget->restorePosition(6);
}

void Window::on_actionRecallMapPosition5_triggered()
{
    glWidget->restorePosition(5);
}

void Window::on_actionRecallMapPosition4_triggered()
{
    glWidget->restorePosition(4);
}

void Window::on_actionRecallMapPosition3_triggered()
{
    glWidget->restorePosition(3);
}

void Window::on_actionRecallMapPosition2_triggered()
{
    glWidget->restorePosition(2);
}

void Window::on_actionRememberPosition_triggered() {
    glWidget->rememberPosition(1);
}

void Window::on_actionRememberMapPosition7_triggered()
{
    glWidget->rememberPosition(7);
}

void Window::on_actionRememberMapPosition6_triggered()
{
    glWidget->rememberPosition(6);
}

void Window::on_actionRememberMapPosition5_triggered()
{
    glWidget->rememberPosition(5);
}

void Window::on_actionRememberMapPosition4_triggered()
{
    glWidget->rememberPosition(4);
}

void Window::on_actionRememberMapPosition3_triggered()
{
    glWidget->rememberPosition(3);
}

void Window::on_actionRememberMapPosition2_triggered()
{
    glWidget->rememberPosition(2);
}

void Window::on_actionMoveLeft_triggered()
{
    glWidget->scrollBy(-1, 0);
}

void Window::on_actionMoveRight_triggered()
{
    glWidget->scrollBy(1, 0);
}

void Window::on_actionMoveUp_triggered()
{
    glWidget->scrollBy(0, -1);
}

void Window::on_actionMoveDown_triggered()
{
    glWidget->scrollBy(0, 1);
}

void Window::on_tbZoomIn_clicked()
{
    glWidget->zoomIn();
}

void Window::on_tbZoomOut_clicked()
{
    glWidget->zoomOut();
}

void Window::updateGLPilots() {
    glWidget->createPilotsList();
    glWidget->updateGL();
}

void Window::shootScreenshot() {
    QString filename = QString(qApp->applicationDirPath() + "/screenshots/%1_%2")
              .arg(Settings::downloadNetwork())
              .arg(Whazzup::getInstance()->whazzupData().timestamp().toString("yyyyMMdd-HHmmss"));

    // variant 1: only works if QuteScoop Window is shown on top
    QPixmap::grabWindow(glWidget->winId()).save(QString("%1.png").arg(filename), "png");
    qDebug() << "shot screenie" << QString("%1.png").arg(filename); //fixme

/*    QPixmap::grabWidget(glWidget).save(QString("%1-variant2.png").arg(filename), "png");
    qDebug() << "shot screenie" << QString("%1-variant2.png").arg(filename); //fixme

    glWidget->grabFrameBuffer(true).save(QString("%1-variant3.png").arg(filename), "png");
    qDebug() << "shot screenie" << QString("%1-variant3.png").arg(filename); //fixme

    glWidget->renderPixmap().save(QString("%1-variant4.png").arg(filename), "png");
    qDebug() << "shot screenie" << QString("%1-variant4.png").arg(filename); //fixme
    */
}

// show the active route from PlanFlightDialog
void Window::setPlotFlightPlannedRoute(bool value) {
    glWidget->plotFlightPlannedRoute = value;
    glWidget->createPilotsList();
    glWidget->updateGL();
}

void Window::on_actionShowRoutes_triggered(bool checked)
{
    qDebug() << "showRoutes()" << checked;
    QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().getAllPilots();
    for (int i=0; i < pilots.size(); i++) {
        pilots[i]->displayLineToDest = checked;
        pilots[i]->displayLineFromDep = checked;
    }
    glWidget->newWhazzupData();

    qDebug() << "showRoutes() -- finished" << checked;

/*    if (checked) {
        QList<Airport*> airports = NavData::getInstance()->airports().values();
        for(int i = 0; i < airports.size(); i++) {
            if(airports[i] != 0) {
                airports[i]->setDisplayFlightLines(false);
            }
        }

        QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().getAllPilots();
        for(int i = 0; i < pilots.size(); i++) {
            if(pilots[i] != 0) {
                pilots[i]->displayLineFromDep = false;
                pilots[i]->displayLineToDest = false;
            }
        }
        // adjust the "plot route" tick in dialogs
        AirportDetails::getInstance(true)->refresh();
        PilotDetails::getInstance(true)->refresh();

        // tell glWidget that there is new whazzup data (which is a lie)
        // so it will refresh itself and clear the lines
        glWidget->newWhazzupData();
    } else {
        QList<Airport*> airports = NavData::getInstance()->airports().values();
        for(int i = 0; i < airports.size(); i++) {
            if(airports[i] != 0) {
                airports[i]->setDisplayFlightLines(true);
            }
        }

        // tell glWidget that there is new whazzup data (which is a lie)
        // so it will refresh itself and clear the lines
    }*/
}

