/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#include <QtGui>

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

// singleton instance
Window *windowInstance = 0;

Window* Window::getInstance() {
	if(windowInstance == 0)
		windowInstance = new Window;
	return windowInstance;
}

Window::Window(QWidget *parent) :
	QMainWindow(parent) {
	setupUi(this);

	if(Settings::resetOnNextStart())
		QSettings().clear();

	QSettings* settings = new QSettings();
	QGLFormat fmt;

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
			<< "\nOptions from Config:"
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

	setProgressBar(0);
	lblStatus->setText("");
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

	qDebug() << "Expecting data directory at" << Settings::dataDirectory() << "(Option: general/dataDirectory)";

	Whazzup *whazzup = Whazzup::getInstance();
	connect(actionDownload, SIGNAL(triggered()), whazzup, SLOT(download()));
	connect(actionDownload, SIGNAL(triggered()), glWidget, SLOT(updateGL()));
	connect(whazzup, SIGNAL(newData(bool)), glWidget, SLOT(newWhazzupData(bool)));
	connect(whazzup, SIGNAL(newData(bool)), this, SLOT(whazzupDownloaded(bool)));
	connect(whazzup, SIGNAL(networkMessage(QString)), this, SLOT(networkMessage(QString)));
	connect(whazzup, SIGNAL(downloadError(QString)), this, SLOT(downloadError(QString)));

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

//#ifndef Q_WS_MAC // lets use CTRL +/- which I think shuld work everywhere. No normal input characters, as these get trapped by the EditWidgets (Search etc.)
	// F11 is Fullscreen on most Linux Displaymanagers
	//actionZoomIn->setShortcut(QKeySequence("F11"));
	//actionZoomOut->setShortcut(QKeySequence("F12"));
//#endif

	connect(actionZoomIn, SIGNAL(triggered()), glWidget, SLOT(zoomIn()));
	connect(actionZoomOut, SIGNAL(triggered()), glWidget, SLOT(zoomOut()));
	connect(actionDisplayAllFirs, SIGNAL(toggled(bool)), glWidget, SLOT(displayAllFirs(bool)));
	connect(actionShowInactiveAirports, SIGNAL(toggled(bool)), glWidget, SLOT(showInactiveAirports(bool)));
	actionShowInactiveAirports->setChecked(Settings::showInactiveAirports());

	connect(metarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(metarDockMoved(Qt::DockWidgetArea)));
	connect(searchDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(searchDockMoved(Qt::DockWidgetArea)));
	connect(metarDecoderDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(metarDecoderDockMoved(Qt::DockWidgetArea)));
	metarDecoderDock->hide();

	connect(friendsDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(friendsDockMoved(Qt::DockWidgetArea)));

	versionChecker = 0;
	versionBuffer = 0;
	if(Settings::checkForUpdates())
		checkForUpdates();

	// restore saved states
	glWidget->restorePosition(1);
	if(restoreState(Settings::getSavedState(), VERSION_INT)) {
		QSize savedSize = Settings::getSavedSize();
		if(!savedSize.isNull()) resize(savedSize);

		QPoint savedPos = Settings::getSavedPosition();
		if(!savedPos.isNull()) move(savedPos);
	}

	// Forecast / Predict settings
	//datePredictTime->setMinimumDate(QDateTime::currentDateTime().toUTC().date().addDays(-1));
	datePredictTime->setDate(QDateTime::currentDateTime().toUTC().date());
	timePredictTime->setTime(QDateTime::currentDateTime().toUTC().time());
	framePredict->hide();
	warpTimer.stop();
	connect(&warpTimer, SIGNAL(timeout()), this, SLOT(performWarp()));

	QFont font = lblWarpInfo->font();
	font.setPointSize(lblWarpInfo->fontInfo().pointSize() - 1);
	lblWarpInfo->setFont(font); //make it a bit smaller than standard text

	setEnableBookedAtc(Settings::downloadBookings());
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

	const QString gpl(
"<small><a href='http://www.qutescoop.org'>QuteScoop</a> - Display FSD network status<br>\
Copyright (C) 2007-2009 Martin Domig <a href='mailto:martin@domig.net'>martin@domig.net</a>\
<p>\
This program is free software: you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation, either version 3 of the License, or \
(at your option) any later version.\
<p>\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details.\
<p>\
You should have received a copy of the GNU General Public License \
along with this program.  If not, see <a href='http://www.gnu.org/licenses/'>www.gnu.org/licenses</a>.</small>");

	QMessageBox::about(this, tr("About QuteScoop"),
			"<font size=\"+1\">" + VERSION_STRING + "</font><br><br>" + gpl);
}

void Window::networkMessage(QString message) {
	QMessageBox::information(this, tr("Network Message"), message);
}

void Window::downloadError(QString message) {
	QMessageBox::critical(this, tr("Download Failed"),
			tr("Data download failed:") +
			QString("<br><br><strong>%1</strong>").arg(message));
}

void Window::whazzupDownloaded(bool isNew) {
	const WhazzupData &realdata = Whazzup::getInstance()->realWhazzupData();
	const WhazzupData &data = Whazzup::getInstance()->whazzupData();

	QString msg = QString(tr("%1%2 - %3 UTC: %4 clients"))
				  .arg(Settings::downloadNetworkName())
				  .arg(Whazzup::getInstance()->getPredictedTime().isValid()
					   ? " - <b>W A R P E D</b>  to"
					   : ""
					   )
				  .arg(data.timestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
						? QString("today %1").arg(data.timestamp().time().toString())
						: data.timestamp().toString("ddd yyyy/MM/dd HH:mm:ss"))
				  .arg(data.clients());
	setStatusText(msg);

	msg = QString("Whazzup %1, bookings %2 updated")
				  .arg(realdata.timestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
						? QString("today %1").arg(realdata.timestamp().time().toString())
						: (realdata.timestamp().isValid()
						   ? realdata.timestamp().toString("ddd yyyy/MM/dd HH:mm:ss")
						   : "never")
						)
				  .arg(realdata.bookingsTimestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
						? QString("today %1").arg(realdata.bookingsTimestamp().time().toString())
						: (realdata.bookingsTimestamp().isValid()
						   ? realdata.bookingsTimestamp().toString("ddd yyyy/MM/dd HH:mm:ss")
						   : "never")
						);
	lblWarpInfo->setText(msg);
	if (Whazzup::getInstance()->getPredictedTime().isValid()) {
		framePredict->show();
		timePredictTime->setTime(Whazzup::getInstance()->getPredictedTime().time());
		datePredictTime->setDate(Whazzup::getInstance()->getPredictedTime().date());
		if(isNew) {
			// recalculate prediction on new data arrived
			if(data.predictionBasedOnTimestamp() != realdata.timestamp()
				|| data.predictionBasedOnBookingsTimestamp() != realdata.bookingsTimestamp()) {
				Whazzup::getInstance()->setPredictedTime(QDateTime(datePredictTime->date(), timePredictTime->time(), Qt::UTC));
			}
		}
	}

	if(isNew) {
		clientSelection->clearClients();
		clientSelection->close();
		performSearch();

		AirportDetails::getInstance()->refresh();
		PilotDetails::getInstance()->refresh();
		ControllerDetails::getInstance()->refresh();
		if(realdata.bookingsTimestamp().isValid()) BookedAtcDialog::getInstance()->refresh();

		refreshFriends();
	}
	downloadWatchdog.stop();
	if(Settings::downloadPeriodically())
		downloadWatchdog.start(Settings::downloadInterval() * 60 * 1000 * 4);
}

void Window::refreshFriends() {
	// update friends list
	FriendsVisitor *visitor = new FriendsVisitor();
	Whazzup::getInstance()->whazzupData().accept(visitor);
	friendsModel.setData(visitor->result());
	delete visitor;
	friendsList->reset();
}

void Window::mapClicked(int x, int y, QPoint absolutePos) {
	QList<MapObject*> objects = glWidget->objectsAt(x, y);
	if(objects.size() == 0) {
		on_actionHideAllWindows_triggered();
		return;
	}

	if(objects.size() == 1) {
		objects[0]->showDetailsDialog();
	} else {
		clientSelection->setObjects(objects);
		clientSelection->move(absolutePos);
		clientSelection->show();
		clientSelection->raise();
		clientSelection->activateWindow();
		clientSelection->setFocus();
	}
}

void Window::showOnMap(double lat, double lon) {
	if ((lat != 0) || (lon != 0)) // exclude prefiled (non-connected) flights - do not mapcenter on Atlantic between Brazil and Angola (N0/E0)
		glWidget->setMapPosition(lat, lon, 0.1);
}

void Window::openPreferences() {
	PreferencesDialog::getInstance()->show();
	PreferencesDialog::getInstance()->raise();
	PreferencesDialog::getInstance()->activateWindow();
	PreferencesDialog::getInstance()->setFocus();
}

void Window::openPlanFlight() {
	PlanFlightDialog::getInstance()->show();
	PlanFlightDialog::getInstance()->raise();
	PlanFlightDialog::getInstance()->activateWindow();
	PlanFlightDialog::getInstance()->setFocus();
}

void Window::openBookedAtc() {
	BookedAtcDialog::getInstance()->show();
	BookedAtcDialog::getInstance()->raise();
	BookedAtcDialog::getInstance()->activateWindow();
	BookedAtcDialog::getInstance()->setFocus();
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
	Settings::saveState(saveState(VERSION_INT));
	Settings::saveSize(size());
	Settings::savePosition(pos());

	on_actionHideAllWindows_triggered();

	QMainWindow::closeEvent(event);
}

void Window::on_actionHideAllWindows_triggered() {
	PilotDetails::getInstance()->close();
	ControllerDetails::getInstance()->close();
	AirportDetails::getInstance()->close();
	//PreferencesDialog::getInstance()->close(); // maybe let them open as they got not invoked by map click - but depends on user feel
	//PlanFlightDialog::getInstance()->close();
	//BookedAtcDialog::getInstance()->close();

	if(metarDecoderDock->isFloating())
		metarDecoderDock->hide();

	clientSelection->close();
}

void Window::on_actionClearAllFlightPaths_triggered() {
	QList<Airport*> airports = NavData::getInstance()->airports().values();
	for(int i = 0; i < airports.size(); i++) {
		if(airports[i] != 0) {
			airports[i]->setDisplayFlightLines(false);
		}
	}

	QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().getAllPilots();
	for(int i = 0; i < pilots.size(); i++) {
		pilots[i]->displayLineFromDep = false;
		pilots[i]->displayLineToDest = false;
	}
	// adjust the "plot route" tick in dialogs
	AirportDetails::getInstance()->refresh();
	PilotDetails::getInstance()->refresh();

	// tell glWidget that there is new whazzup data (which is a lie)
	// so it will refresh itself and clear the lines
	glWidget->newWhazzupData();
}

void Window::on_actionDisplayAllFlightPaths_triggered() {
	QList<Airport*> airports = NavData::getInstance()->airports().values();
	for(int i = 0; i < airports.size(); i++) {
		if(airports[i] != 0) {
			airports[i]->setDisplayFlightLines(true);
		}
	}

	// tell glWidget that there is new whazzup data (which is a lie)
	// so it will refresh itself and clear the lines
	glWidget->newWhazzupData();
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
	versionChecker = new QHttp(this);
	connect(versionChecker, SIGNAL(done(bool)), this, SLOT(versionDownloaded(bool)));

	QString downloadUrl = "http://www.qutescoop.org/version.txt";

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

	QUrl url(downloadUrl);
	QFileInfo fileInfo(url.path());
	QString fileName = fileInfo.fileName();
	versionChecker->setHost(url.host(), url.port() != -1 ? url.port() : 80);
	Settings::applyProxySetting(versionChecker);

	if (!url.userName().isEmpty())
		versionChecker->setUser(url.userName(), url.password());

	QString querystr = url.path() + "?" + url.encodedQuery();
	versionBuffer = new QBuffer;
	versionBuffer->open(QBuffer::ReadWrite);
	versionChecker->get(querystr, versionBuffer);
}

void Window::versionDownloaded(bool error) {
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
							+ "<a href='http://www.qutescoop.org'>www.QuteScoop.org</a> for more information.<br><br>"
							+ "You are using: " + VERSION_NUMBER + "<br>"
							+ "New version is: " + newVersion);

					// remember that we told about new version
					Settings::setUpdateVersionNumber(newVersion);
				}
			}
		}
	}
}

void Window::on_actionRememberPosition_triggered() {
	glWidget->rememberPosition(1);
}

void Window::updateMetarDecoder(const QString& airport, const QString& decodedText) {
	metarDecoderDock->show();
	metarDecoderDock->setWindowTitle("METAR for " + airport);
	metarText->setText(decodedText);
}

void Window::downloadWatchdogTriggered() {
	downloadWatchdog.stop();
	QMessageBox::warning(this, tr("Data Download Failed"),
			QString("I failed to download network data for a while. Maybe a Whazzup location went offline. I try to get the Network Status again.")
		);
	Whazzup::getInstance()->setStatusLocation(Settings::statusLocation());
}

void Window::setStatusText(QString text) {
	lblStatus->setText(text);
}

void Window::setProgressBar(bool isVisible) {
	progressBar->setVisible(isVisible);
}

void Window::setProgressBar(int prog, int tot) {
	if (prog == tot) {
		progressBar->setVisible(false);
	} else {
		progressBar->setVisible(true);
		if (tot == 0) {
			progressBar->setFormat(QString("%1 bytes").arg(prog));
			progressBar->setValue(0);
		} else {
			progressBar->setFormat("%p%");
			progressBar->setValue(100 * prog / tot);
		}
	}
}

void Window::setEnableBookedAtc(bool enable) {
	actionBookedAtc->setEnabled(enable);
}

void Window::performWarp()
{
	warpTimer.stop();
	Whazzup::getInstance()->setPredictedTime(QDateTime(datePredictTime->date(), timePredictTime->time(), Qt::UTC));
}

void Window::on_actionPredict_triggered()
{
}

void Window::on_tbDisablePredict_clicked()
{
	Whazzup::getInstance()->setPredictedTime(QDateTime()); // remove time warp
	actionPredict->setChecked(false);
	framePredict->hide();
}

void Window::on_datePredictTime_dateChanged(QDate date)
{
	warpTimer.stop();
	warpTimer.start(1000);
}

void Window::on_timePredictTime_timeChanged(QTime date)
{
	warpTimer.stop();
	warpTimer.start(1000);
}

void Window::on_actionPredict_toggled(bool value)
{
	if(value) {
		datePredictTime->setMinimumDate(QDateTime::currentDateTime().toUTC().date().addDays(-1));
		datePredictTime->setDate(QDateTime::currentDateTime().toUTC().date());
		timePredictTime->setTime(QDateTime::currentDateTime().toUTC().time()
								 .addSecs(- QDateTime::currentDateTime().toUTC().time().second())); // remove second fraction
		framePredict->show();
	} else {
		on_tbDisablePredict_clicked();
	}
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

void Window::setPlotFlightPlannedRoute(bool value) {
	glWidget->plotFlightPlannedRoute = value;
	glWidget->createPilotsList();
	glWidget->updateGL();
}
