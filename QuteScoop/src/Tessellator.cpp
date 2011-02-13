/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Tessellator.h"

#include "helpers.h"

GLdouble vertices[64][6]; // arrary to store newly created vertices (x,y,z,r,g,b) by combine callback
int vertexIndex;          // array index for above array incremented inside combine callback

Tessellator::Tessellator()
{
    tess = gluNewTess();
    gluTessCallback(tess, GLU_TESS_BEGIN,	CALLBACK_CAST tessBeginCB);
    gluTessCallback(tess, GLU_TESS_END,		CALLBACK_CAST tessEndCB);
    gluTessCallback(tess, GLU_TESS_ERROR,	CALLBACK_CAST tessErrorCB);
    gluTessCallback(tess, GLU_TESS_VERTEX,	CALLBACK_CAST tessVertexCB);
    gluTessCallback(tess, GLU_TESS_COMBINE, CALLBACK_CAST tessCombineCB);
}

Tessellator::~Tessellator()
{
    gluDeleteTess(tess);
}

void Tessellator::tessellate(const QList<QPair<double, double> >& points) {
    // tessellate and compile polygon into display list
    // gluTessVertex() takes 3 params: tess object, pointer to vertex coords,
    // and pointer to vertex data to be passed to vertex callback.
    // The second param is used only to perform tessellation, and the third
    // param is the actual vertex data to draw. It is usually same as the second
    // param, but It can be more than vertex coord, for example, color, normal
    // and UV coords which are needed for actual drawing.
    // Here, we are looking at only vertex coods, so the 2nd and 3rd params are
    // pointing same address.

    pointList.clear();
    vertexIndex = 0;

    gluTessBeginPolygon(tess, 0);
    gluTessBeginContour(tess);
    for(int i = 0; i < points.size(); i++) {
        GLdouble *p = new GLdouble[3];
        pointList.append(p);
        p[0] = SXhigh(points[i].first, points[i].second);
        p[1] = SYhigh(points[i].first, points[i].second);
        p[2] = SZhigh(points[i].first, points[i].second);
        gluTessVertex(tess, p, p);
    }
    gluTessEndContour(tess);
    gluTessEndPolygon(tess);

    for(int i = 0; i < pointList.size(); i++)
        delete pointList[i];
    pointList.clear();
}

void Tessellator::tessellateSphere(const QList<QPair<double, double> > &points, double sectorWidth) {
    qDebug() << "Tessellator::tessellateSphere()" << points.size() << "points";
    this->sectorWidth = sectorWidth; // start with same lat/lon extend. We will unify sectors later

    // finding maximum extends of continent
    double minLat =  90, minLon =  180;
    double maxLat = -90, maxLon = -180;
    int minLatPoint; // we will use this to start travelling the polygon because we know exactly that the filled area is North of that point
    for (int i = 0; i < points.size(); i++) {
        Q_ASSERT(points[i].first >=  -90);
        Q_ASSERT(points[i].first <=   90);
        Q_ASSERT(points[i].first >= -180);
        Q_ASSERT(points[i].first <=  180);

        if (points[i].first < minLat) {
            minLat = points[i].first;
            minLatPoint = i;
        }
        maxLat = qMax(maxLat, points[i].first);
        minLon = qMin(minLon, points[i].second);
        maxLon = qMax(maxLon, points[i].second);
    }
    //qDebug() << "minLatPoint" << minLatPoint << "minLat" << minLat << "maxLat" << maxLat << "minLon" << minLon << "maxLon" << maxLon;

    // calculate all sector crossing points and their corresponding polygon directions
    QHash<int, Sector > sectors; // (sector left bottom: sectorHash, list of points)
    Point oldPoint = points[modPositive(minLatPoint - 1, points.size())];
    Direction fillDirLat = NORTH;
    Direction fillDirLon = (points[minLatPoint].second > oldPoint.second? WEST: EAST);
    Direction moveDirLat = NORTH;
    Direction moveDirLon = fillDirLon;

    //qDebug() << moveDirLat << (Direction) -moveDirLat;
    //qDebug() << moveDirLon << (Direction) -moveDirLon;
    int oldHash = hashSector(oldPoint);
    for (int it = 0; it < points.size(); it++) {
        int i = modPositive(it + minLatPoint, points.size()); // map 0..size-1 to minLatPoint..size-1..minLatPoint-1

        int hash = hashSector(points[i]);
        sectors[hash].points.append(Point(points[i].first, points[i].second));

        if (hash != oldHash) { // we are crossing sectors
            //qDebug() << "new sector" << unhashSector(hash);
            Direction dir = crossingDir(oldHash, hash); // the direction of crossing

            // updating old
            sectors[oldHash].points.append(Point(points[i].first, points[i].second)); // append point to last sector also to prevent gaps
            Sector::Border::Crossing oldCrossing;
            oldCrossing.fillDirLat = fillDirLat;
            oldCrossing.fillDirLon = fillDirLon;
            oldCrossing.enterring = false;
            oldCrossing.sector = &sectors[oldHash];
            oldCrossing.pointIndex = sectors[oldHash].points.size() -1; // we just inserted it
            sectors[oldHash].borders[dir].crossings.append(oldCrossing);
            //qDebug() << "old sector borders" << sectors[oldHash].borders.keys();

            // updating this
            Sector::Border::Crossing crossing;
            crossing.fillDirLat = fillDirLat;
            crossing.fillDirLon = fillDirLon;
            crossing.enterring = true;
            crossing.sector = &sectors[hash];
            crossing.pointIndex = sectors[hash].points.size() - 1; // was the last one inserted
            sectors[hash].borders[(Direction) -dir].crossings.append(crossing);
        }
        // updating direction: sounds crazy, but we get the right/left-fillDirection by looking for a turn up/down
        //qDebug() << points[i];
        //qDebug() << (points[i].first > oldPoint.first? NORTH: SOUTH) << moveDirLat;
        if ((points[i].first > oldPoint.first? NORTH: SOUTH) != moveDirLat) {
            moveDirLat = (Direction) -moveDirLat;
            fillDirLon = (Direction) -fillDirLon;
            //qDebug() << "changed dir" << moveDirLat << moveDirLon << fillDirLat << fillDirLon;
        }
        if ((points[i].second > oldPoint.second? WEST: EAST) != moveDirLon) {
            moveDirLon = (Direction) -moveDirLon;
            fillDirLat = (Direction) -fillDirLat;
            //qDebug() << "changed dir" << moveDirLat << moveDirLon << fillDirLat << fillDirLon;
        }

        oldHash = hash;
        oldPoint = points[i];
    }

    foreach(int hash, sectors.keys()) {
        Sector *sector = &sectors[hash];

        // sort crossings
        qSort(sector->borders[NORTH].crossings.begin(), sector->borders[NORTH].crossings.end(), easter);
        qSort(sector->borders[SOUTH].crossings.begin(), sector->borders[SOUTH].crossings.end(), easter);
        qSort(sector->borders[WEST].crossings.begin(),  sector->borders[WEST].crossings.end(),  souther);
        qSort(sector->borders[EAST].crossings.begin(),  sector->borders[EAST].crossings.end(),  souther);

        // add points
        double south = (hash / 360) - 90;
        double north = south + sectorWidth;
        double west  = (hash % 360) - 180;
        double east  = west + sectorWidth;
        QHash<int, Point> pointsToInsert; // we do not insert them straight ahead to not break pointIndex
        QList<Sector::Border::Crossing> *crossings = &sector->borders[NORTH].crossings;
        while (!sector->borders[NORTH].crossings.isEmpty()) {
            QString debug("");
            foreach (Sector::Border::Crossing c, *crossings)
                debug += QString("\nfillLat=%1 fillLon=%2 enter=%3 i=%4 (%5/%6)").arg(c.fillDirLat).arg(c.fillDirLon).arg(c.enterring)
                    .arg(c.pointIndex).arg(sector->points[c.pointIndex].first).arg(sector->points[c.pointIndex].second);

            qDebug() << "NORTH" << debug;
            if (crossings->first().fillDirLon == WEST) { // the most Western needs filling to the west
                sector->points.insert( // line to the edge
                        crossings->first().pointIndex + (crossings->first().enterring? 0: 1),
                        Point(north, east));
                foreach (Sector::Border::Crossing c2, *crossings) // fix pointIndex for elements after this insertion
                    if (c2.pointIndex > crossings->first().pointIndex)
                        c2.pointIndex++;
                crossings->removeFirst();
                qDebug() << " 1st WEST" << crossings->size();
            } else if (crossings->first().fillDirLon == EAST) { // the most Western needs filling to the east
                if (crossings->size() > 1) { // more items: draw to the next crossing
                    //Q_ASSERT(crossings->at(1).fillDirLon == WEST);
                    sector->points.insert( // 1st item to 2nd item
                            crossings->first().pointIndex + (crossings->first().enterring? 0: 1),
                            sector->points[crossings->at(1).pointIndex]);
                    foreach (Sector::Border::Crossing c2, *crossings) // fix pointIndex for elements after this insertion
                        if (c2.pointIndex > crossings->first().pointIndex)
                            c2.pointIndex++;
                    crossings->removeFirst(); // delete 1st element
                    foreach (Sector::Border::Crossing c2, *crossings) // fix pointIndex for elements after this insertion
                        if (c2.pointIndex > crossings->first().pointIndex) // the 2nd is not 1st
                            c2.pointIndex++;
                    crossings->removeFirst();
                    qDebug() << " 1st EAST and connecting" << crossings->size();
                } else { // last crossing: draw to the border
                    sector->points.insert( // line to the edge
                            crossings->first().pointIndex + (crossings->first().enterring? 0: 1),
                            Point(north, west));
                    foreach (Sector::Border::Crossing c2, *crossings) // fix pointIndex for elements after this insertion
                        if (c2.pointIndex > crossings->first().pointIndex)
                            c2.pointIndex++;
                    crossings->removeFirst();
                    qDebug() << " 1st EAST and last" << crossings->size();
                }
            } else {
                QString debug("Error tessellating ");
                foreach (Sector::Border::Crossing c, *crossings)
                    debug += QString(",\nfillLat=%1 fillLon=%2 enter=%3 i=%4").arg(c.fillDirLat).arg(c.fillDirLon).arg(c.enterring).arg(c.pointIndex);
                qFatal(debug.toAscii());
                //Q_ASSERT(false);
            }
        }
        if (!sector->borders[SOUTH].crossings.isEmpty()) {
            //sector->points.append(Point(south, west));
            qDebug() << "SOUTH";
        }
        if (!sector->borders[WEST].crossings.isEmpty()) {
            //sector->points.append(Point(north, west));
            qDebug() << "WEST";
        }
        if (!sector->borders[EAST].crossings.isEmpty()) {
            //sector->points.append(Point(south, east));
            qDebug() << "EAST";
        }
        //sector->points.append(Point(0,0));
    }

    // drawing
    foreach(int hash, sectors.keys()) {
        Point bottomLeft = unhashSector(hash);
        double sectorLatBottom = (hash / 360) - 90;
        double sectorLatTop    = sectorLatBottom + sectorWidth;
        double sectorLonLeft   = (hash % 360) - 180;
        double sectorLonRight  = sectorLonLeft + sectorWidth;
        //qDebug() << "sector" << hash << ":" << sectorLonLeft << sectorLatBottom;
        QList<Point> points = sectors[hash].points;
        //tessellate(points);
        glBegin(GL_LINE_LOOP);
        foreach (Point p, points)
            VERTEX(p.first, p.second);
        glEnd();
    }
    qDebug() << "Tessellator::tessellateSphere() -- finished" << sectors.size() << "sectors";
}

int Tessellator::hashSector(const QPair<double, double> &p) {
    return (qFloor(p.first / sectorWidth) * sectorWidth + 90) * 360
            + qFloor(p.second / sectorWidth) * sectorWidth + 180;
}

QPair<double, double> Tessellator::unhashSector(const int &sectorHash) {
    return QPair<double, double>((sectorHash / 360) - 90, (sectorHash % 360) - 180);
}

Tessellator::Direction Tessellator::crossingDir(const int &sectorHash1, const int &sectorHash2) {
    Point p1SectorBottomLeft = unhashSector(sectorHash1);
    Point p2SectorBottomLeft = unhashSector(sectorHash2);
    short dLat = p2SectorBottomLeft.first  - p1SectorBottomLeft.first;
    short dLon = p2SectorBottomLeft.second - p1SectorBottomLeft.second;

    if (dLat > 0)
        return NORTH;
    if (dLat < 0)
        return SOUTH;
    if (dLon > 0)
        return WEST;
    if (dLon < 0)
        return EAST;
    return NONE;
}


CALLBACK_DECL Tessellator::tessErrorCB(GLenum errorCode) {
    QString str((char*)gluErrorString(errorCode));
}

CALLBACK_DECL Tessellator::tessBeginCB(GLenum which) {
    glBegin(which);
}

CALLBACK_DECL Tessellator::tessEndCB() {
    glEnd();
}

CALLBACK_DECL Tessellator::tessVertexCB(const GLvoid *data) {
    const GLdouble *ptr = (const GLdouble*)data;
    glVertex3dv(ptr);
}

///////////////////////////////////////////////////////////////////////////////
// Combine callback is used to create a new vertex where edges intersect.
// In this function, copy the vertex data into local array and compute the
// color of the vertex. And send it back to tessellator, so tessellator pass it
// to vertex callback function.
//
// newVertex: the intersect point which tessellator creates for us
// neighborVertex[4]: 4 neighbor vertices to cause intersection (given from 3rd param of gluTessVertex()
// neighborWeight[4]: 4 interpolation weights of 4 neighbor vertices
// outData: the vertex data to return to tessellator
///////////////////////////////////////////////////////////////////////////////
CALLBACK_DECL Tessellator::tessCombineCB(const GLdouble newVertex[3], const GLdouble *neighborVertex[4],
                                         const GLfloat neighborWeight[4], GLdouble **outData)
{
    // copy new intersect vertex to local array
    // Because newVertex is temporal and cannot be hold by tessellator until next
    // vertex callback called, it must be copied to the safe place in the app.
    // Once gluTessEndPolygon() called, then you can safly deallocate the array.

    vertices[vertexIndex][0] = newVertex[0];
    vertices[vertexIndex][1] = newVertex[1];
    vertices[vertexIndex][2] = newVertex[2];

    // return output data (vertex coords and others)
    *outData = vertices[vertexIndex]; // assign the address of new intersect vertex
    ++vertexIndex; // increase index for next vertex
}

