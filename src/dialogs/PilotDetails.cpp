#include "PilotDetails.h"

#include "Window.h"
#include "../Settings.h"
#include "../Whazzup.h"

//singleton instance
PilotDetails *pilotDetails = 0;
PilotDetails* PilotDetails::instance(bool createIfNoInstance, QWidget *parent) {
    if(pilotDetails == 0) {
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::instance();
            pilotDetails = new PilotDetails(parent);
        }
    }
    return pilotDetails;
}

// destroys a singleton instance
void PilotDetails::destroyInstance() {
    delete pilotDetails;
    pilotDetails = 0;
}

PilotDetails::PilotDetails(QWidget *parent):
        ClientDetails(parent),
        _pilot(0) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);

    connect(buttonShowOnMap, &QAbstractButton::clicked, this, &ClientDetails::showOnMap);

    auto preferences = Settings::dialogPreferences(m_preferencesName);
    if (!preferences.size.isNull()) { resize(preferences.size); }
    if (!preferences.pos.isNull()) { move(preferences.pos); }
    if (!preferences.geometry.isNull()) { restoreGeometry(preferences.geometry); }
}

void PilotDetails::refresh(Pilot *pilot) {
    if(pilot != 0)
        _pilot = pilot;
    else
        _pilot = Whazzup::instance()->whazzupData().findPilot(callsign);
    if (_pilot == 0) {
        hide();
        return;
    }
    setMapObject(_pilot);
    setWindowTitle(_pilot->toolTip());

    // Pilot Information
    lblPilotInfo->setText(
      QString("<strong>%1</strong>%2")
        .arg(
          _pilot->displayName(true),
          _pilot->detailInformation().isEmpty() ? "" : ", " + _pilot->detailInformation()
        )
    );
    if (_pilot->server.isEmpty()) {
        lblConnected->setText(QString("<i>not connected</i>"));
    } else {
        lblConnected->setText(QString("on %1 for %2").arg(_pilot->server, _pilot->onlineTime()));
    }

    buttonShowOnMap->setEnabled(_pilot->lat != 0 || _pilot->lon != 0);

    // Aircraft Information
    lblAircraft->setText(
        QString("<a href='https://contentzone.eurocontrol.int/aircraftperformance/default.aspx?ICAOFilter=%1'>%1</a>").arg(_pilot->planAircraft.toHtmlEscaped())
    );
    lblAircraft->setToolTip(
        QString("Opens performance data in web browser.<br>Raw data: %1 – FAA: %2").arg(_pilot->planAircraftFull, _pilot->planAircraftFaa)
    );

    if (_pilot->airline != 0) {
        lblAirline->setText(_pilot->airline->label());
        lblAirline->setToolTip(_pilot->airline->toolTip());
    } else {
        lblAirline->setText("n/a");
        lblAirline->setToolTip("");
    }

    lblAltitude->setText(_pilot->humanAlt());
    lblAltitude->setToolTip(
      QString("local QNH %1 inHg / %2 hPa (%3 ft)")
        .arg(_pilot->qnh_inHg)
        .arg(_pilot->qnh_mb)
        .arg(_pilot->altitude)
    );

    lblGroundspeed->setText(QString("%1 kt").arg(_pilot->groundspeed));

    if (_pilot->transponderAssigned != "0000" && _pilot->transponderAssigned != "") {
        if (_pilot->transponderAssigned != _pilot->transponder) {
            lblSquawk->setText(QString("%1 !").arg(_pilot->transponder, _pilot->transponderAssigned));
            lblSquawk->setToolTip(QString("ATC assigned: %1").arg(_pilot->transponderAssigned));
        } else {
            lblSquawk->setText(QString("%1 =").arg(_pilot->transponder));
            lblSquawk->setToolTip("ATC assigned squawk set");
        }
    } else {
        lblSquawk->setText(QString("%1").arg(_pilot->transponder));
        lblSquawk->setToolTip("no ATC assigned squawk");
    }


    // flight status
    groupStatus->setTitle(QString("Status: %1").arg(_pilot->flightStatusShortString()));
    lblFlightStatus->setText(_pilot->flightStatusString());

    // flight plan
    groupFp->setVisible(!_pilot->planFlighttype.isEmpty()); // hide for Bush pilots

    groupFp->setTitle(QString("Flightplan (%1)")
                      .arg(_pilot->planFlighttypeString()));

    buttonFrom->setEnabled(_pilot->depAirport() != 0);
    buttonFrom->setText(   _pilot->depAirport() != 0? _pilot->depAirport()->toolTip(): _pilot->planDep);
    buttonDest->setEnabled(_pilot->destAirport() != 0);
    buttonDest->setText(   _pilot->destAirport() != 0? _pilot->destAirport()->toolTip(): _pilot->planDest);
    buttonAlt->setEnabled( _pilot->altAirport() != 0);
    buttonAlt->setText(   _pilot->altAirport() != 0? _pilot->altAirport()->toolTip(): _pilot->planAltAirport);

    lblPlanEtd->setText(_pilot->etd().toString("HHmm"));
    lblPlanEta->setText(_pilot->etaPlan().toString("HHmm"));
    lblFuel->setText(QTime(_pilot->planFuel_hrs, _pilot->planFuel_mins).toString("H:mm"));

    static QRegularExpression routeSlashAmendRe("([^/ ]+)/([^/ ]+)");
    lblRoute->setText(
        QString("<code>%1</code>").arg(
            _pilot->planRoute.toHtmlEscaped().trimmed().replace(
                routeSlashAmendRe,
                "\\1<span style='color: " + Settings::lightTextColor().name(QColor::HexArgb) + "'>/<small>\\2</small></span>"
            )
        )
    );

    lblPlanTas->setText(QString("N%1").arg(_pilot->planTasInt()));
    lblPlanFl->setText(QString("F%1").arg(_pilot->defuckPlanAlt(_pilot->planAlt)/100));
    lblPlanEte->setText(QString("%1").arg(QTime(_pilot->planEnroute_hrs, _pilot->planEnroute_mins).toString("H:mm")));

    static QRegularExpression rmkSlashAmendRe("([ ]?[^ ]+)/");
    lblRemarks->setText(
        QString("<code>%1</code>").arg(
            _pilot->planRemarks.toHtmlEscaped().trimmed().replace(
                rmkSlashAmendRe,
                // we make use of the error-tolerant HTML-parser...
                "</small><b>\\1</b><span style='color: " + Settings::lightTextColor().name(QColor::HexArgb) + "'>/</span><small>"
            )
        )
    );

    // check if we know userId
    bool invalidID = !(_pilot->hasValidID());
    buttonAddFriend->setDisabled(invalidID);
    pbAlias->setDisabled(invalidID);
    buttonAddFriend->setText(_pilot->isFriend()? "remove &friend": "add &friend");

    // check if we know position
    buttonShowOnMap->setDisabled(qFuzzyIsNull(_pilot->lon) && qFuzzyIsNull(_pilot->lat));

    // plotted?
    bool plottedAirports = false;
    if (_pilot->depAirport() != 0)
        plottedAirports |= _pilot->depAirport()->showRoutes;
    if (_pilot->destAirport() != 0)
        plottedAirports |= _pilot->destAirport()->showRoutes;

    if (!plottedAirports && !_pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::Unchecked);
    if (plottedAirports && !_pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::PartiallyChecked);
    if (_pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::Checked);
    if (_pilot->showDepDestLine || plottedAirports)
        lblPlotStatus->setText(QString("<span style='color: " + Settings::lightTextColor().name(QColor::HexArgb) + "'>waypoints (calculated): <small><code>%1</code></small></span>").arg(_pilot->routeWaypointsStr()));
    lblPlotStatus->setVisible(_pilot->showDepDestLine || plottedAirports);

    // @see https://github.com/qutescoop/qutescoop/issues/124
    // adjustSize();
}

void PilotDetails::on_buttonDest_clicked() {
    if(_pilot->destAirport() != 0)
        _pilot->destAirport()->showDetailsDialog();
}

void PilotDetails::on_buttonAlt_clicked() {
    if(_pilot->altAirport() != 0)
        _pilot->altAirport()->showDetailsDialog();
}

void PilotDetails::on_buttonFrom_clicked() {
    if(_pilot->depAirport() != 0)
        _pilot->depAirport()->showDetailsDialog();
}

void PilotDetails::on_buttonAddFriend_clicked() {
    friendClicked();
    refresh();
}

void PilotDetails::on_cbPlotRoute_clicked(bool checked) {
    qDebug() << "PilotDetails::on_cbPlotRoute_clicked()" << checked;
    if (_pilot->showDepDestLine != checked) {
        _pilot->showDepDestLine = checked;
        if (Window::instance(false) != 0) {
            Window::instance()->mapScreen->glWidget->invalidatePilots();
        }
        refresh();
    }
    qDebug() << "PilotDetails::on_cbPlotRoute_clicked() -- finished";
}

void PilotDetails::closeEvent(QCloseEvent *event) {
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

void PilotDetails::on_pbAlias_clicked()
{
    if (_pilot->showAliasDialog(this)) {
        refresh();
    }
}

