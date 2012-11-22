/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef HELPERSPLATFORM_H
#define HELPERSPLATFORM_H

#include "_pch.h"

/*
  return platform information string
*/
QString platformOS() {
    QString os;

#ifdef Q_WS_WIN
    os += "[compiled on WS_WIN]";
    switch (QSysInfo::WindowsVersion) {
        case QSysInfo::WV_32s:      os += "[running on WV_32s]";      break;
        case QSysInfo::WV_95:       os += "[running on WV_95]";       break;
        case QSysInfo::WV_98:       os += "[running on WV_98]";       break;
        case QSysInfo::WV_Me:       os += "[running on WV_Me]";       break;
        case QSysInfo::WV_NT:       os += "[running on WV_NT]";       break;
        case QSysInfo::WV_2000:     os += "[running on WV_2000]";     break;
        case QSysInfo::WV_XP:       os += "[running on WV_XP]";       break;
        case QSysInfo::WV_2003:     os += "[running on WV_2003]";     break;
        case QSysInfo::WV_VISTA:    os += "[running on WV_VISTA]";    break;
        case QSysInfo::WV_WINDOWS7: os += "[running on WV_WINDOWS7]"; break;
        case QSysInfo::WV_CE:       os += "[running on WV_CE]";       break;
        case QSysInfo::WV_CENET:    os += "[running on WV_CENET]";    break;
        case QSysInfo::WV_CE_5:     os += "[running on WV_CE_6]";     break;
        default: os += QString("[running on unknown WIN (QSysInfo::WindowsVersion=%1)]")
                    .arg(QSysInfo::WindowsVersion);
    }
#endif
#ifdef Q_WS_MAC
    os += "[compiled on WS_MAC]";
    switch (QSysInfo::MacintoshVersion) {
        case QSysInfo::MV_9;        os += "[running on MV_9]";       break;
        case QSysInfo::MV_10_0;     os += "[running on MV_10_0]";    break;
        case QSysInfo::MV_10_1;     os += "[running on MV_10_1]";    break;
        case QSysInfo::MV_10_2;     os += "[running on MV_10_2]";    break;
        case QSysInfo::MV_10_3;     os += "[running on MV_10_3]";    break;
        case QSysInfo::MV_10_4;     os += "[running on MV_10_4]";    break;
        case QSysInfo::MV_10_5;     os += "[running on MV_10_5]";    break;
        case QSysInfo::MV_10_6;     os += "[running on MV_10_6]";    break;
        case QSysInfo::MV_10_7;     os += "[running on MV_10_7]";    break;
        case QSysInfo::MV_10_8;     os += "[running on MV_10_8]";    break;
        case QSysInfo::MV_Unknown;  os += "[running on MV_Unknown]"; break;
        default: os += QString("[running on unknown MAC (QSysInfo::MacintoshVersion=%1)]")
                    .arg(QSysInfo::MacintoshVersion);
    }
#endif
#ifdef Q_WS_X11
    os += "[compiled on WS_X11]";
#endif
#ifdef Q_WS_QWS
    os += "[compiled on WS_QWS (embedded Linux)]";
#endif
#ifdef Q_WS_QPA
    os += "[compiled on WS_QPA (embedded Linux lite)]";
#endif
#ifdef Q_WS_S60
    os += "[compiled on WS_S60]";
#endif

#ifdef Q_OS_AIX
    os += "[compiled on OS_AIX]";
#endif
#ifdef Q_OS_BSD4
    os += "[compiled on OS_BSD4]";
#endif
#ifdef Q_OS_BSDI
    os += "[compiled on OS_BSDI]";
#endif
#ifdef Q_OS_CYGWIN
    os += "[compiled on OS_CYGWIN]";
#endif
#ifdef Q_OS_DARWIN
    os += "[compiled on OS_DARWIN]";
#endif
#ifdef Q_OS_DGUX
    os += "[compiled on OS_DGUX]";
#endif
#ifdef Q_OS_DYNIX
    os += "[compiled on OS_DYNIX]";
#endif
#ifdef Q_OS_FREEBSD
    os += "[compiled on OS_FREEBSD]";
#endif
#ifdef Q_OS_HPUX
    os += "[compiled on OS_HPUX]";
#endif
#ifdef Q_OS_HURD
    os += "[compiled on OS_HURD]";
#endif
#ifdef Q_OS_IRIX
    os += "[compiled on OS_IRIX]";
#endif
#ifdef Q_OS_LINUX
    os += "[compiled on OS_LINUX]";
#endif
#ifdef Q_OS_LYNX
    os += "[compiled on OS_LYNX]";
#endif
#ifdef Q_OS_MAC
    os += "[compiled on OS_MAC]";
#endif
#ifdef Q_OS_MSDOS
    os += "[compiled on OS_MSDOS]";
#endif
#ifdef Q_OS_NETBSD
    os += "[compiled on OS_NETBSD]";
#endif
#ifdef Q_OS_OS2
    os += "[compiled on OS_OS2]";
#endif
#ifdef Q_OS_OPENBSD
    os += "[compiled on OS_OPENBSD]";
#endif
#ifdef Q_OS_OS2EMX
    os += "[compiled on OS_OS2EMX]";
#endif
#ifdef Q_OS_OSF
    os += "[compiled on OS_OSF]";
#endif
#ifdef Q_OS_QNX
    os += "[compiled on OS_QNX]";
#endif
#ifdef Q_OS_RELIANT
    os += "[compiled on OS_RELIANT]";
#endif
#ifdef Q_OS_SCO
    os += "[compiled on OS_SCO]";
#endif
#ifdef Q_OS_SOLARIS
    os += "[compiled on OS_SOLARIS]";
#endif
#ifdef Q_OS_SYMBIAN
    os += "[compiled on OS_SYMBIAN version" + QSysInfo::symbianVersion() + "]";
#endif
#ifdef Q_OS_ULTRIX
    os += "[compiled on OS_ULTRIX]";
#endif
#ifdef Q_OS_UNIX
    os += "[compiled on OS_UNIX]";
#endif
#ifdef Q_OS_UNIXWARE
    os += "[compiled on OS_UNIXWARE]";
#endif
#ifdef Q_OS_WIN32
    os += "[compiled on OS_WIN32]";
#endif
#ifdef Q_OS_WINCE
    os += "[compiled on OS_WINCE]";
#endif

    os += QString (" | %1 bit, %2 endian")
            .arg(QSysInfo::WordSize)
            .arg(QSysInfo::ByteOrder == QSysInfo::BigEndian? "big": "little");

    return os;
}

QString compiler() {
#ifdef Q_CC_SYM
    return "Digital Mars C/C++ (used to be Symantec C++)";
#endif
#ifdef Q_CC_MWERKS
    return "Metrowerks CodeWarrior";
#endif
#ifdef Q_CC_MSVC
    return "Microsoft Visual C/C++, Intel C++ for Windows";
#endif
#ifdef Q_CC_BOR
    return "Borland/Turbo C++";
#endif
#ifdef Q_CC_WAT
    return "Watcom C++";
#endif
#ifdef Q_CC_GNU
    return "GNU C++";
#endif
#ifdef Q_CC_COMEAU
    return "Comeau C++";
#endif
#ifdef Q_CC_EDG
    return "Edison Design Group C++";
#endif
#ifdef Q_CC_OC
    return "CenterLine C++";
#endif
#ifdef Q_CC_SUN
    return "Forte Developer, or Sun Studio C++";
#endif
#ifdef Q_CC_MIPS
    return "MIPSpro C++";
#endif
#ifdef Q_CC_DEC
    return "DEC C++";
#endif
#ifdef Q_CC_HPACC
    return "HP aC++";
#endif
#ifdef Q_CC_USLC
    return "SCO OUDK and UDK";
#endif
#ifdef Q_CC_CDS
    return "Reliant C++";
#endif
#ifdef Q_CC_KAI
    return "KAI C++";
#endif
#ifdef Q_CC_INTEL
    return "Intel C++ for Linux, Intel C++ for Windows";
#endif
#ifdef Q_CC_HIGHC
    return "MetaWare High C/C++";
#endif
#ifdef Q_CC_PGI
    return "Portland Group C++";
#endif
#ifdef Q_CC_GHS
    return "Green Hills Optimizing C++ Compilers";
#endif
#ifdef Q_CC_GCCE
    return "GCCE (Symbian GCCE builds)";
#endif
#ifdef Q_CC_RVCT
    return "ARM Realview Compiler Suite";
#endif
#ifdef Q_CC_NOKIAX86
    return "Nokia x86 (Symbian WINSCW builds)";
#endif
#ifdef Q_CC_CLANG
    return "C++ front-end for the LLVM compiler";
#endif
}

QString memoryFree() {
#ifdef Q_WS_X11
    QProcess p;
    p.start("awk", QStringList() << "/MemFree/ { print $2 }" << "/proc/meminfo");
    p.waitForFinished();
    return (QString("%1 MB").arg(p.readAllStandardOutput().trimmed().toLong() / 1024));
#endif
//#ifdef Q_WS_MAC
//    QProcess p;
//    p.start("sysctl", QStringList() << "kern.version" << "hw.physmem");
//    p.waitForFinished();
//    return p.readAllStandardOutput();
//#endif
#ifdef Q_WS_WIN
    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memory_status)) {
        return QString("%1 MB")
            .arg(memory_status.ullAvailPhys / (1024 * 1024));
    }
#endif
    return "unknown";
}

QString memoryOverall() {
#ifdef Q_WS_X11
    QProcess p;
    p.start("awk", QStringList() << "/MemTotal/ { print $2 }" << "/proc/meminfo");
    p.waitForFinished();
    return (QString("%1 MB").arg(p.readAllStandardOutput().trimmed().toLong() / 1024));
#endif
#ifdef Q_WS_MAC
    QProcess p;
    p.start("sysctl", QStringList() << "kern.version" << "hw.physmem");
    p.waitForFinished();
    return p.readAllStandardOutput();
#endif
#ifdef Q_WS_WIN
    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memory_status)) {
        return QString("%1 MB")
            .arg(memory_status.ullTotalPhys / (1024 * 1024));
    }
#endif
    return "unknown";
}

#endif // HELPERSPLATFORM_H
