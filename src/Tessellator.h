/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef TESSELLATOR_H_
#define TESSELLATOR_H_

#include "_pch.h"
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
// platform specific stuff. tessellator callback definitions are not the
// same on all platforms

#ifdef Q_OS_MAC
#define CALLBACK_CAST (GLvoid (*)())
#define CALLBACK_DECL GLvoid
#endif

#ifdef Q_OS_LINUX
#define CALLBACK_CAST (void (*)())
#define CALLBACK_DECL GLvoid
#endif

#ifdef Q_OS_WIN
#define CALLBACK_CAST (GLvoid (*) ())
#define CALLBACK_DECL void CALLBACK
#endif
#include <QPair>
class Tessellator {
    public:
        Tessellator();
        ~Tessellator();

        void tessellate(const QList<QPair<double, double> >& points);

    private:
        GLUtesselator *_tess;
        QList<GLdouble*> _pointList;

        static CALLBACK_DECL tessBeginCB(GLenum which);
        static CALLBACK_DECL tessEndCB();
        static CALLBACK_DECL tessVertexCB(const GLvoid *data);
        static CALLBACK_DECL tessErrorCB(GLenum errorCode);
        static CALLBACK_DECL tessCombineCB(const GLdouble newVertex[3],
                                           const GLdouble *neighborVertex[4],
                                           const GLfloat neighborWeight[4], GLdouble **outData);
};

#endif /*TESSELLATOR_H_*/
