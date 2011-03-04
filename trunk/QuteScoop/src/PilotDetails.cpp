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
    setWindowTitle(pilot->toolTip());

    // Pilot Information
    lblPilotInfo->setText(QString("<strong>%1</strong>%2")
                        .arg(pilot->displayName(true))
                        .arg(pilot->detailInformation().isEmpty() ? "" : ", " + pilot->detailInformation()));
    if (pilot->server.isEmpty()) {
        lblConnected->setText(QString("<i>not connected</i>"));
    } else {
        lblConnected->setText(QString("on %1 for %2%3")
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
    groupStatus->setTitle(QString("Status: %1").arg(pilot->flightStatusShortString()));
    lblFlightStatus->setText(pilot->flightStatusString());

    // flight plan
    groupFp->setVisible(!pilot->planFlighttype.isEmpty()); // hide for Bush pilots

    groupFp->setTitle(QString("Flightplan (%1)")
                      .arg(pilot->planFlighttypeString()));

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
        buttonAddFriend->setText("remove &friend");
    else
        buttonAddFriend->setText("add &friend");

    // check if we know position
    buttonShowOnMap->setDisabled(qFuzzyIsNull(pilot->lon) && qFuzzyIsNull(pilot->lat));

    cbPlotRoute->setChecked(pilot->showDepDestLine);

    adjustSize();
    //updateGeometry();
}

void PilotDetails::on_buttonDest_clicked() {
    Airport *airport = pilot->destAirport();
    if(airport != 0) {
        airport->showDetailsDialog();
        close();
    }
}

void PilotDetails::on_buttonAlt_clicked() {
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

void PilotDetails::on_cbPlotRoute_toggled(bool checked) {
    if (pilot->showDepDestLine != checked) {
        pilot->showDepDestLine = checked;
        if (Window::getInstance(false) != 0) {
            Window::getInstance(true)->glWidget->createPilotsList();
            Window::getInstance(true)->glWidget->updateGL();
        }
    }
}
