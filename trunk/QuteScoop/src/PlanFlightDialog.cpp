/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "PlanFlightDialog.h"

#include <QtXml/QDomDocument>
#include "Settings.h"
#include "Route.h"
#include "Window.h"
#include "NavData.h"
#include "helpers.h"

PlanFlightDialog *planFlightDialogInstance = 0;

PlanFlightDialog *PlanFlightDialog::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(planFlightDialogInstance == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::getInstance(true);
            planFlightDialogInstance = new PlanFlightDialog(parent);
        }
    return planFlightDialogInstance;
}

PlanFlightDialog::PlanFlightDialog(QWidget *parent):
    QDialog(parent)
{
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);

    selectedRoute = 0;

    routesSortModel = new QSortFilterProxyModel;
    routesSortModel->setDynamicSortFilter(true);
    routesSortModel->setSourceModel(&routesModel);

    treeRoutes->setModel(routesSortModel);
    treeRoutes->header()->setResizeMode(QHeaderView::Interactive);
    treeRoutes->sortByColumn(0, Qt::AscendingOrder);
    connect(treeRoutes->header(), SIGNAL(sectionClicked(int)), treeRoutes, SLOT(sortByColumn(int)));
    connect(treeRoutes, SIGNAL(clicked(const QModelIndex&)), this, SLOT(routeSelected(const QModelIndex&)));

    lblPlotStatus->setText(QString(""));
    linePlotStatus->setVisible(false);
    gbResults->setTitle("Results");
}

void PlanFlightDialog::on_buttonRequest_clicked() { // get routes from selected providers
    routes.clear();
    routesModel.setClients(routes);
    gbResults->setTitle(QString("Results [%1-%2] (%3)")
                         .arg(edDep->text())
                         .arg(edDest->text())
                         .arg(routes.size()));
    lblGeneratedStatus->setText(QString());
    lblVrouteStatus->setText(QString());
    lblVatrouteStatus->setText(QString());

    if (cbGenerated->isChecked()) requestGenerated();
    if (cbVroute->isChecked()) requestVroute();
    if (cbVatroute->isChecked()) requestVatroute();
}

void PlanFlightDialog::requestGenerated() {
    if (edDep->text().length() != 4 || edDest->text().length() != 4) {
        lblGeneratedStatus->setText(QString("bad request"));
        return;
    }

    Route *r = new Route();
    r->provider = QString("direct");
    r->dep = edDep->text();
    r->dest = edDest->text();
    r->route = QString("DCT");
    r->comments = QString("just for reference");
    r->lastChange = QString("just now");

    r->calculateWaypointsAndDistance();

    routes.append(r);
    lblGeneratedStatus->setText(QString("1 route"));
    routesModel.setClients(routes);
    routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
    gbResults->setTitle(QString("Results [%1-%2] (%3)")
                         .arg(edDep->text())
                         .arg(edDest->text())
                         .arg(routes.size()));
}

void PlanFlightDialog::requestVroute() {
    lblVrouteStatus->setText(QString("request sent..."));

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
        url.addQueryItem(QString("cycle"), edCycle->text().trimmed()); // defaults to the last freely available (0701)
    url.addQueryItem(QString("type"), QString("query"));
    url.addQueryItem(QString("level"),QString("0")); // details level, so far only 0 supported
    url.addQueryItem(QString("dep"), edDep->text().trimmed());
    url.addQueryItem(QString("arr"), edDest->text().trimmed());

    vrouteDownloader = new QHttp;
    vrouteBuffer = 0;

    connect(vrouteDownloader, SIGNAL(done(bool)), this, SLOT(vrouteDownloaded(bool)));
    Settings::applyProxySetting(vrouteDownloader);

    vrouteDownloader->setHost(url.host(), url.port() != -1 ? url.port() : 80);

    QString querystr = url.path() + "?" + url.encodedQuery();

    if(vrouteBuffer != 0) delete vrouteBuffer;
    vrouteBuffer = new QBuffer;
    vrouteBuffer->open(QBuffer::ReadWrite);

    vrouteDownloader->get(querystr, vrouteBuffer);
}

void PlanFlightDialog::requestVatroute() {
    lblVatrouteStatus->setText(QString("request sent..."));
    QUrl url("http://www.michael-nagler.de/getvatroute.php");

    url.addQueryItem(QString("dep"), edDep->text().trimmed());
    url.addQueryItem(QString("dest"), edDest->text().trimmed());

    vatrouteDownloader = new QHttp;
    vatrouteBuffer = 0;

    connect(vatrouteDownloader, SIGNAL(done(bool)), this, SLOT(vatrouteDownloaded(bool)));
    Settings::applyProxySetting(vatrouteDownloader);

    vatrouteDownloader->setHost(url.host(), url.port() != -1 ? url.port() : 80);

    QString querystr = url.path() + "?" + url.encodedQuery();

    if(vatrouteBuffer != 0) delete vatrouteBuffer;
    vatrouteBuffer = new QBuffer;
    vatrouteBuffer->open(QBuffer::ReadWrite);

    vatrouteDownloader->get(querystr, vatrouteBuffer);
}


void PlanFlightDialog::vrouteDownloaded(bool error) {
    if(vrouteBuffer == 0) {
        lblVatrouteStatus->setText(QString("nothing returned"));
        return;
    }

    if(error) {
        lblVrouteStatus->setText(QString("error: %1")
                              .arg(QString(vrouteDownloader->errorString())));
        return;
    }

    QList<Route*> newroutes;
    QString msg;

    QDomDocument domdoc = QDomDocument();
    vrouteBuffer->seek(0);
    if (!domdoc.setContent(vrouteBuffer)) return;
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

    if (msg.isEmpty()) {
        msg = (QString("%1 route%2")
                            .arg(newroutes.size())
                            .arg(newroutes.size() == 1 ? "": "s"));
    }
    lblVrouteStatus->setText(msg);

    routes += newroutes;
    routesModel.setClients(routes);
    routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
    gbResults->setTitle(QString("Results [%1-%2] (%3)")
                         .arg(edDep->text())
                         .arg(edDest->text())
                         .arg(routes.size()));

    delete vrouteBuffer;
    vrouteBuffer = 0;
    delete vrouteDownloader;
}

void PlanFlightDialog::vatrouteDownloaded(bool error) {
    if(vatrouteBuffer == 0) {
        lblVatrouteStatus->setText(QString("nothing returned"));
        return;
    }

    if(error) {
        lblVatrouteStatus->setText(QString("error: %1")
                              .arg(QString(vatrouteDownloader->errorString())));
        return;
    }

    QList<Route*> newroutes;
    QString msg;

    vatrouteBuffer->seek(0);
    while (vatrouteBuffer->canReadLine()) {
        QString line = vatrouteBuffer->readLine();

        QRegExp regEx = QRegExp("FL(\\d*)-FL(\\d*): (\\w{4})-(.*)-(\\w{4}) \\[(.*)\\]");
        regEx.indexIn(line);

        Route *r = new Route();
        r->provider = QString("VATroute");
        r->dep = edDep->text();
        r->dest = edDest->text();
        r->minFl = regEx.cap(1);
        r->maxFl = regEx.cap(2);
        r->route = regEx.cap(4).split("-").join(" ");;
        r->comments = regEx.cap(6);

        r->calculateWaypointsAndDistance();
        newroutes.append(r);
    }


    if (msg.isEmpty()) {
        msg = (QString("%1 route%2")
                            .arg(newroutes.size())
                            .arg(newroutes.size() == 1 ? "": "s"));
    }
    lblVatrouteStatus->setText(msg);

    routes += newroutes;
    routesModel.setClients(routes);
    routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
    gbResults->setTitle(QString("Results [%1-%2] (%3)")
                         .arg(edDep->text())
                         .arg(edDest->text())
                         .arg(routes.size()));

    delete vatrouteBuffer;
    vatrouteBuffer = 0;
    delete vatrouteDownloader;
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
    if(!index.isValid()) {
        selectedRoute = 0;
        return;
    }
    selectedRoute = routes[routesSortModel->mapToSource(index).row()];
    if(cbPlot->isChecked()) qobject_cast<Window *>(this->parent())->setPlotFlightPlannedRoute(true);
}

void PlanFlightDialog::plotPlannedRoute() const {
    if(selectedRoute == 0) {
        lblPlotStatus->setText("No route to plot");
        return;
    }

    lblPlotStatus->setText(QString("Route waypoints (calculated): %1").arg(selectedRoute->waypointsStr));

    if(selectedRoute->waypoints.size() < 2)
        return;

    QColor lineCol = QColor(255, 0, 0, 255);//Settings::planLineColor();
    glColor4f(lineCol.redF(), lineCol.greenF(), lineCol.blueF(), lineCol.alphaF());

    glLineWidth(Settings::planLineStrength() * 2 + 2);
    if(!Settings::dashedTrackInFront())
        glLineStipple(3, 0xAAAA);

    glBegin(GL_LINE_STRIP);
        double currLat = selectedRoute->waypoints[0]->lat;
        double currLon = selectedRoute->waypoints[0]->lon;
        for(int i = 0; i < selectedRoute->waypoints.size(); i++) {
            NavData::plotPath(currLat, currLon,
                              selectedRoute->waypoints[i]->lat, selectedRoute->waypoints[i]->lon);
            currLat = selectedRoute->waypoints[i]->lat;
            currLon = selectedRoute->waypoints[i]->lon;
        }
        VERTEX(currLat, currLon);

    glEnd();
    glLineStipple(1, 0xFFFF);
}

void PlanFlightDialog::on_cbPlot_toggled(bool checked)
{
    qobject_cast<Window *>(this->parent())->setPlotFlightPlannedRoute(checked);
    lblPlotStatus->setVisible(checked);
    linePlotStatus->setVisible(checked);
}

/* Some test to let the user enter and plot routes
void PlanFlightDialog::on_textRoute_textChanged()
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
    if(cbPlot->isChecked()) qobject_cast<Window *>(this->parent())->setPlotFlightPlannedRoute(true);
}
*/

void PlanFlightDialog::on_pbCopyToClipboard_clicked()
{
    if(selectedRoute != 0) {
        QApplication::clipboard()->setText(selectedRoute->route);
    }
}

void PlanFlightDialog::on_pbVatsimPrefile_clicked()
{
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
