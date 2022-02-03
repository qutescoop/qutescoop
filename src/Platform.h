/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef PLATFORM_H
#define PLATFORM_H

#include "_pch.h"

class Platform {
    public:
        static QString platformOS();
        static QString compiler();
        static QString memoryFree();
        static QString memoryOverall();
        static QString id();
        static QString compileMode();
};

#endif // PLATFORM_H
