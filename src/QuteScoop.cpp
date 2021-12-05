/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "_pch.h"

#include "Window.h"
#include "helpers.h"
#include "Platform.h"
#include "Settings.h"
#include "Launcher.h"

/* main */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // before QT_REQUIRE_VERSION to prevent creating duplicate..
    QT_REQUIRE_VERSION(argc, argv, "5.10.0"); // ..application objects
    // catch all messages
    qRegisterMetaType<QtMsgType>("QtMsgType");

    // some app init
    app.setOrganizationName("QuteScoop");
    app.setOrganizationDomain("qutescoop.github.io");
    app.setApplicationName("QuteScoop");
    app.setApplicationVersion(Version::str());
    app.setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));
    //QLocale::setDefault(QLocale::C); // bullet-proof string->float conversions

    // some initial debug logging
    qDebug() << Version::str();
    qDebug() << QString("Compiled with Qt %1, running with Qt %2.")
                             .arg(QT_VERSION_STR, qVersion());
#ifdef QT_NO_DEBUG
    qDebug() << "COMPILED IN RELEASE MODE - this is best performance-wise.";
#endif
#ifdef QT_DEBUG
    qDebug() << "COMPILED IN DEBUG MODE - performance might be degraded.";
#endif
#ifdef QT_NO_DEBUG_OUTPUT
    qDebug() << "COMPILED WITHOUT DEBUG OUTPUT - no debug messages will be captured.";
#endif

    qDebug() << "Platform info:\t" << Platform::platformOS();
    qDebug() << "RAM:\t" << Platform::memoryOverall() << "," << Platform::memoryFree() << "free";
    qDebug() << "Compiler:\t" << Platform::compiler();

    // image format plugins
    app.addLibraryPath(QString("%1/imageformats").arg(app.applicationDirPath()));
    qDebug() << "Library paths:" << app.libraryPaths();
    qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();

    // directories
    Settings::calculateApplicationDataDirectory();

    qDebug() << "Expecting application data directory at"
             << Settings::applicationDataDirectory() << "(gets calculated on each start)";

    // show Launcher
    Launcher::instance()->fireUp();

    // start event loop
    int ret = app.exec();

    return ret;
}
