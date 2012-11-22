/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "_pch.h"

#include "Window.h"
#include "LogBrowserDialog.h"
#include "helpers.h"
#include "helpersPlatform.h"
#include "Settings.h"
#include "Launcher.h"

#include <QSysInfo>

/* logging */
QFile *logFile = new QFile();
QByteArray cacheLogByteArray;
void myMessageOutput(QtMsgType type, const char *msg) {
    // uninstall this routine for "normal" debug output
    qInstallMsgHandler(0);

    QByteArray output = QByteArray::number(type).append(": ").append(msg);

    // LogBrowser output
    if(LogBrowserDialog::getInstance(false) != 0)
        LogBrowserDialog::getInstance(true)->outputMessage(output);

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
        qDebug() << msg;
        break;
    case QtWarningMsg:
        qWarning() << msg;
        break;
    case QtCriticalMsg:
        qCritical() << msg;
        break;
    case QtFatalMsg:
        qFatal(msg);
        break;
    }
    qInstallMsgHandler(myMessageOutput);
}


/* main */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // before QT_REQUIRE_VERSION to prevent creating duplicate..
    QT_REQUIRE_VERSION(argc, argv, "4.7.0") // ..application objects
    // catch all messages
    qRegisterMetaType<QtMsgType>("QtMsgType");
    qInstallMsgHandler(myMessageOutput);

    // some app init
    app.setOrganizationName("QuteScoop");
    app.setOrganizationDomain("qutescoop.sourceforge.net");
    app.setApplicationName("QuteScoop");
    app.setApplicationVersion(VERSION_STRING);
    app.setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));

    // some initial debug logging
    qDebug() << VERSION_STRING;
    qDebug() << QString("Compiled on Qt %1, running on Qt %2.")
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

    qDebug() << "platform info:\t" << platformOS();
    qDebug() << "RAM:\t" << memoryOverall() << "," << memoryFree() << "free";
    qDebug() << "compiler:\t" << compiler();

    cacheLogByteArray.append(
             "message levels: 0 - DEBUG, 1 - WARNING, 2 - CRITICAL/SYSTEM, 3 - FATAL\n");

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
    Launcher launch;
    launch.fireUp();

    // start event loop
    Window::getInstance(true)->show();
    app.processEvents();

    launch.close();

    int ret = app.exec();

    delete NetworkManager::getInstance(true);

    // close log
    if (logFile->isOpen())
        logFile->close();
    delete logFile;

    return ret;
}
