#ifndef FRIENDSVISITOR_H_
#define FRIENDSVISITOR_H_

#include "MapObjectVisitor.h"
#include "MapObject.h"

#include <QStringList>

class FriendsVisitor: public MapObjectVisitor {
    public:
        FriendsVisitor();
        virtual void visit(MapObject* object);
        virtual QList<MapObject*> result() const;

    private:
        QStringList _friendList;
        QList<MapObject*> _friends;
};

#endif /*FRIENDSVISITOR_H_*/
