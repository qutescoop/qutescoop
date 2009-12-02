/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
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

#ifndef SECTOR_H_
#define SECTOR_H_

#include <QStringList>
#include <QString>
#include <QGLWidget>
#include <QList>
#include <QPair>
#include <QPolygon>

class Sector
{
public:
    Sector();
    Sector(QStringList strings);
    ~Sector();

	bool isNull() const;
	
	const QString& icao() const { return _icao; }
	const QString& name() const { return _name; }
	const QString& countryCode() const { return _countryCode; }
	const QString& id() const { return _id; }
	const double& lat() const { return _lat; }
	const double& lon() const { return _lon; }
    const QList<QPair<double, double> > sector() const { return _points; }
	
	void setPointList(const QList<QPair<double, double> >& points);

	GLuint getPolygon();
	GLuint getBorderLine();

    //Working on better way; Markus
    //QPair<double, double> equidistantPoint(); // finds a point which is suitable to draw the minimum circle that encloses the whole FIR
    //int maxDistanceFromCenter();

private:
	void compileDisplayLists();

	QList<QPair<double, double> > _points;
	QString _icao, _name, _countryCode, _id;
	double _lat, _lon;
	GLuint polygon, borderline;

    QPair<double, double> _equidistantPoint;
    int _maxDistFromCenter;
};

#endif /*SECTOR_H_*/
