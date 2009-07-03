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

#include <QMessageBox>

#include "ControllerDetails.h"
#include "Window.h"
#include "Settings.h"

ControllerDetails *controllerDetailsInstance = 0;

ControllerDetails* ControllerDetails::getInstance() {
	if(controllerDetailsInstance == 0) {
		controllerDetailsInstance = new ControllerDetails();
	}
	return controllerDetailsInstance;
}

ControllerDetails::ControllerDetails():
	ClientDetails(),
	controller(0)
{
	setupUi(this);
    setWindowFlags(Qt::Tool);

	connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
	connect(this, SIGNAL(showOnMap(double, double)), Window::getInstance(), SLOT(showOnMap(double, double)));
}

void ControllerDetails::refresh(Controller *newController) {
	if(newController != 0) {		
		controller = newController;
	} else {
		controller = Whazzup::getInstance()->whazzupData().getController(callsign);
	}
	if(controller == 0) return;
	setMapObject(controller);
	
	setWindowTitle(controller->label);

	// Controller Information
	QString controllerInfo = QString("<strong>ATC: %1</strong><br>").arg(controller->displayName(true));
	
	QString details = controller->detailInformation();
	if(!details.isEmpty()) controllerInfo += details + "<br>";
	
	controllerInfo += QString("On %3 for %4")
                      .arg(controller->server)
                      .arg(controller->onlineTime());
	QString software = controller->clientInformation();
	if(!software.isEmpty()) controllerInfo += ", " + software;
	lblControllerInfo->setText(controllerInfo);

	QString stationInfo = controller->facilityString();
	if(!controller->isObserver() && !controller->frequency.length() == 0) 
		stationInfo += QString(" on frequency %1<br>Voice: %2")
		.arg(controller->frequency)
		.arg(controller->voiceServer);
	lblStationInformatoin->setText(stationInfo);

    lblAtis->setText(controller->atisMessage.split("^$").join("\n"));
	
	if(controller->isFriend())
		buttonAddFriend->setText("Remove Friend");
	else
		buttonAddFriend->setText("Add Friend");
	
	btnJoinChannel->setVisible(Settings::voiceType() != Settings::NONE);
	btnJoinChannel->setEnabled(!controller->voiceLink().isEmpty());
    
    // check if we know UserId
    buttonAddFriend->setDisabled(controller->userId.isEmpty());
    
    // check if we know position
    buttonShowOnMap->setDisabled(controller->lat == 0 && controller->lon == 0);
}

void ControllerDetails::on_buttonAddFriend_clicked() {
	friendClicked();
	refresh();
}

void ControllerDetails::on_btnJoinChannel_clicked() {
	if(controller == 0) return;
	if(Settings::voiceType() == Settings::NONE) return;
	
	if(Settings::voiceUser().isEmpty()) {
		QMessageBox::information(this, tr("Voice Settings"),
				"You have to enter your network user ID and password in the preferences before you can join a TeamSpeak channel.");
		return;
	}
	
#ifdef Q_WS_WIN
	QString program = "start";
#else
	QString program = "open";
#endif
	
	QString command = program + " " + controller->voiceLink();	
    int ret = system(command.toAscii());
    qDebug() << program << "returned" << ret;
}
