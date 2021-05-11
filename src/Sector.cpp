/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Sector.h"

#include "Settings.h"
#include "Tessellator.h"
#include "NavData.h"
#include "helpers.h"

Sector::Sector(QStringList strings) {
    //LSAZ:Zurich:CH:46.9:9.1:189
    icao = strings[0];
    name = strings[1];
    countryCode = strings[2];
    lat = strings[3].toDouble();
    lon = strings[4].toDouble();
    id = strings[5];

    _polygon = 0;
    _borderline = 0;
}

Sector::~Sector() {
    if(_polygon != 0)
        glDeleteLists(_polygon, 1);
    if(_borderline != 0)
        glDeleteLists(_borderline, 1);
}

const QPolygonF Sector::sectorPolygon() const {
    QPolygonF pol;
    foreach(const DoublePair p, points)
        pol.append(QPointF(p.first, p.second));
    return pol;
}

GLuint Sector::glPolygon() {
    if (_polygon == 0) {
        _polygon = glGenLists(1);
        glNewList(_polygon, GL_COMPILE);
        QColor color = Settings::firFillColor();
        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        Tessellator().tessellate(points);
        // without Tesselator:
//        glBegin(GL_POLYGON);
//        foreach(const DoublePair &p, points)
//            VERTEXhigh(p.first, p.second);
//        glEnd;
        glEndList();
    }
    return _polygon;
}

GLuint Sector::glBorderLine() {
    if (_borderline == 0) {
        _borderline = glGenLists(1);
        glNewList(_borderline, GL_COMPILE);
        glLineWidth(Settings::firBorderLineStrength());
        glBegin(GL_LINE_LOOP);
        QColor color = Settings::firBorderLineColor();
        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        for (int i = 0; i < points.size(); i++)
            VERTEXhigh(points[i].first, points[i].second);
        glEnd();
        glEndList();
    }
    return _borderline;
}

QPair<double, double> Sector::getCenter() {
    QPair<double, double> runningTotal;
    runningTotal.first = 0;
    runningTotal.second = 0;
    foreach(const DoublePair p, points) {
        runningTotal.first += p.first;
        runningTotal.second += p.second;
    }
    runningTotal.first /= points.size();
    runningTotal.second /= points.size();

    return runningTotal;
}