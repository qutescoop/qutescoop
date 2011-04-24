/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SEARCHVISITOR_H_
#define SEARCHVISITOR_H_

#include <QHash>

#include "MapObjectVisitor.h"
#include "MapObject.h"
#include "Controller.h"
#include "BookedController.h"
#include "Pilot.h"

class SearchVisitor : public MapObjectVisitor
{
public:
	SearchVisitor(const QString& search);
	virtual void visit(MapObject *object);
        virtual void checkAirlines(void);
	virtual QList<MapObject*> result() const;
        QHash<QString, QString> AirlineCodes;
		
protected:
	QRegExp regex;
	
	QHash<QString, Pilot*> pilots;
	QHash<QString, Controller*> controllers, observers;
	QHash<QString, MapObject*> others;
        QHash<QString, QString> otherStrings;
        QString searchString;
};

#endif /*SEARCHVISITOR_H_*/
