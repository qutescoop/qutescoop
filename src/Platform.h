/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <QString>

class Platform {
    public:
        static QString platformOS();
        static QString compiler();
        static QString compileMode();
        static QString version();
};

#endif // PLATFORM_H
