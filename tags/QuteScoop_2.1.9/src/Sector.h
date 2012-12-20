/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SECTOR_H_
#define SECTOR_H_

#include "_pch.h"


class Sector {
    public:
        Sector() :
            icao(), name(), countryCode(), id(), lat(0.), lon(0.), _polygon(0), _borderline(0)
        {}
        Sector(QStringList strings);
        ~Sector();

        bool isNull() const { return icao.isNull(); }

        const QPolygonF sectorPolygon() const;

        QList<QPair<double, double> > points;
        QString icao, name, countryCode, id;
        double lat, lon;

        GLuint getGlPolygon();
        GLuint getGlBorderLine();
    private:
        GLuint _polygon, _borderline;
};

#endif /*SECTOR_H_*/
