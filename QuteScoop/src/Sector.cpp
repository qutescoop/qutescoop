/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Sector.h"

#include "Settings.h"
#include "Tessellator.h"
#include "NavData.h"
#include "helpers.h"

Sector::Sector() {
    _polygon = 0;
    _borderline = 0;
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

    _polygon = 0;
    _borderline = 0;
}

Sector::~Sector() {
    if(_polygon != 0) glDeleteLists(_polygon, 1);
    if(_borderline != 0) glDeleteLists(_borderline, 1);
}

const QPolygonF& Sector::sectorPolygon() const {
    QPolygonF pol;
    foreach (DoublePair p, _points)
        pol.append(QPointF(p.first, p.second));
    return pol;
}

void Sector::compileDisplayLists() {
    // filled polygon
    _polygon = glGenLists(1);
    glNewList(_polygon, GL_COMPILE);
    QColor color = Settings::firFillColor();
    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    Tessellator().tessellate(_points);
    glEndList();

    // border line
    _borderline = glGenLists(1);
    glNewList(_borderline, GL_COMPILE);
    glLineWidth(Settings::firBorderLineStrength());
    glBegin(GL_LINE_STRIP);
    color = Settings::firBorderLineColor().rgba();
    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    for (int i = 0; i < _points.size(); i++)
        VERTEXhigh(_points[i].first, _points[i].second);
    glEnd();
    glEndList();
}

GLuint Sector::getPolygon() {
    if (_polygon == 0)
        compileDisplayLists();
    return _polygon;
}

GLuint Sector::getBorderLine() {
    if (_borderline == 0)
        compileDisplayLists();
    return _borderline;
}

void Sector::setPointList(const QList<QPair<double, double> >& points) {
    _points = points;
    if(_points.last() != _points.first())
        _points.append(_points.first());
}
