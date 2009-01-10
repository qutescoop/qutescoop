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
	
	connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
	connect(this, SIGNAL(showOnMap(double, double)), Window::getInstance(), SLOT(showOnMap(double, double)));
}

void PilotDetails::refresh(Pilot *newPilot) {
	if(newPilot != 0) {
		pilot = newPilot;
	} else {
		pilot = Whazzup::getInstance()->whazzupData().getPilot(callsign);
	}
	if(pilot == 0) return;
	setMapObject(pilot);

	setWindowTitle(pilot->label);
	
	// Pilot Information
	QString pilotInfo = QString("<strong>PILOT: %1</strong><br>").arg(pilot->displayName(true));
	pilotInfo += pilot->detailInformation() + "<br>";
	pilotInfo += QString("On %3 for %4").arg(pilot->server).arg(pilot->onlineTime());
	QString software = pilot->clientInformation();
	if(!software.isEmpty()) pilotInfo += ", " + software;
	lblPilotInfo->setText(pilotInfo);
	
	// Aircraft Information
	QString aircraftInfo = QString("Aircraft: <strong>%1</strong><br>").arg(pilot->planAircraft);
	aircraftInfo += QString("Altitude: <strong>%1</strong> feet<br>").arg(pilot->altitude);
	aircraftInfo += QString("Ground Speed: <strong>%1</strong> kts").arg(pilot->groundspeed);
	lblAircraftInfo->setText(aircraftInfo);
	
	// flight status
	lblFlightStatus->setText(pilot->flightStatusString());
	
	QString depStr = pilot->planDep;
	Airport *airport = pilot->depAirport();
	if(airport != 0) depStr = airport->toolTip();
	buttonFrom->setText(depStr);
	
	QString destStr = pilot->planDest;
	airport = pilot->destAirport();
	if(airport != 0) destStr = airport->toolTip();
	buttonDest->setText(destStr);

	// do something about alternate here
	
	lblRoute->setText(pilot->planRoute);
	lblRemarks->setText(pilot->planRemarks);
	
	if(pilot->isFriend())
		buttonAddFriend->setText("Remove Friend");
	else
		buttonAddFriend->setText("Add Friend");
}

void PilotDetails::on_buttonDest_clicked() {
	Airport *airport = pilot->destAirport();
	if(airport != 0)
		airport->showDetailsDialog();
	close();
}

void PilotDetails::on_buttonFrom_clicked() {
	Airport *airport = pilot->depAirport();
	if(airport != 0)
		airport->showDetailsDialog();
	close();
}

void PilotDetails::on_buttonAddFriend_clicked() {
	friendClicked();
	refresh();
}