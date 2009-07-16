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

#include "PlanFlightDialog.h"
#include "ui_PlanFlightDialog.h"

#include <QDebug>
#include <QtGui>
#include <QtCore>
#include <qdom.h>

#include "Settings.h"
#include "Route.h"
#include "Window.h"
#include "NavData.h"
#include "helpers.h"

PlanFlightDialog *planFlightDialogInstance = 0;

PlanFlightDialog *PlanFlightDialog::getInstance() {
	if(planFlightDialogInstance == 0)
		planFlightDialogInstance = new PlanFlightDialog();
	return planFlightDialogInstance;
}

PlanFlightDialog::PlanFlightDialog():
<<<<<<< .mine
    QDialog(Window::getInstance())
=======
	QDialog()
>>>>>>> .r102
{
<<<<<<< .mine
    setupUi(this);
//    setWindowFlags(Qt::Tool);
=======
	setupUi(this);
	setWindowFlags(Qt::Tool);
>>>>>>> .r102

	selectedRoute = 0;

	routesSortModel = new QSortFilterProxyModel;
	routesSortModel->setDynamicSortFilter(true);
	routesSortModel->setSourceModel(&routesModel);

	treeRoutes->setModel(routesSortModel);
	treeRoutes->header()->setResizeMode(QHeaderView::Interactive);
	treeRoutes->sortByColumn(0, Qt::AscendingOrder);
	connect(treeRoutes->header(), SIGNAL(sectionClicked(int)), treeRoutes, SLOT(sortByColumn(int)));
	connect(treeRoutes, SIGNAL(clicked(const QModelIndex&)), this, SLOT(routeSelected(const QModelIndex&)));
	connect(this, SIGNAL(networkMessage(QString)), Window::getInstance(), SLOT(networkMessage(QString)));
	connect(this, SIGNAL(downloadError(QString)), Window::getInstance(), SLOT(downloadError(QString)));

	lblPlotStatus->setText(QString(""));
}

void PlanFlightDialog::on_buttonRequest_clicked() { // get routes from selected providers
	routes.clear();
	routesModel.setClients(routes);
	lblGeneratedStatus->setText(QString());
	lblVrouteStatus->setText(QString());

	if (cbGenerated->isChecked()) requestGenerated();
	if (cbVroute->isChecked()) requestVroute();
}

void PlanFlightDialog::requestGenerated() {
<<<<<<< .mine
    if (edDep->text().length() != 4 || edDest->text().length() != 4) {
        lblGeneratedStatus->setText(QString("bad request"));
        return;
    }
      
    // add bogus routes
    QList<Route*> newroutes;
    for (int i=0; i<1; i++) {
        QStringList sl = QStringList();
        sl.append("generated");
        sl.append(QString("%1").arg(1000 + i));
        sl.append(edDep->text());
        sl.append(edDest->text());
        sl.append(QString("%1").arg(180 - i*10));
        sl.append(QString("%1").arg(180 + i*10));
        sl.append("DCT");
        sl.append("n/a");
        sl.append("just a test");
        
        Route *r = new Route(sl);
        newroutes.append(r);
    }
    
    lblGeneratedStatus->setText(QString("%1 route%2")
                                .arg(newroutes.size())
                                .arg(newroutes.size() == 1 ? "": "s"));
    routes.append(newroutes);
    routesModel.setClients(routes);
    routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
=======
	if (edDep->text().length() != 4 || edDest->text().length() != 4) {
		lblGeneratedStatus->setText(QString("bad request"));
		return;
	}

	// add 3 bogus routes
	QList<Route*> newroutes;
	for (int i=0; i<3; i++) {
		QStringList sl = QStringList();
		sl.append("generated");
		sl.append(QString("%1").arg(1000 + i));
		sl.append(edDep->text());
		sl.append(edDest->text());
		sl.append(QString("%1").arg(180 - i*10));
		sl.append(QString("%1").arg(180 + i*10));
		sl.append("AETSCH DCT UPPS DCT HOPPLA");
		sl.append("n/a");
		sl.append("just a test");

		Route *r = new Route(sl);
		newroutes.append(r);
	}

	lblGeneratedStatus->setText(QString("%1 route%2")
								.arg(newroutes.size())
								.arg(newroutes.size() == 1 ? "": "s"));
	routes += newroutes;
	routesModel.setClients(routes);
	routesSortModel->invalidate();
	treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
>>>>>>> .r102
}

void PlanFlightDialog::requestVroute() {
	lblVrouteStatus->setText(QString("request sent..."));
	QString vr = QString("http://data.vroute.net/internal/query.php");

	// Add it manually - we need to find some way to manage that this code is not abused
	// Cause if it is - we are all blocked from vroute access!
	QString authCode("12f2c7fd6654be40037163242d87e86f"); //fixme

	if (authCode == "") {
		lblVrouteStatus->setText(QString("auth code unavailable. add it in the source"));
		return;
	}

	QUrl url(vr);
	url.addQueryItem(QString("auth_code"), authCode);
	if(!edCycle->text().trimmed().isEmpty())
		url.addQueryItem(QString("cycle"), edCycle->text().trimmed()); // defaults to the last freely available (0701)
	url.addQueryItem(QString("type"), QString("query"));
	url.addQueryItem(QString("level"),QString("0")); // details level, so far only 0 supported
	url.addQueryItem(QString("dep"), edDep->text().trimmed());
	url.addQueryItem(QString("arr"), edDest->text().trimmed());

	fpDownloader = new QHttp;
	fpBuffer = 0;

	connect(fpDownloader, SIGNAL(done(bool)), this, SLOT(fpDownloaded(bool)));
	connect(fpDownloader, SIGNAL(dataReadProgress(int,int)), this, SLOT(fpDownloading(int,int)));
	Settings::applyProxySetting(fpDownloader);

	fpDownloader->setHost(url.host(), url.port() != -1 ? url.port() : 80);

	QString querystr = url.path() + "?" + url.encodedQuery();

	if(fpBuffer != 0) delete fpBuffer;
	fpBuffer = new QBuffer;
	fpBuffer->open(QBuffer::ReadWrite);

	fpDownloader->get(querystr, fpBuffer);
}

void PlanFlightDialog::fpDownloading(int prog, int tot) {
	//qDebug() << prog << tot;
}

void PlanFlightDialog::fpDownloaded(bool error) {
	if(fpBuffer == 0)
		return;

	if(error) {
		lblVrouteStatus->setText(QString("error: %1")
							  .arg(QString(fpDownloader->errorString())));
		//emit downloadError(fpDownloader->errorString());
		return;
	}

	QList<Route*> newroutes;
	QString msg;

	QDomDocument domdoc = QDomDocument();
	fpBuffer->seek(0);
	if (!domdoc.setContent(fpBuffer)) return;
	QDomElement root = domdoc.documentElement();
	if (root.nodeName() != "flightplans") return;
	QDomElement e = root.firstChildElement();
	while (!e.isNull()) {
		if (e.nodeName() == "result") {
			if (e.firstChildElement("num_objects").text() != "")
				msg = QString("%1 route%2")
						.arg(e.firstChildElement("num_objects").text())
						.arg(e.firstChildElement("num_objects").text() == "1" ? "": "s");
			if (e.firstChildElement("version").text() != "1")
				msg = QString("unknown version: %1").arg(e.firstChildElement("version").text());
			if (e.firstChildElement("result_code").text() != "200") {
				if (e.firstChildElement("result_code").text() == "400")
					msg = (QString("bad request"));
				else if (e.firstChildElement("result_code").text() == "403")
					msg = (QString("unauthorized / maximum queries reached"));
				else if (e.firstChildElement("result_code").text() == "404")
					msg = (QString("flightplan not found / server error")); // should not be signaled for non-privileged queries (signals a server error)
				else if (e.firstChildElement("result_code").text() == "405")
					msg = (QString("method not allowed"));
				else if (e.firstChildElement("result_code").text() == "500")
					msg = (QString("internal database error"));
				else if (e.firstChildElement("result_code").text() == "501")
					msg = (QString("level not implemented"));
				else if (e.firstChildElement("result_code").text() == "503")
					msg = (QString("allowed queries from one IP reached - try later"));
				else if (e.firstChildElement("result_code").text() == "505")
					msg = (QString("query mal-formed"));
				else
					msg = (QString("unknown error: #%1")
										.arg(e.firstChildElement("result_code").text()));
			}
		} else if (e.nodeName() == "flightplan") {
			QStringList sl = QStringList();
			sl.append("vroute");
			sl.append(e.firstChildElement("distance").text()); //fixme, should be checked if type="route"
			sl.append(edDep->text());
			sl.append(edDest->text());
			sl.append(e.firstChildElement("min_fl").text());
			sl.append(e.firstChildElement("max_fl").text());
			sl.append(e.firstChildElement("full_route").text());
			sl.append(e.firstChildElement("last_change").text());
			sl.append(e.firstChildElement("comments").text());
			Route *r = new Route(sl);
			newroutes.append(r);
			msg = (QString("%1 route%2")
								.arg(newroutes.size())
								.arg(newroutes.size() == 1 ? "": "s"));
		}
		e = e.nextSiblingElement();
	}
	if (!msg.isEmpty()) lblVrouteStatus->setText(msg);

	routes += newroutes;
	routesModel.setClients(routes);
	routesSortModel->invalidate();
	treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);

	delete fpBuffer;
	fpBuffer = 0;
}

void PlanFlightDialog::on_edDep_textChanged(QString str)
{
	edDep->setText(str.toUpper());
}

void PlanFlightDialog::on_edDest_textChanged(QString str)
{
	edDest->setText(str.toUpper());
}

void PlanFlightDialog::routeSelected(const QModelIndex& index) {
<<<<<<< .mine
    if(!index.isValid()) {
        selectedRoute = 0;
        return;
    }
    selectedRoute = routes[routesSortModel->mapToSource(index).row()];
    if(cbPlot->isChecked()) Window::getInstance()->setPlotFlightPlannedRoute(true);
=======
	if(!index.isValid()) {
		qDebug() <<"deselected";
		selectedRoute = 0;
		return;
	}
	selectedRoute = routes[routesSortModel->mapToSource(index).row()];
	if(cbPlot->isChecked()) Window::getInstance()->setPlotFlightPlannedRoute(true);
>>>>>>> .r102
}

void PlanFlightDialog::plotPlannedRoute() const {
	if(selectedRoute == 0) {
		lblPlotStatus->setText("No route to plot");
		return;
	}

	Airport* dep;
	if(NavData::getInstance()->airports().contains(selectedRoute->dep))
		dep = NavData::getInstance()->airports()[selectedRoute->dep];
	else {
		lblPlotStatus->setText("Dep not found");
		return;
	}

	Airport* dest;
	if(NavData::getInstance()->airports().contains(selectedRoute->dest))
		dest = NavData::getInstance()->airports()[selectedRoute->dest];
	else {
		lblPlotStatus->setText("Dest not found");
		return;
	}

	QStringList list = selectedRoute->flightPlan.split(' ', QString::SkipEmptyParts);
	QList<Waypoint*> points = NavData::getInstance()->getAirac().resolveFlightplan(list, dep->lat, dep->lon);

	Waypoint* depWp = new Waypoint(dep->label, dep->lat, dep->lon);
	points.prepend(depWp);
	Waypoint* destWp = new Waypoint(dest->label, dest->lat, dest->lon);
	points.append(destWp);

	QString resolved;
	for(int i = 0; i < points.size(); i++) {
		resolved += points[i]->label;
		resolved += "-";
	}
	lblPlotStatus->setText(QString("Route resolved to: %1").arg(resolved));

	if(points.size() < 2)
		return;

	QColor lineCol = QColor(255, 0, 0, 255);//Settings::planLineColor();
	glColor4f(lineCol.redF(), lineCol.greenF(), lineCol.blueF(), lineCol.alphaF());

	glLineWidth(Settings::planLineStrength() * 2 + 2);
	if(!Settings::dashedTrackInFront())
		glLineStipple(3, 0xAAAA);

	glBegin(GL_LINE_STRIP);
		double currLat = points[0]->lat;
		double currLon = points[0]->lon;
		for(int i = 0; i < points.size(); i++) {
			NavData::plotPath(currLat, currLon, points[i]->lat, points[i]->lon);
			currLat = points[i]->lat; currLon = points[i]->lon;
		}
		VERTEX(currLat, currLon);

	glEnd();
	glLineStipple(1, 0xFFFF);
}

void PlanFlightDialog::on_cbPlot_toggled(bool checked)
{
	Window::getInstance()->setPlotFlightPlannedRoute(checked);
}

/*void PlanFlightDialog::on_textRoute_textChanged()
{
	textRoute->setPlainText(textRoute->toPlainText().toUpper());
	if(selectedRoute == 0) {
		QStringList sl = QStringList();
			sl.append("user");
			sl.append("");
			sl.append(edDep->text());
			sl.append(edDest->text());
			sl.append("");
			sl.append("");
			sl.append(textRoute->toPlainText());
			sl.append("");
			sl.append("");
		selectedRoute = new Route(sl);
	}
	selectedRoute->flightPlan = textRoute->toPlainText();
	if(cbPlot->isChecked()) Window::getInstance()->setPlotFlightPlannedRoute(true);
}
*/
