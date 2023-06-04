#ifndef MAPOBJECTVISITOR_H_
#define MAPOBJECTVISITOR_H_

#include "MapObject.h"

#include <QList>

class MapObjectVisitor {
    public:
        virtual ~MapObjectVisitor();

        virtual void visit(MapObject* object) = 0;
        virtual QList<MapObject*> result() const = 0;
};

#endif /*MAPOBJECTVISITOR_H_*/
