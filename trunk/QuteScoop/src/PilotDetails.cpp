/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "PilotDetails.h"
#include "NavData.h"
#include "Window.h"
#include "Whazzup.h"

//singleton instance
PilotDetails *pilotDetails = 0;
PilotDetails* PilotDetails::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(pilotDetails == 0) {
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::getInstance(true);
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
    pilot(0)
{
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);

    connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
}

void PilotDetails::refresh(Pilot *newPilot) {
    if(newPilot != 0)
        pilot = newPilot;
    else
        pilot = Whazzup::getInstance()->whazzupData().findPilot(callsign);
    if (pilot == 0) {
        hide();
        return;
    }
    setMapObject(pilot);
    setWindowTitle(pilot->toolTip());

    // Pilot Information
    lblPilotInfo->setText(QString("<strong>%1</strong>%2")
                        .arg(pilot->displayName(true))
                        .arg(pilot->detailInformation().isEmpty() ? "" : ", " + pilot->detailInformation()));
    if (pilot->server.isEmpty())
        lblConnected->setText(QString("<i>not connected</i>"));
    else
        lblConnected->setText(QString("on %1 for %2%3")
                     .arg(pilot->server)
                     .arg(pilot->onlineTime())
                     .arg(pilot->clientInformation().isEmpty()? "": ", "+ pilot->clientInformation()));
    buttonShowOnMap->setEnabled(pilot->lat != 0 || pilot->lon != 0);


    // Aircraft Information
    lblAircraft->setText(QString("%1").arg(pilot->planAircraft));
    lblAltitude->setText(QString("%1 ft").arg(pilot->altitude));
    lblGroundspeed->setText(QString("%1 kts").arg(pilot->groundspeed));

    // flight status
    groupStatus->setTitle(QString("Status: %1").arg(pilot->flightStatusShortString()));
    lblFlightStatus->setText(pilot->flightStatusString());

    // flight plan
    groupFp->setVisible(!pilot->planFlighttype.isEmpty()); // hide for Bush pilots

    groupFp->setTitle(QString("Flightplan (%1)")
                      .arg(pilot->planFlighttypeString()));

    buttonFrom->setEnabled(pilot->depAirport()   != 0);
    buttonFrom->setText(   pilot->depAirport()   != 0? pilot->depAirport() ->toolTip(): pilot->planDep);
    buttonDest->setEnabled(pilot->destAirport()  != 0);
    buttonDest->setText(   pilot->destAirport()  != 0? pilot->destAirport()->toolTip(): pilot->planDest);
    buttonAlt->setEnabled( pilot->altAirport()   != 0);
    buttonAlt-> setText(   pilot->altAirport()   != 0? pilot->altAirport() ->toolTip(): pilot->planAltAirport);

    lblPlanEtd->setText(pilot->etd().toString("HHmm"));
    lblPlanEta->setText(pilot->etaPlan().toString("HHmm"));
    lblFuel->setText(QTime(pilot->planHrsFuel, pilot->planMinFuel).toString("H:mm"));
    lblRoute->setText(pilot->planRoute);
    lblPlanTas->setText(QString("N%1").arg(pilot->planTasInt()));
    lblPlanFl->setText(QString("F%1").arg(pilot->defuckPlanAlt(pilot->planAlt)/100));
    lblPlanEte->setText(QString("%1").arg(QTime(pilot->planHrsEnroute, pilot->planMinEnroute).toString("H:mm")));
    lblRemarks->setText(pilot->planRemarks);

    // check if we know userId
    buttonAddFriend->setDisabled(pilot->userId.isEmpty());
    buttonAddFriend->setText(pilot->isFriend()? "remove &friend": "add &friend");

    // check if we know position
    buttonShowOnMap->setDisabled(qFuzzyIsNull(pilot->lon) && qFuzzyIsNull(pilot->lat));

    // plotted?
    bool plottedAirports = false;
    if (pilot->depAirport() != 0)
        plottedAirports |= pilot->depAirport()->showFlightLines;
    if (pilot->destAirport() != 0)
        plottedAirports |= pilot->destAirport()->showFlightLines;

    if (!plottedAirports && !pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::Unchecked);
    if (plottedAirports && !pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::PartiallyChecked);
    if (pilot->showDepDestLine)
        cbPlotRoute->setCheckState(Qt::Checked);
    if (pilot->showDepDestLine || plottedAirports)
        lblPlotStatus->setText(QString("waypoints (calculated): %1").arg(pilot->routeWaypointsStr()));
    lblPlotStatus->setVisible(pilot->showDepDestLine || plottedAirports);

    adjustSize();
}

void PilotDetails::on_buttonDest_clicked() {
    if(pilot->destAirport() != 0)
        pilot->destAirport()->showDetailsDialog();
}

void PilotDetails::on_buttonAlt_clicked() {
    if(pilot->altAirport() != 0)
        pilot->altAirport()->showDetailsDialog();
}

void PilotDetails::on_buttonFrom_clicked() {
    if(pilot->depAirport() != 0)
        pilot->depAirport()->showDetailsDialog();
}

void PilotDetails::on_buttonAddFriend_clicked() {
    friendClicked();
    refresh();
}

void PilotDetails::on_cbPlotRoute_clicked(bool checked) {
    bool plottedAirports = false; // plotted?
    if (pilot->depAirport() != 0)
        plottedAirports |= pilot->depAirport()->showFlightLines;
    if (pilot->destAirport() != 0)
        plottedAirports |= pilot->destAirport()->showFlightLines;

    if (pilot->showDepDestLine != checked) {
        pilot->showDepDestLine = checked;
        if (Window::getInstance(false) != 0) {
            Window::getInstance(true)->glWidget->createPilotsList();
            Window::getInstance(true)->glWidget->updateGL();
        }
        refresh();
    }
}
