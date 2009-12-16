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
