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

#include <QDebug>
#include <QPair>

#include "Settings.h"
#include "Fir.h"
#include "Tessellator.h"
#include "NavData.h"
#include "helpers.h"

Fir::Fir() {
	polygon = 0;
	borderline = 0;
}

bool Fir::isNull() const {
	return _icao.isNull();
}

Fir::Fir(QStringList strings) {
	//LSAZ:Zurich:CH:46.9:9.1:189
	_icao = strings[0];
	_name = strings[1];
	_countryCode = strings[2];
	_lat = strings[3].toDouble();
	_lon = strings[4].toDouble();
	_id = strings[5];

    polygon = 0;
	borderline = 0;
    _maxDistFromCenter = 0;
}

Fir::~Fir() {
	if(polygon != 0) glDeleteLists(polygon, 1);
	if(borderline != 0) glDeleteLists(borderline, 1);
}

void Fir::compileDisplayLists() {
	// filled polygon
	polygon = glGenLists(1);
	glNewList(polygon, GL_COMPILE);
	QColor color = Settings::firFillColor();// was: .rgba(); // fixes a transparency bug for me - please test
	glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	Tessellator().tessellate(_points);
	glEndList();
	
	// border line
	borderline = glGenLists(1);
	glNewList(borderline, GL_COMPILE);	
	glLineWidth(Settings::firBorderLineStrength());
	glBegin(GL_LINE_STRIP);
	color = Settings::firBorderLineColor().rgba();
	glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	for (int i = 0; i < _points.size(); i++) {
		VERTEX(_points[i].first, _points[i].second);
	}
	glEnd();

	glEndList();
}

GLuint Fir::getPolygon() {
	if(polygon == 0)
		compileDisplayLists();
	return polygon;
}

GLuint Fir::getBorderLine() {
	if(borderline == 0)
		compileDisplayLists();
	return borderline;
}

void Fir::setPointList(const QList<QPair<double, double> >& points) {
	_points = points;
	if(_points.last() != _points.first())
		_points.append(_points.first());
}

QPair<double, double> Fir::equidistantPoint() {
    QPair<double, double> topLeft = _points[0];
    QPair<double, double> bottomRight = _points[0];
    for(int i = 0; i < _points.size(); i++) {
        if(_points[i].first < topLeft.first)
            topLeft.first = _points[i].first;
        else if(_points[i].first > bottomRight.first)
            bottomRight.first = _points[i].first;
        else if(_points[i].second < topLeft.second)
            topLeft.first = _points[i].first;
        else if(_points[i].second > bottomRight.second)
            bottomRight.first = _points[i].first;
    }

    return QPair<double, double>((topLeft.first + bottomRight.first) / 2, (topLeft.second + bottomRight.second) / 2);
}

int Fir::maxDistanceFromCenter() {
    if(_maxDistFromCenter != 0)
        return _maxDistFromCenter;

    if(_points.size() == 0)
        return -1;

    double maxDist = NavData::distance(lat(), lon(), _points[0].first, _points[0].second);
    for(int i = 0; i < _points.size(); i++) {
        if(NavData::distance(lat(), lon(), _points[i].first, _points[i].second) > maxDist) {
            maxDist = NavData::distance(lat(), lon(), _points[i].first, _points[i].second);
        }
    }

    _maxDistFromCenter = (int) maxDist;
    return _maxDistFromCenter;
}
