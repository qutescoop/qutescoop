/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ClientDetails.h"
#include "Settings.h"
#include "Client.h"
#include "Window.h"

#include <QTimer>

ClientDetails::ClientDetails():
	QDialog(Window::getInstance())
{
	setModal(false);
}

void ClientDetails::setMapObject(MapObject *object) {
	lat = object->lat;
	lon = object->lon;
	Client *c = dynamic_cast<Client*>(object);
	if(c != 0) {
		userId = c->userId;
		callsign = c->label;
	} else {
		userId = QString();
		callsign = QString();
	}
}

void ClientDetails::showOnMap() {
	emit showOnMap(lat, lon);
}

void ClientDetails::friendClicked() {
	if(!userId.isEmpty()) {
		QStringList friends = Settings::friends();
		if(friends.contains(userId)) {
			// was friend, remove it
			Settings::removeFriend(userId);
		} else {
			// new friend
			Settings::addFriend(userId);
		}
	}
	Window::getInstance()->refreshFriends();
}
