#include "Client.h"
#include "FriendsVisitor.h"

FriendsVisitor::FriendsVisitor() {}

void FriendsVisitor::visit(MapObject* object) {
    Client* c = dynamic_cast<Client*>(object);
    if (c == 0) {
        return;
    }
    if (!c->isFriend()) {
        return;
    }

    MapObject* m = dynamic_cast<MapObject*>(object);
    if (m != 0) {
        m_friends.append(m);
    }
}

QList<MapObject*> FriendsVisitor::result() const {
    return m_friends;
}
