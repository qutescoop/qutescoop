/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SEARCHVISITOR_H_
#define SEARCHVISITOR_H_

#include <QHash>

#include "MapObjectVisitor.h"
#include "Controller.h"
#include "BookedController.h"
#include "Pilot.h"

class SearchVisitor : public MapObjectVisitor
{
public:
	SearchVisitor(const QString& search);
	virtual void visit(MapObject *object);
	virtual QList<MapObject*> result() const;
		
protected:
	QRegExp regex;
	
	QHash<QString, Pilot*> pilots;
	QHash<QString, Controller*> controllers, observers;
	QHash<QString, MapObject*> others;
};

#endif /*SEARCHVISITOR_H_*/
