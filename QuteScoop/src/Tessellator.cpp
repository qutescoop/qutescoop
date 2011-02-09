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

