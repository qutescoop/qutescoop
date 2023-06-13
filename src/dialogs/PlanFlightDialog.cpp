#include "PlanFlightDialog.h"

#include "Window.h"
#include "../helpers.h"
#include "../NavData.h"
#include "../Net.h"
#include "../Route.h"
#include "../Settings.h"

#include <QtXml/QDomDocument>

//singleton instance
PlanFlightDialog* planFlightDialogInstance = 0;
PlanFlightDialog* PlanFlightDialog::instance(bool createIfNoInstance, QWidget* parent) {
    if (planFlightDialogInstance == 0) {
        if (createIfNoInstance) {
            if (parent == 0) {
                parent = Window::instance();
            }
            planFlightDialogInstance = new PlanFlightDialog(parent);
        }
    }
    return planFlightDialogInstance;
}

PlanFlightDialog::PlanFlightDialog(QWidget* parent)
    : QDialog(parent),
      selectedRoute(0) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);

    auto preferences = Settings::dialogPreferences(m_preferencesName);
    if (!preferences.size.isNull()) {
        resize(preferences.size);
    }
    if (!preferences.pos.isNull()) {
        move(preferences.pos);
    }
    if (!preferences.geometry.isNull()) {
        restoreGeometry(preferences.geometry);
    }

    bDepDetails->hide(); bDestDetails->hide();
    edCycle->setText(QDate::currentDate().toString("yyMM"));

    _routesSortModel = new QSortFilterProxyModel;
    _routesSortModel->setDynamicSortFilter(true);
    _routesSortModel->setSourceModel(&_routesModel);

    treeRoutes->setModel(_routesSortModel);
    treeRoutes->header()->setSectionResizeMode(QHeaderView::Interactive);
    treeRoutes->sortByColumn(0, Qt::AscendingOrder);
    connect(treeRoutes, &QAbstractItemView::clicked, this, &PlanFlightDialog::routeSelected);

    pbVatsimPrefile->setVisible(Settings::downloadNetwork() == 1); // only for VATSIM

    lblPlotStatus->setText(QString(""));
    linePlotStatus->setVisible(false);
    gbResults->setTitle("Results");
}

void PlanFlightDialog::on_buttonRequest_clicked() { // get routes from selected providers
    _routes.clear();
    _routesModel.setClients(_routes);
    gbResults->setTitle(
        QString("Results [%1-%2] (%3)")
        .arg(edDep->text(), edDest->text())
        .arg(_routes.size())
    );
    lblGeneratedStatus->setText("");
    lblVrouteStatus->setText("");

    edDep->setText(edDep->text().toUpper());
    edDest->setText(edDest->text().toUpper());

    if (cbGenerated->isChecked()) {
        requestGenerated();
    }
    if (cbVroute->isChecked()) {
        requestVroute();
    }
}

void PlanFlightDialog::requestGenerated() {
    if (edDep->text().length() != 4 || edDest->text().length() != 4) {
        lblGeneratedStatus->setText(QString("bad request"));
        return;
    }
    edGenerated->setText(edGenerated->text().toUpper());

    Route* r = new Route();
    r->provider = "user";
    r->dep = edDep->text();
    r->dest = edDest->text();
    r->route = edGenerated->text();
    r->comments = "your route";
    r->lastChange = QString();

    r->calculateWaypointsAndDistance();

    _routes.append(r);
    lblGeneratedStatus->setText("1 route");
    _routesModel.setClients(_routes);
    _routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
    gbResults->setTitle(
        QString("Results [%1-%2] (%3)")
        .arg(edDep->text(), edDest->text())
        .arg(_routes.size())
    );
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
    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QString("auth_code"), authCode);
    if (!edCycle->text().trimmed().isEmpty()) {
        urlQuery.addQueryItem(
            QString("cycle"),
            edCycle->text().trimmed()
        ); // defaults to the last freely available
    }
    urlQuery.addQueryItem("type", "query");
    urlQuery.addQueryItem("level", "0"); // details level, so far only 0 supported
    urlQuery.addQueryItem("dep", edDep->text().trimmed());
    urlQuery.addQueryItem("arr", edDest->text().trimmed());
    url.setQuery(urlQuery);


    _replyVroute = Net::g(url);
    connect(_replyVroute, &QNetworkReply::finished, this, &PlanFlightDialog::vrouteDownloaded);

    lblVrouteStatus->setText(QString("request sent..."));
}

void PlanFlightDialog::vrouteDownloaded() {
    disconnect(_replyVroute, &QNetworkReply::finished, this, &PlanFlightDialog::vrouteDownloaded);
    _replyVroute->deleteLater();

    if (_replyVroute->error() != QNetworkReply::NoError) {
        lblVrouteStatus->setText(
            QString("error: %1")
            .arg(_replyVroute->errorString())
        );
        return;
    }

    if (_replyVroute->bytesAvailable() == 0) {
        lblVrouteStatus->setText("nothing returned");
    }


    QList<Route*> newroutes;
    QString msg;

    QDomDocument domdoc = QDomDocument();
    if (!domdoc.setContent(_replyVroute->readAll())) {
        return;
    }
    QDomElement root = domdoc.documentElement();
    if (root.nodeName() != "flightplans") {
        return;
    }
    QDomElement e = root.firstChildElement();
    while (!e.isNull()) {
        if (e.nodeName() == "result") {
            if (e.firstChildElement("num_objects").text() != "") {
                msg = QString("%1 route%2").arg(
                    e.firstChildElement("num_objects").text(),
                    e.firstChildElement("num_objects").text() == "1"? "": "s"
                );
            }
            if (e.firstChildElement("version").text() != "1") {
                msg = QString("unknown version: %1").arg(e.firstChildElement("version").text());
            }
            if (e.firstChildElement("result_code").text() != "200") {
                if (e.firstChildElement("result_code").text() == "400") {
                    msg = (QString("bad request"));
                } else if (e.firstChildElement("result_code").text() == "403") {
                    msg = (QString("unauthorized / maximum queries reached"));
                } else if (e.firstChildElement("result_code").text() == "404") {
                    msg = (QString("flightplan not found / server error")); // should not be..
                }
                // ..signaled for non-privileged queries (signals a server error)
                else if (e.firstChildElement("result_code").text() == "405") {
                    msg = (QString("method not allowed"));
                } else if (e.firstChildElement("result_code").text() == "500") {
                    msg = (QString("internal database error"));
                } else if (e.firstChildElement("result_code").text() == "501") {
                    msg = (QString("level not implemented"));
                } else if (e.firstChildElement("result_code").text() == "503") {
                    msg = (QString("allowed queries from one IP reached - try later"));
                } else if (e.firstChildElement("result_code").text() == "505") {
                    msg = (QString("query mal-formed"));
                } else {
                    msg = (QString("unknown error: #%1")
                        .arg(e.firstChildElement("result_code").text()));
                }
            }
        } else if (e.nodeName() == "flightplan") {
            Route* r = new Route();
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
        msg = QString("%1 route%2").arg(newroutes.size()).arg(newroutes.size() == 1? "": "s");
    }
    lblVrouteStatus->setText(msg);

    _routes += newroutes;
    _routesModel.setClients(_routes);
    _routesSortModel->invalidate();
    treeRoutes->header()->resizeSections(QHeaderView::ResizeToContents);
    gbResults->setTitle(
        QString("Results [%1-%2] (%3)")
        .arg(edDep->text(), edDest->text())
        .arg(_routes.size())
    );
}

void PlanFlightDialog::on_edDep_textChanged(QString str) {
    int cursorPosition = edDep->cursorPosition();
    edDep->setText(str.toUpper());
    edDep->setCursorPosition(cursorPosition);
    bDepDetails->setVisible(NavData::instance()->airports.contains(str.toUpper()));
    Airport* airport = NavData::instance()->airports.value(str.toUpper());
    if (airport != 0) {
        bDepDetails->setText(airport->mapLabel());
        bDepDetails->setToolTip(airport->toolTip());
    }
    bDepDetails->setVisible(airport != 0);
}

void PlanFlightDialog::on_edDest_textChanged(QString str) {
    int cursorPosition = edDest->cursorPosition();
    edDest->setText(str.toUpper());
    edDest->setCursorPosition(cursorPosition);
    Airport* airport = NavData::instance()->airports.value(str.toUpper());
    if (airport != 0) {
        bDestDetails->setText(airport->mapLabel());
        bDestDetails->setToolTip(airport->toolTip());
    }
    bDestDetails->setVisible(airport != 0);
}

void PlanFlightDialog::on_bDepDetails_clicked() {
    Airport* airport = NavData::instance()->airports.value(edDep->text().toUpper());
    if (airport != 0 && airport->hasPrimaryAction()) {
        airport->primaryAction();
    }
}

void PlanFlightDialog::on_bDestDetails_clicked() {
    Airport* airport = NavData::instance()->airports.value(edDest->text().toUpper());
    if (airport != 0 && airport->hasPrimaryAction()) {
        airport->primaryAction();
    }
}

void PlanFlightDialog::routeSelected(const QModelIndex& index) {
    if (!index.isValid()) {
        selectedRoute = 0;
        return;
    }
    if (selectedRoute != _routes[_routesSortModel->mapToSource(index).row()]) {
        selectedRoute = _routes[_routesSortModel->mapToSource(index).row()];
        if (cbPlot->isChecked()) {
            on_cbPlot_toggled(true);
        }
    }
}

void PlanFlightDialog::plotPlannedRoute() const {
    if (selectedRoute == 0 || !cbPlot->isChecked()) {
        lblPlotStatus->setText("<span style='color: " + Settings::lightTextColor().name(QColor::HexArgb) + "'>no route to plot</span>");
        return;
    }
    lblPlotStatus->setText(QString("<span style='color: " + Settings::lightTextColor().name(QColor::HexArgb) + "'>waypoints (calculated): <code>%1</code></span>").arg(selectedRoute->waypointsStr));
    if (selectedRoute->waypoints.size() < 2) {
        return;
    }
    QList<DoublePair> points;
    foreach (const Waypoint* wp, selectedRoute->waypoints) {
        points.append(DoublePair(wp->lat, wp->lon));
    }

    // @todo: make plot line color adjustable
    // @todo: are we GLing here ouside a GL context?!
    glColor4f((GLfloat) .7, (GLfloat) 1., (GLfloat) .4, (GLfloat) .8);
    glLineWidth(2.);
    glBegin(GL_LINE_STRIP);
    NavData::plotGreatCirclePoints(points);
    glEnd();
    glPointSize(4.);
    glColor4f(.5, .5, .5, .5);
    glBegin(GL_POINTS);
    foreach (const DoublePair p, points) {
        VERTEX(p.first, p.second);
    }
    glEnd();
}

void PlanFlightDialog::on_cbPlot_toggled(bool checked) {
    if (Window::instance(false) != 0) {
        Window::instance()->mapScreen->glWidget->invalidatePilots();
    }
    lblPlotStatus->setVisible(checked);
    linePlotStatus->setVisible(checked);
}

void PlanFlightDialog::on_pbCopyToClipboard_clicked() {
    if (selectedRoute != 0) {
        QApplication::clipboard()->setText(selectedRoute->route);
    }
}

void PlanFlightDialog::on_pbVatsimPrefile_clicked() {
    if (selectedRoute != 0) {
        QUrl url = QUrl(
            QString("http://www.vatsim.net/fp/?1=I&5=%1&9=%2&8=%3&voice=/V/")
            .arg(selectedRoute->dep, selectedRoute->dest, selectedRoute->route)
            , QUrl::TolerantMode
        );
        if (url.isValid()) {
            if (!QDesktopServices::openUrl(url)) {
                QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("Could not invoke browser"));
            }
        } else {
            QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("URL %1 is invalid").arg(url.toString()));
        }
    }
}

void PlanFlightDialog::closeEvent(QCloseEvent* event) {
    Settings::setDialogPreferences(
        m_preferencesName,
        Settings::DialogPreferences {
            .size = size(),
            .pos = pos(),
            .geometry = saveGeometry()
        }
    );
    event->accept();
}
