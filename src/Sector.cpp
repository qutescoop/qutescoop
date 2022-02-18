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

/* At 180 the longitude wraps around to -180
 * Since this can cause problems in the computation this adjusts point B relative to point A
 * such that the difference in longitude is less than 180
 * That is: Instead of going from a longitude of 179 in point A to a longitude of -179 in point B by going westwards
 * we set the longitude of point B to 181 to indicate that we're going east
 */
void adjustPoint(const QPair<double, double> &a, QPair<double, double> &b) {
    const double diff = a.second - b.second;
    if(std::abs(diff) > 180)
        b.second += ((diff > 0) - (diff < 0)) * 360;
}

QPair<double, double> Sector::getCenter() const {
    // https://en.wikipedia.org/wiki/Centroid#Of_a_polygon

    double A = 0;
    QPair<double, double> runningTotal;
    runningTotal.first = 0;
    runningTotal.second = 0;
    const int count = points.size();

    if(count == 0) {
        qCritical() << "Sector::getCenter() Sector " << name << "(" << icao << ") doesn't contain any points";
        QTextStream(stdout) << "CRITICAL: Sector " << name << "(" << icao << ") doesn't contain any points";
        exit(EXIT_FAILURE);
    }

    QPair<double, double> previous = points[0];

    for(int i = 0; i < count; ++i) {
        QPair<double, double> current = points[i];
        QPair<double, double> next = points[(i + 1)%count];
        if(i > 0)
            adjustPoint(previous, current);
        adjustPoint(current, next);
        previous = current;

        A += (current.first * next.second
             - next.first * current.second);

        double multiplyBy = current.first * next.second
                            - (next.first * current.second);

        runningTotal.first += (current.first + next.first)
                            * multiplyBy;
        
        runningTotal.second += (current.second + next.second)
                            * multiplyBy;
    }
    A /= 2;

    runningTotal.first /= 6 * A;
    runningTotal.second /= 6 * A;

    runningTotal.second = std::fmod(runningTotal.second + 180, 360) - 180;

    return runningTotal;
}
