#include "Sector.h"

#include "helpers.h"
#include "Settings.h"
#include "Tessellator.h"

Sector::Sector(QStringList fields, unsigned int _debugLineNumber) :
    _polygon(0),
    _borderline(0),
    _polygonHighlighted(0),
    _borderlineHighlighted(0)
{
    // LSAZ:Zurich:CH:46.9:9.1:189[:CTR]
    if(fields.size() != 6 && fields.size() != 7) {
        auto msg = QString("While processing line #%1 '%2' from %3: Expected 6-7 fields, got %5")
            .arg(_debugLineNumber)
            .arg(fields.join(':'))
            .arg("data/firlist.dat")
            .arg(fields.count());
        qCritical() << "NavData::loadCountryCodes()" << msg;
        QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
        exit(EXIT_FAILURE);
    }

    icao = fields[0];
    name = fields[1];
    id = fields[5];
    if(fields.size() >= 7) {
        m_controllerSuffixes = fields[6].split(" ", Qt::SkipEmptyParts);
    }
    debugControllerLineNumber = _debugLineNumber;
}

Sector::~Sector() {
    if(_polygon != 0) {
        glDeleteLists(_polygon, 1);
    }
    if(_borderline != 0) {
        glDeleteLists(_borderline, 1);
    }
    if(_polygonHighlighted != 0) {
        glDeleteLists(_polygonHighlighted, 1);
    }
    if(_borderlineHighlighted != 0) {
        glDeleteLists(_borderlineHighlighted, 1);
    }
}

const QList<QPair<double, double> > &Sector::points() const
{
    return m_points;
}

bool Sector::containsPoint(const QPointF &pt) const
{
    foreach(const auto polygon, nonWrappedPolygons()) {
        if(polygon.containsPoint(pt, Qt::OddEvenFill)) {
            return true;
        }
    }
    return false;
}

/**
 * This contains 2 polygons where 1 can be empty.
 * If this sector wraps at the longitude +/-180, this returns this sector split
 * at the lon=-/+180 meridian.
 * They are suitable to check for containment of a point.
 */
const QList<QPolygonF> &Sector::nonWrappedPolygons() const
{
    return m_nonWrappedPolygons;
}

void Sector::setPoints(const QList<QPair<double, double> > &points)
{
    m_points = points;

    // Populate m_nonWrappedPolygons:
    m_nonWrappedPolygons = { QPolygonF(), QPolygonF() };
    bool iCurrentPolygon = 0;
    auto currentPolygon = &m_nonWrappedPolygons[iCurrentPolygon];

    const int count = m_points.size();
    for(int i = 0; i < count; i++) {
        auto current = m_points[i];

        auto next = m_points[(i + 1) % count];
        currentPolygon->append(QPointF(current.first, current.second));
        // Lon +/-180 wrap
        auto diff = std::abs(next.second - current.second);
        if(diff > 180.) {
            // add supporting point at the border to both sub-polygons
            float latAtBorder = 0.;
            float t;
            if(current.second > 90.) {
                auto diffCurrent = 180. - current.second;
                auto diffNext = 180. + next.second;
                auto total = diffCurrent + diffNext;

                if(qFuzzyIsNull(total)) {
                    t = 0.;
                } else {
                    t = diffCurrent / total;
                }
                latAtBorder = Helpers::lerp(current.first, next.first, t);

                currentPolygon->append(QPointF(latAtBorder, 180.));
            } else { // < -90.
                auto diffCurrent = 180. + current.second;
                auto diffNext = 180. - next.second;
                auto total = diffCurrent + diffNext;

                if(qFuzzyIsNull(total)) {
                    t = 0.;
                } else {
                    t = diffCurrent / total;
                }
                latAtBorder = Helpers::lerp(current.first, next.first, t);

                currentPolygon->append(QPointF(latAtBorder, -180.));
            }
            iCurrentPolygon = !iCurrentPolygon;
            currentPolygon = &m_nonWrappedPolygons[iCurrentPolygon];
            if(current.second > 90.) {
                currentPolygon->append(QPointF(latAtBorder, -180.));
            } else {
                currentPolygon->append(QPointF(latAtBorder, 180.));
            }
        }
    }
}

GLuint Sector::glPolygon() {
    if(_polygon == 0) {
        _polygon = glGenLists(1);
        glNewList(_polygon, GL_COMPILE);
        QColor color = Settings::firFillColor();
        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        Tessellator().tessellate(m_points);
        glEndList();
    }
    return _polygon;
}

GLuint Sector::glBorderLine() {
    if(_borderline == 0) {
        _borderline = glGenLists(1);
        glNewList(_borderline, GL_COMPILE);
        glLineWidth(Settings::firBorderLineStrength());
        glBegin(GL_LINE_LOOP);
        QColor color = Settings::firBorderLineColor();
        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        for(int i = 0; i < m_points.size(); i++) {
            VERTEXhigh(m_points[i].first, m_points[i].second);
        }
        glEnd();
        glEndList();
    }
    return _borderline;
}

GLuint Sector::glPolygonHighlighted() {
    if(_polygonHighlighted == 0) {
        _polygonHighlighted = glGenLists(1);
        glNewList(_polygonHighlighted, GL_COMPILE);
        QColor color = Settings::firHighlightedFillColor();
        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        Tessellator().tessellate(m_points);
        glEndList();
    }
    return _polygonHighlighted;
}

GLuint Sector::glBorderLineHighlighted() {
    if(_borderlineHighlighted == 0) {
        _borderlineHighlighted = glGenLists(1);
        glNewList(_borderlineHighlighted, GL_COMPILE);
        glLineWidth(Settings::firHighlightedBorderLineStrength());
        glBegin(GL_LINE_LOOP);
        QColor color = Settings::firHighlightedBorderLineColor();
        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        for(int i = 0; i < m_points.size(); i++) {
            VERTEXhigh(m_points[i].first, m_points[i].second);
        }
        glEnd();
        glEndList();
    }
    return _borderlineHighlighted;
}

QPair<double, double> Sector::getCenter() const {
    return Helpers::polygonCenter(m_points);
}
