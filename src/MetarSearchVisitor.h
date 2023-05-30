#ifndef METARSEARCHVISITOR_H_
#define METARSEARCHVISITOR_H_

#include "SearchVisitor.h"

class MetarSearchVisitor : public SearchVisitor {
    public:
        MetarSearchVisitor(const QString& search): SearchVisitor(search) {}
        virtual void visit(MapObject *object);
        QList<Airport*> airports() const;

        virtual QList<MapObject*> result() const;
private:
        QHash<QString, Airport*> _airportMap;
};

#endif /*METARSEARCHVISITOR_H_*/
