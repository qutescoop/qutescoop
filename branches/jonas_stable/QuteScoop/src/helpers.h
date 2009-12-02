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

// VERSION_INT is used as QMainWindow::saveState() and restoreState()
#define VERSION_INT         111

#define VERSION_NUMBER "1.0.3 alpha jonasmarkus 7"
#define VERSION_STRING QString("QuteScoop %1").arg(VERSION_NUMBER)

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
