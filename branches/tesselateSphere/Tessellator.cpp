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
    Direction moveDirLat = SOUTH;
    Direction moveDirLon = fillDirLon;

    int oldHash = hashSector(oldPoint);
    for (int it = 0; it < points.size(); it++) {
        int i = modPositive(it + minLatPoint, points.size()); // map 0..size-1 to minLatPoint..size-1..minLatPoint-1

        int hash = hashSector(points[i]);
        sectors[hash].points.append(Point(points[i].first, points[i].second)); // append point to current sector

        // updating direction: sounds crazy, but we get the right/left-fillDirection by looking for a turn up/down and vice versa
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

        if (hash != oldHash) { // we are crossing sectors
            //qDebug() << "new sector" << unhashSector(hash);
            Direction dir = crossingDir(oldHash, hash); // the direction of crossing

            // updating old
            sectors[oldHash].points.append(Point(points[i].first, points[i].second)); // append point to last sector to prevent gaps
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

        //qDebug() << "point" << i << points[i] << "move" << moveDirLat << moveDirLon << "fill" << fillDirLat << fillDirLon;
        oldHash = hash;
        oldPoint = points[i];
    }

    foreach(int hash, sectors.keys()) {
        Sector *sector = &sectors[hash];
        // fill in some data
        sector->borders[SOUTH].position = unhashSector(hash).first;
        sector->borders[EAST].position  = unhashSector(hash).second;
        sector->borders[NORTH].position = sector->borders[SOUTH].position + sectorWidth;
        sector->borders[WEST].position  = sector->borders[EAST ].position + sectorWidth;

        // here comes the real work
        if (!(sector->finalizeBorder(NORTH, sectors, sectorWidth) && // (border, lower / higher end of the border)
              sector->finalizeBorder(SOUTH, sectors, sectorWidth) &&
              sector->finalizeBorder(WEST,  sectors, sectorWidth) &&
              sector->finalizeBorder(EAST,  sectors, sectorWidth))) { // error occured
            qDebug() << "ERROR with sector" << unhashSector(hash) << ". Will not show it.";
            sectors.remove(hash); // do not show the sector
        } else
            qDebug() << "sector" << unhashSector(hash) << "ok.";
    }

    // drawing
    foreach(int hash, sectors.keys()) {
        QList<Point> points = sectors[hash].points;
        glColor4f(fmod(qrand(), 100.) / 100., fmod(qrand(), 100.) / 100., fmod(qrand(), 100.) / 100., 1.0);
        tessellate(points);

        glBegin(GL_LINE_STRIP);
        int i = 0;
        foreach (Point p, points) {
            float c = (float) ++i / points.size();
            glColor4f(c, c, c, 1.0);
            VERTEX(p.first, p.second);
        }
        glEnd();
    }
    qDebug() << "Tessellator::tessellateSphere() -- finished" << sectors.size() << "sectors";
}

bool Tessellator::Sector::finalizeBorder(const Direction borderSide, QHash<int, Sector > &sectors, int sectorWidth) {
    Border *border = &borders[borderSide];
    QList<Border::Crossing> *crossings = &border->crossings;

    Direction low  = (borderSide == NORTH || borderSide == SOUTH)? EAST : SOUTH;
    Direction high = (Direction) -low;
    Point lowPoint  = low == EAST?
                         Point(border->position, borders[low].position):
                         Point(borders[low].position, border->position);
    Point highPoint = low == EAST?
                         Point(border->position, borders[high].position):
                         Point(borders[high].position, border->position);

    qDebug() << " BORDER: borderSide" << borderSide << "low" << low << "high" << high << "lowP" << lowPoint << "highP" << highPoint;

    // sort crossings ascending by either Lat or Lon
    if (low == EAST)
        qSort(crossings->begin(), crossings->end(), border->easter);
    else
        qSort(crossings->begin(), crossings->end(), border->souther);

    for (int i = 0; i < crossings->size(); i++) { // we start at 'low', moving to 'high'
        Border::Crossing *c = &borders[borderSide].crossings[i];
        qDebug() << "  CROSSING: enterring" << c->enterring << "i=" << c->pointIndex << "fillLat" << c->fillDirLat << "fillLon" << c->fillDirLon;

        bool fillLow  = c->fillDirLat == low  || c->fillDirLon == low;
        bool fillHigh = c->fillDirLat == high || c->fillDirLon == high;

        if (i == 0) { // on the low side
            if (fillLow) { // filling to the lowPoint
                insertPoint(c->pointIndex + (c->enterring? 0: 1), lowPoint);
            }
        } else if (i == crossings->size() - 1) { // on the high side
            if (fillHigh) { // filling to the highPoint
                insertPoint(c->pointIndex + (c->enterring? 0: 1), highPoint);
                if (borderSide == NORTH || borderSide == SOUTH) { // look for neighbouring empty sectors
                    for (double dLon = 0; dLon < 360 ; dLon += sectorWidth) {
                        int lon = Tessellator::modPositive(borders[WEST].position + dLon + 180., 360.) - 180.;
                        int hash = Tessellator::hashSector(Point(borders[SOUTH].position, lon), sectorWidth);
                        if (sectors.contains(hash))
                            break;
                        sectors[hash].points.append(Point(borders[SOUTH].position, lon));
                        sectors[hash].points.append(Point(borders[NORTH].position, lon));
                        sectors[hash].points.append(Point(borders[NORTH].position, lon + sectorWidth));
                        sectors[hash].points.append(Point(borders[SOUTH].position, lon + sectorWidth));
                    }
                }
            }
        } else { // somewhere in the middle
            if (fillHigh) { // normal case

            } else { // nothing to do
            }
        }
    }

    // calculate lines to/from the edges or between crossings
    /*while (!borders[border].crossings.isEmpty()) {
        QString debug = QString("border %1, low %2, high %3 {SOUTH = -1, NORTH = 1, EAST = -2, WEST = 2}")
                        .arg(border).arg(low).arg(high);
        foreach (Border::Crossing c, borders[border].crossings)
            debug += QString("\nfillLat=%1 fillLon=%2 enter=%3 i=%4 (%5/%6)").arg(c.fillDirLat).arg(c.fillDirLon).arg(c.enterring)
                .arg(c.pointIndex).arg(points[c.pointIndex].first).arg(points[c.pointIndex].second);
        //qDebug() << debug;

        if (borders[border].crossings.first().fillDirLon == high) { // the most Eastern/Southern needs filling to the W/N
//            if (borders[border].crossings.first().enterring)
//                points.prepend(Point(borders[border].position, borders[low].position));
//            else
//                points.append(Point(borders[border].position, borders[low].position));
            points.insert( // line to the edge
                    borders[border].crossings.first().pointIndex + (borders[border].crossings.first().enterring? 0: 1),
                    Point(borders[border].position, borders[low].position));
            foreach (Border::Crossing c2, borders[border].crossings) // fix pointIndex for elements after this insertion
                if (c2.pointIndex > borders[border].crossings.first().pointIndex)
                    c2.pointIndex++;
            borders[border].crossings.removeFirst();
            //qDebug() << " 1st WEST";
        } else if (borders[border].crossings.last().fillDirLon == low) { // the most Eastern needs filling to the East
//            if (borders[border].crossings.first().enterring)
//                points.prepend(Point(borders[border].position, borders[high].position));
//            else
//                points.append(Point(borders[border].position, borders[high].position));
            points.insert( // line to the edge
                    borders[border].crossings.last().pointIndex + (borders[border].crossings.last().enterring? 0: 1),
                    Point(borders[border].position, borders[high].position));
            foreach (Border::Crossing c2, borders[border].crossings) // fix pointIndex for elements after this insertion
                if (c2.pointIndex > borders[border].crossings.last().pointIndex)
                    c2.pointIndex++;
            borders[border].crossings.removeLast();
            //qDebug() << " 1st EAST";
        } else if (borders[border].crossings.first().fillDirLon == low
                   && borders[border].crossings.size() > 1) { // draw to the next crossing
            //Q_ASSERT(crossings->at(1).fillDirLon == high);
            points.insert( // 1st item to 2nd item
                    borders[border].crossings.first().pointIndex + (borders[border].crossings.first().enterring? 0: 1),
                    points[borders[border].crossings.at(1).pointIndex]);
            foreach (Border::Crossing c2, borders[border].crossings) // fix pointIndex for elements after this insertion
                if (c2.pointIndex > borders[border].crossings.first().pointIndex)
                    c2.pointIndex++;
            borders[border].crossings.removeFirst(); // delete 1st element
            foreach (Sector::Border::Crossing c2, borders[border].crossings) // fix pointIndex for elements after this insertion
                if (c2.pointIndex > borders[border].crossings.first().pointIndex) // the 2nd is not 1st
                    c2.pointIndex++;
            borders[border].crossings.removeFirst(); // delete 2nd element
            //qDebug() << " 1st EAST and connecting";
        } else { // This should never happen! Either an error while getting the fillDirections or bad input data
            qCritical(QString("Error tessellating sphere-sectorborder. State: %1").arg(debug).toAscii());
            return false;
        }
    }*/
    return true;
}

void Tessellator::Sector::insertPoint(int i, Point &p) {
    qDebug() << "   insertPoint size=" << points.size() << "i=" << i << p;
    points.insert(i, p);
    foreach(Border b, borders) {
        foreach (Border::Crossing c, b.crossings) {
            if (c.pointIndex <= i)
                c.pointIndex++;
        }
    }
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
                                         const GLfloat neighborWeight[4], GLdouble **outData) {
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
