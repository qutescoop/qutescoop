/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef TESSELLATOR_H_
#define TESSELLATOR_H_

#include "_pch.h"

// platform specific stuff. tessellator callback definitions are not the
// same on all platforms

#ifdef Q_WS_MAC
#define CALLBACK_CAST (GLvoid (*)())
#define CALLBACK_DECL GLvoid
#endif

#ifdef Q_WS_X11
#define CALLBACK_CAST (void (*)())
#define CALLBACK_DECL GLvoid
#endif

#ifdef Q_WS_WIN
#define CALLBACK_CAST (_GLUfuncptr)
#define CALLBACK_DECL void CALLBACK
#endif

class Tessellator {
public:
    Tessellator();
    ~Tessellator();

    void tessellate(const QList<QPair<double, double> > &points);
    void tessellateSphere(const QList<QPair<double, double> > &points, double sectorWidth);

private:
    GLUtesselator *tess;
    QList<GLdouble*> pointList;
    double sectorWidth; // given in degrees at the equator

    typedef QPair<double, double> Point; // just for convenience
    enum Direction {SOUTH = -1, NORTH = 1, EAST = -2, WEST = 2, NONE = 0};
    class Sector { // one slice that has ~sectorWidth width and height. OpenGLs tessellator will take these
    public:
        class Border { // 0..4 borders (NORTH, SOUTH...) where the path crosses to the next sector
        public:
            class Crossing { // the actual crossing into the next sector
            public:
                Direction fillDirLat; // where is the filled area on the lat-axis?
                Direction fillDirLon; // ...and on the lon-axis?
                bool enterring; // Does the points path enter this sector here?
                unsigned short pointIndex; // point index for the vertex at this Crossing
                Sector *sector; // just a pointer for qSort()'s helper-methods
            };
            QList<Crossing> crossings; // all border-crossings at this border
        };
        QList<Point> points; // all points in this sector (plus a defined overlap where the path exits)
        QHash<Direction, Border> borders; // border[LEFT] etc.
    };

    // compare function for Points
    static bool souther (Sector::Border::Crossing &c1, Sector::Border::Crossing &c2) {
        return c1.sector->points[c1.pointIndex].first  < c2.sector->points[c2.pointIndex].first; }
    static bool easter  (Sector::Border::Crossing &c1, Sector::Border::Crossing &c2) {
        return c1.sector->points[c1.pointIndex].second < c2.sector->points[c2.pointIndex].second; }

    // hashing und unhashing functions for the sectors hash
    int hashSector(const Point &p);
    Point unhashSector(const int &sectorHash);
    // calculating the direction of a sector crossing
    Direction crossingDir(const int &sectorHash1, const int &sectorHash2);

    // modulo, but always positive, not like fmod()
    double inline modPositive(double x, double y)
    {
        if (0 == y)
            return x;
        return x - y * floor(x/y);
    }

    static CALLBACK_DECL tessBeginCB(GLenum which);
    static CALLBACK_DECL tessEndCB();
    static CALLBACK_DECL tessVertexCB(const GLvoid *data);
    static CALLBACK_DECL tessErrorCB(GLenum errorCode);
    static CALLBACK_DECL tessCombineCB(const GLdouble newVertex[3],
                                       const GLdouble *neighborVertex[4],
                                       const GLfloat neighborWeight[4], GLdouble **outData);
};

#endif /*TESSELLATOR_H_*/
