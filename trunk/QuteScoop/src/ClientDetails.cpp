/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ClientDetails.h"

#include "Settings.h"
#include "Client.h"
#include "Window.h"

ClientDetails::ClientDetails(QWidget *parent):
        QDialog(parent)
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
    if ((lat != 0 || lon != 0) && Window::getInstance(false)) {
        Window::getInstance(true)->glWidget->setMapPosition(lat, lon, 0.1);
    }
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
    qobject_cast<Window *>(this->parent())->refreshFriends();
}
