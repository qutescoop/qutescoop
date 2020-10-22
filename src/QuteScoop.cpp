/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "_pch.h"

#include "Window.h"
#include "LogBrowserDialog.h"
#include "helpers.h"
#include "Platform.h"
#include "Settings.h"
#include "Launcher.h"

/* logging */
QFile *logFile = new QFile();
QByteArray cacheLogByteArray;
void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    // uninstall this routine for "normal" debug output
    qInstallMessageHandler(0);

    QByteArray output = QByteArray::number(type).append(": ").append(msg);

    // LogBrowser output
    if(LogBrowserDialog::instance(false) != 0)
        LogBrowserDialog::instance()->on_message_new(output);

    // log.txt output
    if (logFile->isWritable()) {
        logFile->write(output.append("\n"));
        // deactivate this during testing if you are sending a lot of debug messages.
        logFile->flush(); // make sure all messages are written. Mind the DebugLogBrowser that
                    // depends on finding a complete file when opened.
    } else // write to buffer while logFile not yet available
        cacheLogByteArray.append(output.append("\n"));

    // "normal" debug output
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
        qDebug() << msg;
        break;
    case QtWarningMsg:
        qWarning() << msg << context.file << context.function << context.line;
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        qCritical() << msg << context.file << context.function << context.line;
        break;
    }
    qInstallMessageHandler(myMessageOutput);
}

/* main */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // before QT_REQUIRE_VERSION to prevent creating duplicate..
    QT_REQUIRE_VERSION(argc, argv, "5.10.0"); // ..application objects
    // catch all messages
    qRegisterMetaType<QtMsgType>("QtMsgType");
    qInstallMessageHandler(myMessageOutput);

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

    cacheLogByteArray.append(
             "Debug log message levels: 0 - DEBUG, 1 - WARNING, 2 - CRITICAL/SYSTEM, 3 - FATAL\n");

    // directories
    Settings::calculateApplicationDataDirectory();

    // Open log.txt
    logFile->setFileName(Settings::applicationDataDirectory("log.txt"));
    logFile->open(QIODevice::WriteOnly | QIODevice::Text);
    logFile->write(cacheLogByteArray); // debug messages ended up there until now
    logFile->flush();

    qDebug() << "Expecting application data directory at"
             << Settings::applicationDataDirectory() << "(gets calculated on each start)";

    // show Launcher
    Launcher::instance()->fireUp();

    // start event loop
    int ret = app.exec();

    // close log
    if (logFile->isOpen())
        logFile->close();
    delete logFile;

    return ret;
}
