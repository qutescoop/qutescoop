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

#ifndef FIR_H_
#define FIR_H_

#include <QStringList>
#include <QString>
#include <QGLWidget>
#include <QList>
#include <QPair>

class Fir
{
public:
	Fir();
	Fir(QStringList strings);
	~Fir();

	bool isNull() const;
	
	const QString& icao() const { return _icao; }
	const QString& name() const { return _name; }
	const QString& countryCode() const { return _countryCode; }
	const QString& id() const { return _id; }
	const double& lat() const { return _lat; }
	const double& lon() const { return _lon; }
	
	void setPointList(const QList<QPair<double, double> >& points);

	GLuint getPolygon();
	GLuint getBorderLine();

    int maxDistanceFromCenter();

private:
	void compileDisplayLists();

	QList<QPair<double, double> > _points;
	QString _icao, _name, _countryCode, _id;
	double _lat, _lon;
	GLuint polygon, borderline;

    int _maxDistFromCenter;
};

#endif /*FIR_H_*/
