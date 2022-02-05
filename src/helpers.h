/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef HELPERS_H_
#define HELPERS_H_

#include "_pch.h"

// typedefs: needed to get the QPair template running inside foreach's
typedef QPair<double, double> DoublePair;

// modulo, but always positive, not like fmod()
static double inline modPositive(double x, double y) {
    if (qFuzzyIsNull(y))
        return x;
    return x - y * floor(x/y);
}

/* mathematical constants */
const double Pi180 = M_PI / 180.;

/* 3D-calculations */
#define SX(lat, lon) (GLfloat) qCos((lat) * Pi180) * (GLfloat) qSin((lon) * Pi180)
#define SY(lat, lon) (GLfloat) -qCos((lat) * Pi180) * (GLfloat) qCos((lon) * Pi180)
#define SZ(lat, lon) (GLfloat) -qSin((lat) * Pi180)
#define VERTEX(lat, lon) glVertex3f(SX(lat, lon), SY(lat, lon), SZ(lat, lon))
// higher VERTEX: 30km AGL (used for polygons to prevent intersecting with the globe)
#define SXhigh(lat, lon) SX(lat, lon) * 1.005
#define SYhigh(lat, lon) SY(lat, lon) * 1.005
#define SZhigh(lat, lon) SZ(lat, lon) * 1.005
#define VERTEXhigh(lat, lon) glVertex3f(SXhigh(lat, lon), SYhigh(lat, lon), SZhigh(lat, lon))

/* units */
#define Nm2Deg(miles) (miles / 60.0)


#endif /*HELPERS_H_*/
