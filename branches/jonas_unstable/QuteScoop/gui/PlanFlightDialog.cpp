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

PlanFlightDialog *planFlightDialogInstance = 0;

PlanFlightDialog *PlanFlightDialog::getInstance() {
	if(planFlightDialogInstance == 0)
		planFlightDialogInstance = new PlanFlightDialog();
	return planFlightDialogInstance;
}

PlanFlightDialog::PlanFlightDialog():
    QDialog()
{
    setupUi(this);
    
	routesSortModel = new QSortFilterProxyModel;
	routesSortModel->setDynamicSortFilter(true);
	routesSortModel->setSourceModel(&routesModel);

    treeRoutes->setModel(routesSortModel);
    treeRoutes->header()->setResizeMode(QHeaderView::Interactive);
    connect(treeRoutes->header(), SIGNAL(sectionClicked(int)), treeRoutes, SLOT(sortByColumn(int)));
    //connect(treeVroute, SIGNAL(clicked(const QModelIndex&)), this, SLOT(routeSelected(const QModelIndex&)));
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
    if (edDep->text().length() != 4 || edDest->text().length() != 4) {
        lblGeneratedStatus->setText(QString("bad request"));
        return;
    }
        
    QList<Route*> newroutes;
  /*  for (int i=0; i<10; i++) {    
        QStringList sl = QStringList();
        sl.append("generated");
        sl.append(QString("%1").arg(1000 + i));
        sl.append(edDep->text());
        sl.append(edDest->text());
        sl.append(QString("%1").arg(180 - i*10));
        sl.append(QString("%1").arg(180 + i*10));
        sl.append("AETSCH DCT UPPS");
        sl.append("n/a");
        sl.append("just a test");
        
        Route *r = new Route(sl);
        newroutes.append(r);
    }
    */
    lblGeneratedStatus->setText(QString("%1 route%2")
                                .arg(newroutes.size())
                                .arg(newroutes.size() == 1 ? "s": ""));
    routes.append(newroutes);
    routesModel.setClients(routes);
    treeRoutes->sortByColumn(routesSortModel->sortColumn(), routesSortModel->sortOrder());
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
}

void PlanFlightDialog::requestVroute() {
    lblVrouteStatus->setText(QString("request sent..."));
    QString vr = QString("http://data.vroute.net/internal/query.php");  
    QString airacCycle("0906");
    
    QString authCode("");
    if (authCode == "") lblVrouteStatus->setText(QString("auth code unavailable. add it in the source"));
        
    QUrl url(vr);
    url.addQueryItem(QString("auth_code"), authCode);
    url.addQueryItem(QString("cycle"), airacCycle);
    url.addQueryItem(QString("type"), QString("query"));
    url.addQueryItem(QString("level"),QString("0"));
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

    QDomDocument domdoc = QDomDocument();
    fpBuffer->seek(0);
    if (!domdoc.setContent(fpBuffer)) return;
    QDomElement root = domdoc.documentElement();
    if (root.nodeName() != "flightplans") return;
    QDomElement e = root.firstChildElement();
    while (!e.isNull()) {
        if (e.nodeName() == "result") {
            if (e.firstChildElement("version").text() != "1")
                lblVrouteStatus->setText(QString("unknown version: %1")
                                        .arg(e.firstChildElement("version").text()));
            if (e.firstChildElement("result_code").text() != "200") {
                if (e.firstChildElement("result_code").text() == "400")
                    lblVrouteStatus->setText(QString("bad request"));
                else if (e.firstChildElement("result_code").text() == "403")
                    lblVrouteStatus->setText(QString("unauthorized"));
                else if (e.firstChildElement("result_code").text() == "404")
                    lblVrouteStatus->setText(QString("flightplan not found / server error")); // should not be signaled for non-privileged queries (signals a server error)
                else if (e.firstChildElement("result_code").text() == "405")
                    lblVrouteStatus->setText(QString("method not allowed"));
                else if (e.firstChildElement("result_code").text() == "500")
                    lblVrouteStatus->setText(QString("internal database error"));
                else if (e.firstChildElement("result_code").text() == "501")
                    lblVrouteStatus->setText(QString("level not implemented"));
                else if (e.firstChildElement("result_code").text() == "503")
                    lblVrouteStatus->setText(QString("too many queries from one IP - try later"));
                else if (e.firstChildElement("result_code").text() == "505")
                    lblVrouteStatus->setText(QString("query mal-formed"));
                else 
                    lblVrouteStatus->setText(QString("unknown error: #%1")
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
        }
        e = e.nextSiblingElement();
    }
    lblGeneratedStatus->setText(QString("%1 route%2")
                                .arg(newroutes.size())
                                .arg(newroutes.size() == 1 ? "s": ""));
    routes.append(newroutes);
   	routesModel.setClients(routes);
    treeRoutes->sortByColumn(routesSortModel->sortColumn(), routesSortModel->sortOrder());
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
