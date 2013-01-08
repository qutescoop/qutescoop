/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "PilotDetails.h"
#include "NavData.h"
#include "Window.h"
#include "Whazzup.h"

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
//    setWindowFlags(Qt::Tool);

    connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
}

void PilotDetails::refresh(Pilot *pilot) {
    if (!Settings::pilotDetailsSize().isNull()) resize(Settings::pilotDetailsSize());
    if (!Settings::pilotDetailsPos().isNull()) move(Settings::pilotDetailsPos());
    if (!Settings::pilotDetailsGeometry().isNull()) restoreGeometry(Settings::pilotDetailsGeometry());

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
    lblPilotInfo->setText(QString("<strong>%1</strong>%2")
                        .arg(_pilot->displayName(true))
                        .arg(_pilot->detailInformation().isEmpty() ? "" : ", " + _pilot->detailInformation()));
    if (_pilot->server.isEmpty())
        lblConnected->setText(QString("<i>not connected</i>"));
    else
        lblConnected->setText(QString("on %1 for %2%3")
                     .arg(_pilot->server)
                     .arg(_pilot->onlineTime())
                     .arg(_pilot->clientInformation().isEmpty()? "": ", "+ _pilot->clientInformation()));
    buttonShowOnMap->setEnabled(_pilot->lat != 0 || _pilot->lon != 0);


    // Aircraft Information
    lblAircraft->setText(QString("%1").arg(_pilot->planAircraft));
    lblAirline->setText(_pilot->airline);
    lblAltitude->setText(QString("%1 ft").arg(_pilot->altitude));
    lblGroundspeed->setText(QString("%1 kts").arg(_pilot->groundspeed));
    lblSquwak->setText(QString("%1").arg(_pilot->transponder));

    // flight status
    groupStatus->setTitle(QString("Status: %1").arg(_pilot->flightStatusShortString()));
    lblFlightStatus->setText(_pilot->flightStatusString());

    // flight plan
    groupFp->setVisible(!_pilot->planFlighttype.isEmpty()); // hide for Bush pilots

    groupFp->setTitle(QString("Flightplan (%1)")
                      .arg(_pilot->planFlighttypeString()));

    buttonFrom->setEnabled(_pilot->depAirport()   != 0);
    buttonFrom->setText(   _pilot->depAirport()   != 0? _pilot->depAirport() ->toolTip(): _pilot->planDep);
    buttonDest->setEnabled(_pilot->destAirport()  != 0);
    buttonDest->setText(   _pilot->destAirport()  != 0? _pilot->destAirport()->toolTip(): _pilot->planDest);
    buttonAlt->setEnabled( _pilot->altAirport()   != 0);
    buttonAlt-> setText(   _pilot->altAirport()   != 0? _pilot->altAirport() ->toolTip(): _pilot->planAltAirport);

    lblPlanEtd->setText(_pilot->etd().toString("HHmm"));
    lblPlanEta->setText(_pilot->etaPlan().toString("HHmm"));
    lblFuel->setText(QTime(_pilot->planFuel_hrs, _pilot->planFuel_mins).toString("H:mm"));
    lblRoute->setText(_pilot->planRoute);
    lblPlanTas->setText(QString("N%1").arg(_pilot->planTasInt()));
    lblPlanFl->setText(QString("F%1").arg(_pilot->defuckPlanAlt(_pilot->planAlt)/100));
    lblPlanEte->setText(QString("%1").arg(QTime(_pilot->planEnroute_hrs, _pilot->planEnroute_mins).toString("H:mm")));
    lblRemarks->setText(_pilot->planRemarks);

    // check if we know userId
    buttonAddFriend->setDisabled(_pilot->userId.isEmpty());
    buttonAddFriend->setText(_pilot->isFriend()? "remove &friend": "add &friend");

    // check if we know position
    buttonShowOnMap->setDisabled(qFuzzyIsNull(_pilot->lon) && qFuzzyIsNull(_pilot->lat));

    // plotted?
    bool plottedAirports = false;
    if (_pilot->depAirport() != 0)
        plottedAirports |= _pilot->depAirport()->showFlightLines;
    if (_pilot->destAirport() != 0)
        plottedAirports |= _pilot->destAirport()->showFlightLines;

    if (!plottedAirports && !_pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::Unchecked);
    if (plottedAirports && !_pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::PartiallyChecked);
    if (_pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::Checked);
    if (_pilot->showDepDestLine || plottedAirports)
        lblPlotStatus->setText(QString("waypoints (calculated): %1").arg(_pilot->routeWaypointsStr()));
    lblPlotStatus->setVisible(_pilot->showDepDestLine || plottedAirports);

    adjustSize();
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
    bool plottedAirports = false; // plotted?
    if (_pilot->depAirport() != 0)
        plottedAirports |= _pilot->depAirport()->showFlightLines;
    if (_pilot->destAirport() != 0)
        plottedAirports |= _pilot->destAirport()->showFlightLines;

    if (_pilot->showDepDestLine != checked) {
        _pilot->showDepDestLine = checked;
        if (Window::instance(false) != 0) {
            Window::instance()->mapScreen->glWidget->createPilotsList();
            Window::instance()->mapScreen->glWidget->updateGL();
        }
        refresh();
    }
}

void PilotDetails::closeEvent(QCloseEvent *event) {
    Settings::setPilotDetailsPos(pos());
    Settings::setPilotDetailsSize(size());
    Settings::setPilotDetailsGeometry(saveGeometry());
    event->accept();
}
