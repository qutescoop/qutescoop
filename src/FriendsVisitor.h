#ifndef FRIENDSVISITOR_H_
#define FRIENDSVISITOR_H_

#include "MapObjectVisitor.h"
#include "MapObject.h"

#include <QStringList>

class FriendsVisitor
    : public MapObjectVisitor {
    public:
        FriendsVisitor();

        virtual void visit(MapObject* object) override;
        virtual QList<MapObject*> result() const override;
    private:
        QList<MapObject*> m_friends;
};

#endif /*FRIENDSVISITOR_H_*/
