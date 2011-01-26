/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

/* This is the control file for pre-compiled headers. PCHs should be
   automatically controlled by qmake (so this should not harm if your
   compiler chain does not support it). */

#ifndef _PCH_H
#define _PCH_H
// Add C includes here

#if defined __cplusplus
// Add C++ includes here

#include <QtCore>
#include <QtGui>
#include "Settings.h"
#include "Whazzup.h"
#include "NavData.h"
#include "helpers.h"
//#include "Window.h"

#endif

#endif // _PCH_H
