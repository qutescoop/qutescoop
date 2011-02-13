/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "_pch.h"

#include "Window.h"
#include "NavData.h"
#include "LogBrowserDialog.h"
#include "helpers.h"
#include "Settings.h"

QFile *logFile = new QFile();
QByteArray cacheLogByteArray;
void myMessageOutput(QtMsgType type, const char *msg)
{
    // uninstall this routine for "normal" debug output
    qInstallMsgHandler(0);

    QByteArray output = QByteArray::number(type).append(": ").append(msg);

    // LogBrowser output
    if(LogBrowserDialog::getInstance(false) != 0)
        LogBrowserDialog::getInstance(true)->outputMessage(output);

    // log.txt output
    if (logFile->isWritable()) {
        logFile->write(output.append("\n"));
        logFile->flush(); // make sure the last messages before a crash are written
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

int main(int argc, char *argv[]) {
    QT_REQUIRE_VERSION(argc, argv, "4.7.0")
    QApplication app(argc, argv);
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
    cacheLogByteArray.append(VERSION_STRING + "\n");
    cacheLogByteArray.append(QString("Compiled on Qt %1, running on Qt %2.\n").arg(QT_VERSION_STR, qVersion()));
#ifdef QT_NO_DEBUG
    cacheLogByteArray.append("COMPILED IN RELEASE MODE - this is best performance-wise.\n");
#endif
#ifdef QT_DEBUG
    cacheLogByteArray.append("COMPILED IN DEBUG MODE - performance might be degraded.\n");
#endif
#ifdef QT_NO_DEBUG_OUTPUT
    cacheLogByteArray.append("COMPILED WITHOUT DEBUG OUTPUT - no debug messages will be captured.\n");
#endif
    cacheLogByteArray.append("message levels: 0 - DEBUG, 1 - WARNING, 2 - CRITICAL/SYSTEM, 3 - FATAL\n\n");

    // directories
    Settings::calculateApplicationDataDirectory();

    // Open log.txt
    logFile->setFileName(Settings::applicationDataDirectory("log.txt"));
    logFile->open(QIODevice::WriteOnly | QIODevice::Text);
    logFile->write(cacheLogByteArray); // debug messages ended up there until now
    logFile->flush();

    qDebug() << "Expecting application data directory at" << Settings::applicationDataDirectory() << "(gets calculated on each start)";

    // splash screen
    QPixmap pixmap(":/splash/splash");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();
    QString splashMsg;

    // Loading Navdata
    splashMsg.append("Loading navdata...");
    splash->showMessage(splashMsg, Qt::AlignCenter, QColor(0, 24, 81));
    app.processEvents();
    NavData::getInstance();

    // create main window
    splashMsg.append("\nSetting up main window and OpenGL...");
    splash->showMessage(splashMsg, Qt::AlignCenter, QColor(0, 24, 81));
    app.processEvents();
    Window *window = Window::getInstance(true);

    // ready
    splashMsg.append("\nStartup completed");
    splash->showMessage(splashMsg, Qt::AlignCenter, QColor(0, 24, 81));
    splash->finish(window);
    window->show();
    delete splash;

    // starting event loop
    return app.exec();

    // closing log
    if (logFile->isOpen())
        logFile->close();
    delete logFile;
}
