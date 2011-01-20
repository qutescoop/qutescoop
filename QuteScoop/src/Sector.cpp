/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include <QDebug>
#include <QPair>

#include "Settings.h"
#include "Sector.h"
#include "Tessellator.h"
#include "NavData.h"
#include "helpers.h"

Sector::Sector() {
    polygon = 0;
    borderline = 0;
    _maxDistFromCenter = 0;
    _equidistantPoint = QPair<double, double>(181,91);
}

bool Sector::isNull() const {
    return _icao.isNull();
}

Sector::Sector(QStringList strings) {
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
    _equidistantPoint = QPair<double, double>(181,91);
}

Sector::~Sector() {
    if(polygon != 0) glDeleteLists(polygon, 1);
    if(borderline != 0) glDeleteLists(borderline, 1);
}

void Sector::compileDisplayLists() {
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

GLuint Sector::getPolygon() {
    if(polygon == 0)
        compileDisplayLists();
    return polygon;
}

GLuint Sector::getBorderLine() {
    if(borderline == 0)
        compileDisplayLists();
    return borderline;
}

void Sector::setPointList(const QList<QPair<double, double> >& points)
{
    _points = points;
    if(_points.last() != _points.first())
        _points.append(_points.first());

}

/*Looking for better way
QPair<double, double> Sector::equidistantPoint()
{
    if(_equidistantPoint != QPair<double, double>(181, 91)) {
        return _equidistantPoint;
    }
    if (_points.size() == 0)
        return _equidistantPoint;

    QPair<double, double> topLeft = _points[0];
    QPair<double, double> bottomRight = _points[0];
    for(int i = 1; i < _points.size(); i++) {
        if(_points[i].first < topLeft.first)
            topLeft.first = _points[i].first;
        else if(_points[i].first > bottomRight.first)
            bottomRight.first = _points[i].first;

        if(_points[i].second < topLeft.second)
            topLeft.second = _points[i].second;
        else if(_points[i].second > bottomRight.second)
            bottomRight.second = _points[i].second;
    }

    // does not account for FIRs spanning 179E/179W ;)
    _equidistantPoint = QPair<double, double>((topLeft.first + bottomRight.first) / 2, (topLeft.second + bottomRight.second) / 2);
    return _equidistantPoint;
}

int Sector::maxDistanceFromCenter() {
    if(_maxDistFromCenter != 0)
        return _maxDistFromCenter;

    if(_points.size() == 0)
        return -1;

    QPair<double, double> c = equidistantPoint();

    double maxDist = NavData::distance(c.first, c.second, _points[0].first, _points[0].second);
    for(int i = 1; i < _points.size(); i++) {
        if(NavData::distance(c.first, c.second, _points[i].first, _points[i].second) > maxDist) {
            maxDist = NavData::distance(c.first, c.second, _points[i].first, _points[i].second);
        }
    }

    _maxDistFromCenter = (int) maxDist;
    return _maxDistFromCenter;
}*/
