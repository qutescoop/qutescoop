/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef MAPOBJECT_H_
#define MAPOBJECT_H_

#include "_pch.h"

#include "ClientDetails.h"

class MapObject: public QObject {
        Q_OBJECT
    public:
        MapObject();
        MapObject(const MapObject& obj);
        MapObject& operator=(const MapObject& obj);
        virtual ~MapObject();

        bool isNull() const { return label.isNull(); }

        virtual bool matches(const QRegExp& regex) const { return label.contains(regex); }
        virtual QString mapLabel() const { return label; }
        virtual QString toolTip() const { return label; }

        virtual void showDetailsDialog() {return;}

        double lat, lon;
        QString label;
        bool drawLabel;
};

#endif /*MAPOBJECT_H_*/
