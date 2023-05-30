#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <QString>

class Platform {
    public:
        static QString platformOS();
        static QString compiler();
        static QString compileMode();
        static QString version();
};

#endif // PLATFORM_H
