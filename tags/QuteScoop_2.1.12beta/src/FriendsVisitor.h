/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef FRIENDSVISITOR_H_
#define FRIENDSVISITOR_H_

#include <QStringList>
#include <QList>

#include "MapObjectVisitor.h"
#include "MapObject.h"

class FriendsVisitor : public MapObjectVisitor {
    public:
        FriendsVisitor();
        virtual void visit(MapObject *object);
        virtual QList<MapObject*> result() const { return _friends; }

    private:
        QStringList _friendList;
        QList<MapObject*> _friends;
};

#endif /*FRIENDSVISITOR_H_*/
