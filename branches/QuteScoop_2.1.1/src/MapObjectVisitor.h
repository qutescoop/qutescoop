/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef MAPOBJECTVISITOR_H_
#define MAPOBJECTVISITOR_H_

#include <QList>
#include "MapObject.h"

class MapObjectVisitor
{
public:
	virtual ~MapObjectVisitor() {}

	virtual void visit(MapObject* object) = 0;
	virtual QList<MapObject*> result() const = 0;
};

#endif /*MAPOBJECTVISITOR_H_*/
