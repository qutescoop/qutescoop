/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Platform.h"

QString Platform::platformOS() {
    return QString("%1:%2:%3").arg(QSysInfo::prettyProductName(), QSysInfo::kernelType(), QSysInfo::kernelVersion());
}

QString Platform::compiler() {
    QString compiler;
#ifdef Q_CC_MSVC
    compiler += "Microsoft Visual C/C++, Intel C++ for Windows";
#endif
#ifdef Q_CC_GNU
    compiler += "GNU C++";
#endif
#ifdef Q_CC_INTEL
    return "Intel C++ for Linux, Intel C++ for Windows";
#endif
#ifdef Q_CC_CLANG
    compiler += "C++ front-end for the LLVM compiler";
#endif
    return compiler;
}

QString Platform::compileMode()
{
#ifdef QT_NO_DEBUG
    return "release";
#endif
#ifdef QT_DEBUG
    return "debug";
#endif
}

QString Platform::version()
{
    // @see QuteScoop.pro
    return QString(GIT_DESCRIBE);
}

