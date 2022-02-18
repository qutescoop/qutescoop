/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SECTOR_H_
#define SECTOR_H_

#include "_pch.h"


class Sector {
    public:
        Sector() :
            icao(), name(), id(), _polygon(0), _borderline(0)
        {}
        Sector(QStringList strings);
        ~Sector();

        bool isNull() const { return icao.isNull(); }

        const QPolygonF sectorPolygon() const;

        QList<QPair<double, double> > points;
        QString icao, name, id;

        GLuint glPolygon();
        GLuint glBorderLine();

        QPair<double, double> getCenter() const;
    private:
        GLuint _polygon, _borderline;
};

#endif /*SECTOR_H_*/
