#ifndef HELPERS_H_
#define HELPERS_H_

#include <math.h>
#include <QList>
#include <QPair>
#include <QColor>
#include <QPalette>

// typedefs: needed to get the QPair template running inside foreach's
typedef QPair<double, double> DoublePair;

class Helpers {
    public:
        static QStringList linesFilteredTrimmed(const QString &str) {
            QStringList ret;

            foreach (const auto line, str.split("\n")) {
                const auto lineTrimmed = line.trimmed();
                if (!lineTrimmed.isEmpty()) {
                    ret << lineTrimmed;
                }
            }

            return ret;
        }

        static QColor highLightColor(const QColor& c) {
            QColor _c;
            if (c.lightnessF() * c.alphaF() > .6) {
                _c = c.darker(110);
            } else {
                _c = c.lighter(110);
            }
            _c.setAlpha(fmin(255, _c.alpha() * 1.3));
            return _c;
        }

        static QColor shadowColorForBg(const QColor& c) {
            QPalette palette(c);
            return c.lightnessF() > .5? palette.dark().color(): palette.light().color();
        }

        static QColor mixColor (const QColor& c1, const QColor& c2, float fraction) {
            return QColor(
                Helpers::lerp(c1.red(), c2.red(), fraction),
                Helpers::lerp(c1.green(), c2.green(), fraction),
                Helpers::lerp(c1.blue(), c2.blue(), fraction),
                Helpers::lerp(c1.alpha(), c2.alpha(), fraction)
            );
        };

        // modulo, but always positive, not like fmod()
        static float inline modPositive(float x, float y) {
            if (qFuzzyIsNull(y)) {
                return x;
            }
            return x - y * floor(x / y);
        }

        static float inline lerp(float v0, float v1, float t) {
            return v0 + t * (v1 - v0);
        }

        static float fraction(float min, float max, float v) {
            if (max == min) {
                return 1.;
            }
            return (v - min) / (max - min);
        }

        /* At 180 the longitude wraps around to -180
         * Since this can cause problems in the computation this adjusts point B relative to point A
         * such that the difference in longitude is less than 180
         * That is: Instead of going from a longitude of 179 in point A to a longitude of -179 in point B by going
         * westwards
         * we set the longitude of point B to 181 to indicate that we're going east
         */
        static void adjustPoint(const DoublePair &a, DoublePair &b) {
            const double diff = a.second - b.second;
            if (std::abs(diff) > 180) {
                b.second += ((diff > 0) - (diff < 0)) * 360;
            }
        }

        /*
         * @return (-360., -360.) on error
         */
        static DoublePair polygonCenter(const QList<DoublePair> points) {
            // https://en.wikipedia.org/wiki/Centroid#Of_a_polygon

            if (points.isEmpty()) {
                return DoublePair(-360., -360.);
            }

            double A = 0;
            DoublePair runningTotal;
            runningTotal.first = 0;
            runningTotal.second = 0;

            DoublePair previous = points[0];

            const int count = points.size();
            for (int i = 0; i < count; ++i) {
                DoublePair current = points[i];
                DoublePair next = points[(i + 1) % count];
                if (i > 0) {
                    adjustPoint(previous, current);
                }
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
};

/* mathematical constants */
const double Pi180 = M_PI / 180.;

/* 3D-calculations */
#define SX(lat, lon) (GLfloat) qCos((lat) * Pi180) * (GLfloat) qSin((lon) * Pi180)
#define SY(lat, lon) (GLfloat) - qCos((lat) * Pi180) * (GLfloat) qCos((lon) * Pi180)
#define SZ(lat, lon) (GLfloat) - qSin((lat) * Pi180)
#define VERTEX(lat, lon) glVertex3f(SX(lat, lon), SY(lat, lon), SZ(lat, lon))
// higher VERTEX: 30km AGL (used for polygons to prevent intersecting with the globe)
#define SXhigh(lat, lon) SX(lat, lon) * 1.005
#define SYhigh(lat, lon) SY(lat, lon) * 1.005
#define SZhigh(lat, lon) SZ(lat, lon) * 1.005
#define VERTEXhigh(lat, lon) glVertex3f(SXhigh(lat, lon), SYhigh(lat, lon), SZhigh(lat, lon))

/* units */
#define Nm2Deg(miles) (miles / 60.0)
#define mToFt(m) (m * 3.28084)

#endif /*HELPERS_H_*/
