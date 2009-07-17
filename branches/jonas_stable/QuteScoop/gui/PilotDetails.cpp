/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
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

#include "PilotDetails.h"
#include "NavData.h"
#include "Window.h"

PilotDetails *pilotDetailsInstance = 0;

PilotDetails* PilotDetails::getInstance() {
    if(pilotDetailsInstance == 0) {
        pilotDetailsInstance = new PilotDetails();
    }
    return pilotDetailsInstance;
}

PilotDetails::PilotDetails():
    ClientDetails(),
    pilot(0)
{
    setupUi(this);
//    setWindowFlags(Qt::Tool);

    connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
    connect(this, SIGNAL(showOnMap(double, double)), Window::getInstance(), SLOT(showOnMap(double, double)));
}

void PilotDetails::refresh(Pilot *newPilot) {
    if(newPilot != 0) {
        pilot = newPilot;
    } else {
        pilot = Whazzup::getInstance()->whazzupData().getPilot(callsign);
    }
    if(pilot == 0) {
        hide();
        return;
    }
    setMapObject(pilot);

    setWindowTitle(pilot->label);

    // Pilot Information
    lblPilotInfo->setText(QString("<strong>PILOT: %1</strong>%2")
                        .arg(pilot->displayName(true))
                        .arg(pilot->detailInformation().isEmpty() ? "" : ", " + pilot->detailInformation()));
    if (pilot->server.isEmpty()) {
        lblConnected->setText(QString("<i>Not connected</i>"));
    } else {
        lblConnected->setText(QString("On %1 for %2%3")
                     .arg(pilot->server)
                     .arg(pilot->onlineTime())
                     .arg(pilot->clientInformation().isEmpty()? "": ", "+ pilot->clientInformation()));
    }
    buttonShowOnMap->setEnabled(pilot->lat != 0 || pilot->lon != 0);


    // Aircraft Information
    lblAircraft->setText(QString("%1").arg(pilot->planAircraft));
    lblAltitude->setText(QString("%1 ft").arg(pilot->altitude));
    lblGroundspeed->setText(QString("%1 kts").arg(pilot->groundspeed));

    // flight status
    groupStatus->setTitle(QString("Flight Status: %1").arg(pilot->flightStatusShortString()));
    lblFlightStatus->setText(pilot->flightStatusString());

    // flight plan
    groupFp->setVisible(!pilot->planFlighttype.isEmpty()); // hide for Bush pilots

    QString fpTypeStr;
    if (pilot->planFlighttype == "I")
        fpTypeStr = "IFR";
    else if (pilot->planFlighttype == "V")
        fpTypeStr = "VFR";
    else if (pilot->planFlighttype == "Y")
        fpTypeStr = "Y: IFR to VFR";
    else if (pilot->planFlighttype == "Z")
        fpTypeStr = "Z: VFR to IFR";
    else
        fpTypeStr =  pilot->planFlighttype;
    groupFp->setTitle(QString("Flight Plan (%1)")
                      .arg(fpTypeStr));

    QString depStr = pilot->planDep;
    Airport *airport = pilot->depAirport();
    if(airport != 0) depStr = airport->toolTip();
    //lblDep->setText(depStr);
    buttonFrom->setText(depStr);

    lblPlanEtd->setText(pilot->etd().toString("HHmm"));

    QString destStr = pilot->planDest;
    airport = pilot->destAirport();
    if(airport != 0) destStr = airport->toolTip();
    //lblDest->setText(destStr);
    buttonDest->setText(destStr);

    lblPlanEta->setText(pilot->etaPlan().toString("HHmm"));

    QString altStr = pilot->planAltAirport;
    airport = pilot->altAirport();
    if(airport != 0) altStr = airport->toolTip();
    //lblAlt->setText(altStr);
    buttonAlt->setText(altStr);

    lblFuel->setText(QTime(pilot->planHrsFuel, pilot->planMinFuel).toString("H:mm"));
    lblRoute->setText(pilot->planRoute);
    lblPlanTas->setText(QString("N%1").arg(pilot->planTasInt()));
    lblPlanFl->setText(QString("F%1").arg(pilot->defuckPlanAlt(pilot->planAlt)/100));
    lblPlanEte->setText(QString("%1").arg(QTime(pilot->planHrsEnroute, pilot->planMinEnroute).toString("H:mm")));

    lblRemarks->setText(pilot->planRemarks);

    // check if we know userId
    buttonAddFriend->setDisabled(pilot->userId.isEmpty());
    if(pilot->isFriend())
        buttonAddFriend->setText("Remove Friend");
    else
        buttonAddFriend->setText("Add Friend");

    // check if we know position
    buttonShowOnMap->setDisabled(pilot->lon == 0.0 && pilot->lat == 0.0);

    cbPlotRoute->setChecked(pilot->displayLineToDest);

    // adjust window, little tweaking //fixme, does not work perfectly but only on second refresh()
    //setUpdatesEnabled(false);
    adjustSize();
    //updateGeometry();
    //setUpdatesEnabled(true);
}

void PilotDetails::on_buttonDest_clicked() {
    Airport *airport = pilot->destAirport();
    if(airport != 0) {
        airport->showDetailsDialog();
        close();
    }
}

void PilotDetails::on_buttonAlt_clicked()
{
    Airport *airport = pilot->altAirport();
    if(airport != 0) {
        airport->showDetailsDialog();
        close();
    }
}

void PilotDetails::on_buttonFrom_clicked() {
    Airport *airport = pilot->depAirport();
    if(airport != 0) {
        airport->showDetailsDialog();
        close();
    }
}

void PilotDetails::on_buttonAddFriend_clicked() {
    friendClicked();
    refresh();
}

void PilotDetails::on_cbPlotRoute_toggled(bool checked)
{
    if (pilot->displayLineToDest != checked) {
        pilot->toggleDisplayPath();
        Window::getInstance()->updateGLPilots();
    }
}
