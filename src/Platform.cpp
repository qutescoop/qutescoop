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
    // https://stackoverflow.com/questions/2324658/how-to-determine-the-version-of-the-c-standard-used-by-the-compiler

#if defined(_MSVC_LANG)
    #define cpluspluslevel _MSVC_LANG
#elif defined(__cplusplus)
    #define cpluspluslevel __cplusplus
#else
    #define cpluspluslevel 0
#endif

    compiler += QString(", C++ %1").arg(cpluspluslevel);

    return compiler;
}

QString Platform::compileMode() {
#ifdef QT_NO_DEBUG
    return "release";
#endif
#ifdef QT_DEBUG
    return "debug";
#endif
}

QString Platform::version() {
    // @see QuteScoop.pro
    auto describe = QString(GIT_DESCRIBE);
    auto branch = QString(GIT_BRANCH);
    if (branch == "master" || branch == "main" || branch.startsWith("release/")) {
        return describe;
    }
    return describe + "-" + GIT_BRANCH;
}
