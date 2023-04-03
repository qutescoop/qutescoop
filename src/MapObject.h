/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef MAPOBJECT_H_
#define MAPOBJECT_H_

#include <QtCore>

class MapObject: public QObject {
        Q_OBJECT
    public:
        MapObject();
        MapObject(QString label, QString toolTip);
        MapObject(const MapObject& obj);
        MapObject& operator=(const MapObject& obj);
        virtual ~MapObject();

        bool isNull() const { return label.isNull(); }

        virtual bool matches(const QRegExp& regex) const { return label.contains(regex); }
        virtual QString mapLabel() const { return label; }
        virtual QString toolTip() const { return _toolTip; }

        virtual void showDetailsDialog() {}

        double lat, lon;
        QString label;
        bool drawLabel;
    protected:
        QString _toolTip;
};

#endif /*MAPOBJECT_H_*/
