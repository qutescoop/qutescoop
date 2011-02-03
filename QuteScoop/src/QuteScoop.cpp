/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include <QApplication>
#include <QDebug>
#include <QtGui>
#include <QMetaType>
#include "Window.h"
#include "NavData.h"
#include "LogBrowserDialog.h"
#include "helpers.h"
#include "Settings.h"

void myMessageOutput(QtMsgType type, const char *msg)
{
    // LogBrowser output
    if(LogBrowserDialog::getInstance(false) != 0)
        LogBrowserDialog::getInstance(true)->outputMessage(type, msg);

    // log.txt output
    QFile logFile(Settings::applicationDataDirectory("log.txt"));
    logFile.open(QIODevice::Append | QIODevice::Text);
    if (logFile.write(QByteArray::number(type).append(": ").append(msg).append("\n")) < 0)
        qCritical() << "Error writing to logfile";
    if (logFile.isOpen()) logFile.close(); // gets opened and closed on EACH write.
                                //I guess this is best to not lose contents during a crash.

    // normal output
    qInstallMsgHandler(0);

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
    //qApp->processEvents();
    qInstallMsgHandler(myMessageOutput);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("QuteScoop");
    app.setOrganizationDomain("qutescoop.org");
    app.setApplicationName("QuteScoop");
    app.setApplicationVersion(VERSION_STRING);
    app.setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));

    // directories
    Settings::setApplicationDataDirectory(
            QFileInfo(Settings::calculateApplicationDataDirectory()).absoluteFilePath());

    // Overwrite logfile and start with VERSION_STRING
    QFile logFile(Settings::applicationDataDirectory("log.txt"));
    logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    logFile.write(QByteArray(VERSION_STRING.toAscii()).append("\n"));
    logFile.write("message levels: 0 - DEBUG, 1 - WARNING, 2 - CRITICAL/SYSTEM, 3 - FATAL\n\n");
    if (logFile.isOpen()) logFile.close();
    // catch all messages
    qRegisterMetaType<QtMsgType>("QtMsgType");
    qInstallMsgHandler(myMessageOutput);

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

    return app.exec();
}
