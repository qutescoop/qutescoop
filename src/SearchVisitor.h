#ifndef SEARCHVISITOR_H_
#define SEARCHVISITOR_H_

#include "Airline.h"
#include "Controller.h"
#include "MapObject.h"
#include "MapObjectVisitor.h"
#include "Pilot.h"

#include <QHash>

class SearchVisitor
    : public MapObjectVisitor {
    public:
        SearchVisitor(const QString& search);
        virtual void visit(MapObject* object) override;
        virtual QList<MapObject*> result() const override;
        QHash<QString, Airline*> airlines;
    protected:
        QList<MapObject*> _resultFromVisitors;
        QRegExp _regex;
        QHash<QString, Pilot*> _pilots;
        QHash<QString, Controller*> _controllers, _observers;
        QHash<QString, MapObject*> _others;
};

#endif /*SEARCHVISITOR_H_*/
