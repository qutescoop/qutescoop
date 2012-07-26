/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

/* This is the control file for pre-compiled headers. PCHs should be
   automatically controlled by qmake (so this should not harm if your
   compiler chain does not support it).
   They work like this: Get compiled first and the resulting binary is used
   for linking in each file that includes this. If you only use 1 little,
   isolated Qt module it is cheaper to just include it.
   If you think the module will include more code, it tends to be cheaper to
   include these precompiled headers.
*/

#ifndef _PCH_H
#define _PCH_H
// Add C includes here

#if defined __cplusplus
// Add C++ includes here

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtOpenGL>
#include <QDebug>
#include <GL/glu.h>

#endif // __cplusplus

#endif // _PCH_H
