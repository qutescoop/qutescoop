/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef HELPERS_H_
#define HELPERS_H_

#include "_pch.h"

// modulo, but always positive, not like fmod()
static double inline modPositive(double x, double y) {
    if (0 == y)
        return x;
    return x - y * floor(x/y);
}


/* version information */
#define CVS_REVISION "$Revision$" // Gets set automatically on commit of THIS file.
                                        // This is just the revision of THIS file, not the whole working copy.
                                        // Well, better than nothing. No working copy revision information is available cross-platform :(

#define VERSION_NUMBER "2.0rc6"
#define VERSION_STRING QString("QuteScoop %1 - %2").arg(VERSION_NUMBER, CVS_REVISION)

/* mathematical constants */
const GLdouble Pi = 3.14159265358979323846;
const GLdouble Pi180 = Pi/180.0;

/* 3D-calculations */
#define SX(lat, lon)  cos((lat) * Pi180) * sin((lon) * Pi180)
#define SY(lat, lon) -cos((lat) * Pi180) * cos((lon) * Pi180)
#define SZ(lat, lon) -sin((lat) * Pi180)
#define VERTEX(lat, lon) glVertex3f(SX(lat, lon), SY(lat, lon), SZ(lat, lon))
// higher VERTEX: 30km AGL (used for polygons to prevent intersecting with the globe)
#define SXhigh(lat, lon) SX(lat, lon) * 1.005
#define SYhigh(lat, lon) SY(lat, lon) * 1.005
#define SZhigh(lat, lon) SZ(lat, lon) * 1.005
#define VERTEXhigh(lat, lon) glVertex3f(SXhigh(lat, lon), SYhigh(lat, lon), SZhigh(lat, lon))

/* units */
#define Nm2Deg(miles) (miles / 60.0)


#endif /*HELPERS_H_*/
