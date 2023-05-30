#include "Client.h"
#include "Controller.h"
#include "FriendsVisitor.h"
#include "Settings.h"

FriendsVisitor::FriendsVisitor() {
    _friendList = Settings::friends();
}

void FriendsVisitor::visit(MapObject* object) {
    Client* c = dynamic_cast<Client*>(object);
    if(c == 0) {
        return;
    }
    if(!c->isFriend()) {
        return;
    }
    Controller* co = dynamic_cast<Controller*>(object);
    if(co != 0) {
        if(co->isAtis()) {
            return;
        }
    }
    _friends.append(c);
}

QList<MapObject*> FriendsVisitor::result() const {
    return _friends;
}
