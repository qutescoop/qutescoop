/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef METARSEARCHVISITOR_H_
#define METARSEARCHVISITOR_H_

#include "SearchVisitor.h"

class MetarSearchVisitor : public SearchVisitor
{
    public:
        MetarSearchVisitor(const QString& search): SearchVisitor(search) {}
        virtual void visit(MapObject *object);
        virtual QList<MapObject*> result();
        QList<Airport*> airports();

    private:
        QHash<QString, Airport*> _airportMap;
};

#endif /*METARSEARCHVISITOR_H_*/
