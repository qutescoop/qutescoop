/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ClientDetails.h"

#include "Settings.h"
#include "Client.h"
#include "Window.h"

ClientDetails::ClientDetails(QWidget *parent):
        QDialog(parent) {
    setModal(false);
}

void ClientDetails::setMapObject(MapObject *object) {
    _lat = object->lat;
    _lon = object->lon;
    Client *c = dynamic_cast<Client*>(object);
    if(c != 0) {
        userId = c->userId;
        callsign = c->label;
    } else {
        userId = QString();
        callsign = QString();
    }
}

void ClientDetails::showOnMap() const {
    if ((!qFuzzyIsNull(_lat) || !qFuzzyIsNull(_lon)) && Window::instance(false) != 0)
        Window::instance()->mapScreen->glWidget->setMapPosition(_lat, _lon, .06);
}

void ClientDetails::friendClicked() const {
    if(!userId.isEmpty()) {
        if(Settings::friends().contains(userId)) // was friend, remove
            Settings::removeFriend(userId);
        else // new friend
            Settings::addFriend(userId);
        if (Window::instance(false) != 0)
            Window::instance()->refreshFriends();
    }
}
