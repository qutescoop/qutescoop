#ifndef METARSEARCHVISITOR_H_
#define METARSEARCHVISITOR_H_

#include "SearchVisitor.h"

class MetarSearchVisitor
    : public SearchVisitor {
    public:
        MetarSearchVisitor(const QString& search)
            : SearchVisitor(search) {}
        virtual void visit(MapObject* object) override;
        virtual QList<MapObject*> result() const override;

        QList<Airport*> airports() const;
    private:
        QHash<QString, Airport*> _airportMap;
};

#endif /*METARSEARCHVISITOR_H_*/
