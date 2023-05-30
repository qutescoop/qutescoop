#include "ClientDetails.h"

#include "Window.h"
#include "../Client.h"
#include "../Settings.h"

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
        if(Settings::friends().contains(userId))
            Settings::removeFriend(userId);
        else
            Settings::addFriend(userId);

        if (Window::instance(false) != 0) {
            Window::instance()->refreshFriends();
            Window::instance()->mapScreen->glWidget->newWhazzupData(true);
        }
    }
}
