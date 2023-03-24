/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SEARCHVISITOR_H_
#define SEARCHVISITOR_H_

#include <QHash>

#include "MapObjectVisitor.h"
#include "MapObject.h"
#include "Controller.h"
#include "Pilot.h"
#include "Airline.h"

class SearchVisitor : public MapObjectVisitor {
    public:
        SearchVisitor(const QString& search);
        virtual void visit(MapObject *object);
        virtual QList<MapObject*> result() const;
        QHash<QString, Airline*> airlines;
    protected:
        QList<MapObject*> _resultFromVisitors;
        QRegExp _regex;
        QHash<QString, Pilot*> _pilots;
        QHash<QString, Controller*> _controllers, _observers;
        QHash<QString, MapObject*> _others;
};

#endif /*SEARCHVISITOR_H_*/
