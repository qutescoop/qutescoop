/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "_pch.h"

#include "Window.h"
#include "helpers.h"
#include "Platform.h"
#include "Settings.h"
#include "Launcher.h"
#include <stdio.h>

/* logging */
QScopedPointer<QFile>   m_logFile;

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    Q_UNUSED(context);
    QTextStream out(m_logFile.data());
    out << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    switch (type)
    {
    case QtInfoMsg:     out << " [INF] "; break;
    case QtDebugMsg:    out << " [DBG] "; break;
    case QtWarningMsg:  out << " [WRN] "; break;
    case QtCriticalMsg: out << " [CRT] "; break;
    case QtFatalMsg:    out << " [FTL] "; break;
    }
    out << msg << Qt::endl;
    out.flush();
}


/* main */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // before QT_REQUIRE_VERSION to prevent creating duplicate..
    QT_REQUIRE_VERSION(argc, argv, "5.15.0"); // ..application objects

    // some app init
    app.setOrganizationName("QuteScoop");
    app.setOrganizationDomain("qutescoop.github.io");
    app.setApplicationName("QuteScoop");
    app.setApplicationVersion(Version::str());
    app.setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));
    //QLocale::setDefault(QLocale::C); // bullet-proof string->float conversions

    // directories
    Settings::calculateApplicationDataDirectory();

    // Open log.txt
    QTextStream(stdout) << "Log output can be found in " << Settings::applicationDataDirectory("log.txt") << Qt::endl;
    QTextStream(stdout) << "Using settings from " << Settings::fileName() << Qt::endl;
    m_logFile.reset(new QFile(Settings::applicationDataDirectory("log.txt")));
    m_logFile.data()->open(QFile::WriteOnly | QFile::Text);

    // catch all messages
    qInstallMessageHandler(messageHandler);

    // some initial debug logging
    qDebug() << Version::str();

    qDebug() << QString("Compiled with Qt %1 [%4, Qt mode: %2], running with Qt %3 on %5.").arg(
                    QT_VERSION_STR, Platform::compileMode(), qVersion(), Platform::compiler(), Platform::platformOS()
                );

    // image format plugins
    app.addLibraryPath(QString("%1/imageformats").arg(app.applicationDirPath()));
    qDebug() << "Library paths:" << app.libraryPaths();
    qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();

    qDebug() << "Expecting application data directory at"
             << Settings::applicationDataDirectory();

    // show Launcher
    Launcher::instance()->fireUp();

    // start event loop
    int ret = app.exec();

    return ret;
}
