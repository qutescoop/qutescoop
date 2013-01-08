/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "PlanFlightDialog.h"

#include <QtXml/QDomDocument>
#include "Settings.h"
#include "Route.h"
#include "Window.h"
#include "NavData.h"
#include "AirportDetails.h"
#include "helpers.h"

PlanFlightDialog *planFlightDialogInstance = 0;
PlanFlightDialog *PlanFlightDialog::instance(bool createIfNoInstance, QWidget *parent) {
    if(planFlightDialogInstance == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::instance();
            planFlightDialogInstance = new PlanFlightDialog(parent);
        }
    return planFlightDialogInstance;
}

PlanFlightDialog::PlanFlightDialog(QWidget *parent):
        QDialog(parent),
        selectedRoute(0) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);

    bDepDetails->hide(); bDestDetails->hide();
    edCycle->setText(QDate::currentDate().toString("yyMM"));

    _routesSortModel = new QSortFilterProxyModel;
    _routesSortModel->setDynamicSortFilter(true);
    _routesSortModel->setSourceModel(&_routesModel);

    treeRoutes->setModel(_routesSortModel);
    treeRoutes->header()->setResizeMode(QHeaderView::Interactive);
    treeRoutes->sortByColumn(0, Qt::AscendingOrder);
    connect(treeRoutes->header(), SIGNAL(sectionClicked(int)), treeRoutes, SLOT(sortByColumn(int)));
    connect(treeRoutes, SIGNAL(clicked(const QModelIndex&)), this, SLOT(routeSelected(const QModelIndex&)));

    pbVatsimPrefile->setVisible(Settings::downloadNetwork() == 1); // only for VATSIM

    lblPlotStatus->setText(QString(""));
    linePlotStatus->setVisible(false);
    gbResults->setTitle("Results");
}

void PlanFlightDialog::on_buttonRequest_clicked() { // get routes from selected providers
    _routes.clear();
    _routesModel.setClients(_routes);
    gbResults->setTitle(QString("Results [%1-%2] (%3)")
                         .arg(edDep->text())
                         .arg(edDest->text())
                         .arg(_routes.size()));
    lblGeneratedStatus->setText(QString());
    lblVrouteStatus->setText(QString());
    lblVatrouteStatus->setText(QString());

    edDep->setText(edDep->text().toUpper());
    edDest->setText(edDest->text().toUpper());

    if (cbGenerated->isChecked()) requestGenerated();
    if (cbVroute->isChecked()) requestVroute();
    if (cbVatroute->isChecked()) requestVatroute();
}

void PlanFlightDialog::requestGenerated() {
    if (edDep->text().length() != 4 || edDest->text().length() != 4) {
        lblGeneratedStatus->setText(QString("bad request"));
        return;
    }
    edGenerated->setText(edGenerated->text().toUpper());

    Route *r = new Route();
    r->provider = QString("user");
    r->dep = edDep->text();
    r->dest = edDest->text();
    r->route = edGenerated->text();
    r->comments = QString("your route");
    r->lastChange = QString();

    r->calculateWaypointsAndDistance();

    _routes.append(r);
    lblGeneratedStatus->setText(QString("1 route"));
    _routesModel.setClients(_routes);
    _routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
    gbResults->setTitle(QString("Results [%1-%2] (%3)")
                         .arg(edDep->text())
                         .arg(edDest->text())
                         .arg(_routes.size()));
}

void PlanFlightDialog::requestVroute() {
    // We need to find some way to manage that this code is not abused
    // 'cause if it is - we are all blocked from vroute access!
    QString authCode("12f2c7fd6654be40037163242d87e86f"); //fixme
    if (authCode == "") {
        lblVrouteStatus->setText(QString("auth code unavailable. add it in the source"));
        return;
    }
    QUrl url("http://data.vroute.net/internal/query.php");
    url.addQueryItem(QString("auth_code"), authCode);
    if(!edCycle->text().trimmed().isEmpty())
            url.addQueryItem(
                        QString("cycle"),
                        edCycle->text().trimmed()
            ); // defaults to the last freely available
    url.addQueryItem(QString("type"), QString("query"));
    url.addQueryItem(QString("level"),QString("0")); // details level, so far only 0 supported
    url.addQueryItem(QString("dep"), edDep->text().trimmed());
    url.addQueryItem(QString("arr"), edDest->text().trimmed());


    _replyVroute = Net::g(QNetworkRequest(url));
    connect(_replyVroute, SIGNAL(finished()), this, SLOT(vrouteDownloaded()));

    lblVrouteStatus->setText(QString("request sent..."));
}

void PlanFlightDialog::vrouteDownloaded() {
    qDebug() << "PlanFlightDialog::vrouteDownloaded()";
    disconnect(_replyVroute, SIGNAL(finished()), this, SLOT(vrouteDownloaded()));
    _replyVroute->deleteLater();

    if(_replyVroute->error() != QNetworkReply::NoError) {
        lblVrouteStatus->setText(QString("error: %1")
                              .arg(_replyVroute->errorString()));
        return;
    }

    if(_replyVroute->bytesAvailable() == 0)
        lblVrouteStatus->setText(QString("nothing returned"));


    QList<Route*> newroutes;
    QString msg;

    QDomDocument domdoc = QDomDocument();
    if (!domdoc.setContent(_replyVroute->readAll()))
        return;
    QDomElement root = domdoc.documentElement();
    if (root.nodeName() != "flightplans")
        return;
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
                    msg = (QString("flightplan not found / server error")); // should not be..
                                // ..signaled for non-privileged queries (signals a server error)
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
            Route *r = new Route();
            r->provider = QString("vroute");
            r->routeDistance = QString("%1 NM").arg(e.firstChildElement("distance").text());
            r->dep = edDep->text();
            r->dest = edDest->text();
            r->minFl = e.firstChildElement("min_fl").text();
            r->maxFl = e.firstChildElement("max_fl").text();
            r->route = e.firstChildElement("full_route").text();
            r->lastChange = e.firstChildElement("last_change").text();
            r->comments = e.firstChildElement("comments").text();

            r->calculateWaypointsAndDistance();
            newroutes.append(r);
        }
        e = e.nextSiblingElement();
    }

    if (msg.isEmpty())
        msg = QString("%1 route%2").arg(newroutes.size()).arg(newroutes.size() == 1 ? "": "s");
    lblVrouteStatus->setText(msg);

    _routes += newroutes;
    _routesModel.setClients(_routes);
    _routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
    gbResults->setTitle(QString("Results [%1-%2] (%3)")
                         .arg(edDep->text())
                         .arg(edDest->text())
                         .arg(_routes.size()));
}

void PlanFlightDialog::requestVatroute() {
//    QUrl url("http://www.michael-nagler.de/getvatroute.php"); // kindly providing an interface with parsed results
//    url.addQueryItem(QString("dep"), edDep->text().trimmed());
//    url.addQueryItem(QString("dest"), edDest->text().trimmed());

    QUrl url("http://www.vatroute.net/web_showfp.php");
    url.addQueryItem(QString("dep"), edDep->text().trimmed());
    url.addQueryItem(QString("dest"), edDest->text().trimmed());

    _replyVatroute = Net::g(QNetworkRequest(url));
    connect(_replyVatroute, SIGNAL(finished()), this, SLOT(vatrouteDownloaded()));
    lblVatrouteStatus->setText(QString("request sent..."));
}

void PlanFlightDialog::vatrouteDownloaded() {
    qDebug() << "PlanFlightDialog::vatrouteDownloaded()";
    disconnect(_replyVatroute, SIGNAL(finished()), this, SLOT(vatrouteDownloaded()));
    _replyVatroute->deleteLater();

    if(_replyVatroute->error() != QNetworkReply::NoError) {
        lblVatrouteStatus->setText(QString("error: %1")
                              .arg(_replyVatroute->errorString()));
        return;
    }

    if(_replyVatroute->bytesAvailable() == 0)
        lblVatrouteStatus->setText(QString("nothing returned"));

    QList<Route*> newRoutes;

    QRegExp rxRoutes("(?:<tr[^>]*>.*){23}.*(<tr.*>.*<\\/tr>.*)<\\/table>");
    rxRoutes.setMinimal(true);

    if (rxRoutes.indexIn(_replyVatroute->readAll()) != -1) {
        QRegExp rx("\\s*<tr[^<]*<td[^<]*(?:<.><\\/.>)?FL(\\d{3})-FL(\\d{3})(?:<.><\\/.>)?<\\/td[^<]*<td[^<]*>(?:<.><\\/.>)?([^<]*)(?:<.><\\/.>)?<\\/td[^<]*<td[^<]*>(?:<.><\\/.>)?([^<]*)(?:<.><\\/.>)?<\\/td[^<]*<td[^<]*<a[^<]*<img[^<]*<\\/a><\\/td>\\s*<td[^<]*<a[^<]*<img[^<]*<\\/a><\\/td>\\s*<\\/tr>");
        QString rStr = rxRoutes.cap(1);

        int pos = 0;
        while ((pos = rx.indexIn(rStr, pos)) != -1) {
            Route *r = new Route();
            r->provider = QString("VATroute");
            r->dep = edDep->text();
            r->dest = edDest->text();
            r->minFl = rx.cap(1);
            r->maxFl = rx.cap(2);
            r->route = rx.cap(3).toUpper();
            r->comments = rx.cap(4);

            r->calculateWaypointsAndDistance();
            newRoutes.append(r);

            pos += rx.matchedLength();
        }
    }

    lblVatrouteStatus->setText(QString("%1 route%2").arg(newRoutes.size()).
                               arg(newRoutes.size() == 1 ? "": "s"));

    _routes += newRoutes;
    _routesModel.setClients(_routes);
    _routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
    gbResults->setTitle(QString("Results [%1-%2] (%3)")
                         .arg(edDep->text())
                         .arg(edDest->text())
                         .arg(_routes.size()));
}

void PlanFlightDialog::on_edDep_textChanged(QString str) {
    bDepDetails->setVisible(NavData::instance()->airports.contains(str));
}

void PlanFlightDialog::on_edDest_textChanged(QString str) {
    bDestDetails->setVisible(NavData::instance()->airports.contains(str));
}

void PlanFlightDialog::on_bDepDetails_clicked() {
    Airport *airport = NavData::instance()->airports.value(edDep->text());
    if (airport != 0)
        airport->showDetailsDialog();
}

void PlanFlightDialog::on_bDestDetails_clicked() {
    Airport *airport = NavData::instance()->airports.value(edDest->text());
    if (airport != 0)
        airport->showDetailsDialog();
}

void PlanFlightDialog::routeSelected(const QModelIndex& index) {
    if(!index.isValid()) {
        selectedRoute = 0;
        return;
    }
    if(selectedRoute != _routes[_routesSortModel->mapToSource(index).row()]) {
        selectedRoute = _routes[_routesSortModel->mapToSource(index).row()];
        if(cbPlot->isChecked()) on_cbPlot_toggled(true);
    }
}

void PlanFlightDialog::plotPlannedRoute() const {
    if(selectedRoute == 0 || !cbPlot->isChecked()) {
        lblPlotStatus->setText("no route to plot");
        return;
    }
    lblPlotStatus->setText(QString("waypoints (calculated): %1").arg(selectedRoute->waypointsStr));
    if(selectedRoute->waypoints.size() < 2)
        return;
    QList<DoublePair> points;
    foreach(const Waypoint *wp, selectedRoute->waypoints)
        points.append(DoublePair(wp->lat, wp->lon));
    glColor4f(0., 0., 1., 1.);
    glLineWidth(3.);
    glBegin(GL_LINE_STRIP);
    NavData::plotPointsOnEarth(points);
    glEnd();
    glPointSize(4.);
    glColor4f(1., 0., 0., 1.);
    glBegin(GL_POINTS);
    foreach(const DoublePair p, points)
        VERTEX(p.first, p.second);
    glEnd();
}

void PlanFlightDialog::on_cbPlot_toggled(bool checked) {
    if (Window::instance(false) != 0) {
        Window::instance()->mapScreen->glWidget->createPilotsList();
        Window::instance()->mapScreen->glWidget->updateGL();
    }
    lblPlotStatus->setVisible(checked);
    linePlotStatus->setVisible(checked);
}

void PlanFlightDialog::on_pbCopyToClipboard_clicked() {
    if(selectedRoute != 0)
        QApplication::clipboard()->setText(selectedRoute->route);
}

void PlanFlightDialog::on_pbVatsimPrefile_clicked() {
    if(selectedRoute != 0) {
        QUrl url = QUrl(QString("http://www.vatsim.net/fp/?1=I&5=%1&9=%2&8=%3&voice=/V/")
                        .arg(selectedRoute->dep)
                        .arg(selectedRoute->dest)
                        .arg(selectedRoute->route)
                        , QUrl::TolerantMode);
        if (url.isValid()) {
            if(!QDesktopServices::openUrl(url))
                QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("Could not invoke browser"));
        } else
            QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("URL %1 is invalid").arg(url.toString()));
    }
}

void PlanFlightDialog::closeEvent(QCloseEvent *event) {
    Settings::setPlanFlightDialogPos(pos());
    Settings::setPlanFlightDialogSize(size());
    Settings::setPlanFlightDialogGeometry(saveGeometry());
    event->accept();
}
