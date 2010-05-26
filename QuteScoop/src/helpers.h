/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#ifndef HELPERS_H_
#define HELPERS_H_

#include <math.h>
#include <QGLWidget>

// version information
//#define VERSION_INT 114 // VERSION_INT is used as QMainWindow::saveState() and restoreState()
#define CVS_REVISION "$Revision$" // Gets set automatically on commit of THIS file.
                                        // This is just the revision of THIS file, not the whole working copy.
                                        // Well, better than nothing. No working copy revision information is available cross-platform :(

#define VERSION_NUMBER "1.0.3 alpha 10"
#define VERSION_STRING QString("QuteScoop %1 - %2").arg(VERSION_NUMBER, CVS_REVISION)

// mathematical constants
const GLdouble Pi = 3.14159265358979323846;
const GLdouble Pi180 = Pi/180.0;

#define SX(lat, lon) (cos((lat) * Pi180) * sin((lon) * Pi180))
#define SY(lat, lon) (-sin((lat) * Pi180))
#define SZ(lat, lon) (cos((lat) * Pi180) * cos((lon) * Pi180))

#define Nm2Deg(miles) (miles / 60.0)

#define VERTEX(lat, lon) glVertex3f(SX(lat, lon), SY(lat, lon), SZ(lat, lon))

QString lat2str(double lat);
QString lon2str(double lon);
#endif /*HELPERS_H_*/
