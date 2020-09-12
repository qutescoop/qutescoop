/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "_pch.h"

#include "Platform.h"

/*
          return platform information string
        */
QString Platform::platformOS() {
    QString os;

#ifdef Q_WS_WIN
    os += "[WS_WIN]";
    switch (QSysInfo::WindowsVersion) {
    case QSysInfo::WV_32s:      os += "[WV_32s]";      break;
    case QSysInfo::WV_95:       os += "[WV_95]";       break;
    case QSysInfo::WV_98:       os += "[WV_98]";       break;
    case QSysInfo::WV_Me:       os += "[WV_Me]";       break;
    case QSysInfo::WV_NT:       os += "[WV_NT]";       break;
    case QSysInfo::WV_2000:     os += "[WV_2000]";     break;
    case QSysInfo::WV_XP:       os += "[WV_XP]";       break;
    case QSysInfo::WV_2003:     os += "[WV_2003]";     break;
    case QSysInfo::WV_VISTA:    os += "[WV_VISTA]";    break;
    case QSysInfo::WV_WINDOWS7: os += "[WV_WINDOWS7]"; break;
    case QSysInfo::WV_CE:       os += "[WV_CE]";       break;
    case QSysInfo::WV_CENET:    os += "[WV_CENET]";    break;
    case QSysInfo::WV_CE_5:     os += "[WV_CE_6]";     break;
    default: os += QString("[unknown WIN (QSysInfo::WindowsVersion=%1)]")
                .arg(QSysInfo::WindowsVersion);
    }
#endif
#ifdef Q_WS_MAC
    os += "[WS_MAC]";
    switch (QSysInfo::MacintoshVersion) {
    case QSysInfo::MV_9:        os += "[MV_9]";       break;
    case QSysInfo::MV_10_0:     os += "[MV_10_0]";    break;
    case QSysInfo::MV_10_1:     os += "[MV_10_1]";    break;
    case QSysInfo::MV_10_2:     os += "[MV_10_2]";    break;
    case QSysInfo::MV_10_3:     os += "[MV_10_3]";    break;
    case QSysInfo::MV_10_4:     os += "[MV_10_4]";    break;
    case QSysInfo::MV_10_5:     os += "[MV_10_5]";    break;
    case QSysInfo::MV_10_6:     os += "[MV_10_6]";    break;
    case QSysInfo::MV_10_7:     os += "[MV_10_7]";    break;
    case QSysInfo::MV_10_8:     os += "[MV_10_8]";    break;
    case QSysInfo::MV_Unknown:  os += "[MV_Unknown]"; break;
    default: os += QString("[unknown MAC (QSysInfo::MacintoshVersion=%1)]")
                .arg(QSysInfo::MacintoshVersion);
    }
#endif
#ifdef Q_OS_LINUX
    os += "[OS_LINUX]";
#endif
#ifdef Q_WS_QWS
    os += "[WS_QWS (embedded Linux)]";
#endif
#ifdef Q_WS_QPA
    os += "[WS_QPA (embedded Linux lite)]";
#endif
#ifdef Q_WS_S60
    os += "[WS_S60]";
#endif

#ifdef Q_OS_AIX
    os += "[OS_AIX]";
#endif
#ifdef Q_OS_BSD4
    os += "[OS_BSD4]";
#endif
#ifdef Q_OS_BSDI
    os += "[OS_BSDI]";
#endif
#ifdef Q_OS_CYGWIN
    os += "[OS_CYGWIN]";
#endif
#ifdef Q_OS_DARWIN
    os += "[OS_DARWIN]";
#endif
#ifdef Q_OS_DGUX
    os += "[OS_DGUX]";
#endif
#ifdef Q_OS_DYNIX
    os += "[OS_DYNIX]";
#endif
#ifdef Q_OS_FREEBSD
    os += "[OS_FREEBSD]";
#endif
#ifdef Q_OS_HPUX
    os += "[OS_HPUX]";
#endif
#ifdef Q_OS_HURD
    os += "[OS_HURD]";
#endif
#ifdef Q_OS_IRIX
    os += "[OS_IRIX]";
#endif
#ifdef Q_OS_LINUX
    os += "[OS_LINUX]";
#endif
#ifdef Q_OS_LYNX
    os += "[OS_LYNX]";
#endif
#ifdef Q_OS_MAC
    os += "[OS_MAC]";
#endif
#ifdef Q_OS_MSDOS
    os += "[OS_MSDOS]";
#endif
#ifdef Q_OS_NETBSD
    os += "[OS_NETBSD]";
#endif
#ifdef Q_OS_OS2
    os += "[OS_OS2]";
#endif
#ifdef Q_OS_OPENBSD
    os += "[OS_OPENBSD]";
#endif
#ifdef Q_OS_OS2EMX
    os += "[OS_OS2EMX]";
#endif
#ifdef Q_OS_OSF
    os += "[OS_OSF]";
#endif
#ifdef Q_OS_QNX
    os += "[OS_QNX]";
#endif
#ifdef Q_OS_RELIANT
    os += "[OS_RELIANT]";
#endif
#ifdef Q_OS_SCO
    os += "[OS_SCO]";
#endif
#ifdef Q_OS_SOLARIS
    os += "[OS_SOLARIS]";
#endif
#ifdef Q_OS_SYMBIAN
    os += "[OS_SYMBIAN version" + QSysInfo::symbianVersion() + "]";
#endif
#ifdef Q_OS_ULTRIX
    os += "[OS_ULTRIX]";
#endif
#ifdef Q_OS_UNIX
    os += "[OS_UNIX]";
#endif
#ifdef Q_OS_UNIXWARE
    os += "[OS_UNIXWARE]";
#endif
#ifdef Q_OS_WIN32
    os += "[OS_WIN32]";
#endif
#ifdef Q_OS_WINCE
    os += "[OS_WINCE]";
#endif

    os += QString (" | %1 bit, %2 endian")
            .arg(QSysInfo::WordSize)
            .arg(QSysInfo::ByteOrder == QSysInfo::BigEndian? "big": "little");

    return os;
}

QString Platform::compiler() {
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

QString Platform::memoryFree() {
#ifdef Q_OS_LINUX
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

QString Platform::memoryOverall() {
#ifdef Q_OS_LINUX
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

/**
  returns a machine id based on the first real network interface's Mac address
**/
QString Platform::id() {
    const QList<QNetworkInterface> &ifs = QNetworkInterface::allInterfaces();
    for (int i = 0; i < ifs.size(); i++) {
        if (!(ifs[i].flags() & QNetworkInterface::IsLoopBack))
            return ifs[i].hardwareAddress().replace(":", "").toLower();
    }
    return QString();
}

