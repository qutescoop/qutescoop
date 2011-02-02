/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef HELPERS_H_
#define HELPERS_H_

#include <math.h>
#include <QGLWidget>
#include <QString>

// version information
#define CVS_REVISION "$Revision$" // Gets set automatically on commit of THIS file.
                                        // This is just the revision of THIS file, not the whole working copy.
                                        // Well, better than nothing. No working copy revision information is available cross-platform :(

#define VERSION_NUMBER "1.0.3 beta 5"
#define VERSION_STRING QString("QuteScoop %1 - %2").arg(VERSION_NUMBER, CVS_REVISION)

// mathematical constants
const GLdouble Pi = 3.14159265358979323846;
const GLdouble Pi180 = Pi/180.0;

#define SX(lat, lon) (cos((lat) * Pi180) * sin((lon) * Pi180))
#define SY(lat, lon) (-sin((lat) * Pi180))
#define SZ(lat, lon) (cos((lat) * Pi180) * cos((lon) * Pi180))

#define Nm2Deg(miles) (miles / 60.0)

#define VERTEX(lat, lon) glVertex3f(SX(lat, lon), SY(lat, lon), SZ(lat, lon))

#endif /*HELPERS_H_*/
