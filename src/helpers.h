#ifndef HELPERS_H_
#define HELPERS_H_

#include <math.h>
#include <QList>
#include <QPair>

// typedefs: needed to get the QPair template running inside foreach's
typedef QPair<double, double> DoublePair;

class Helpers {
public:
  // modulo, but always positive, not like fmod()
  static float inline modPositive(float x, float y) {
      if (qFuzzyIsNull(y))
          return x;
      return x - y * floor(x/y);
  }

  static float inline lerp(float v0, float v1, float t) {
      return v0 + t * (v1 - v0);
  }

  /* At 180 the longitude wraps around to -180
   * Since this can cause problems in the computation this adjusts point B relative to point A
   * such that the difference in longitude is less than 180
   * That is: Instead of going from a longitude of 179 in point A to a longitude of -179 in point B by going westwards
   * we set the longitude of point B to 181 to indicate that we're going east
   */
  static void adjustPoint(const DoublePair &a, DoublePair &b) {
      const double diff = a.second - b.second;
      if(std::abs(diff) > 180)
          b.second += ((diff > 0) - (diff < 0)) * 360;
  }

  /*
   * @return (-360., -360.) on error
   */
  static DoublePair polygonCenter(const QList<DoublePair> points) {
      // https://en.wikipedia.org/wiki/Centroid#Of_a_polygon

      if (points.size() == 0) {
          return DoublePair(-360., -360.);
      }

      double A = 0;
      DoublePair runningTotal;
      runningTotal.first = 0;
      runningTotal.second = 0;

      DoublePair previous = points[0];

      const int count = points.size();
      for(int i = 0; i < count; ++i) {
          DoublePair current = points[i];
          DoublePair next = points[(i + 1) % count];
          if(i > 0) adjustPoint(previous, current);
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
#define SY(lat, lon) (GLfloat) -qCos((lat) * Pi180) * (GLfloat) qCos((lon) * Pi180)
#define SZ(lat, lon) (GLfloat) -qSin((lat) * Pi180)
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
