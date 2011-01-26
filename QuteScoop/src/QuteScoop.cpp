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

void myMessageOutput(QtMsgType type, const char *msg)
{
    // LogBrowser output
    if(LogBrowserDialog::getInstance(false) != 0)
        LogBrowserDialog::getInstance(true)->outputMessage(type, msg);

    // log.txt output
    QFile logFile(qApp->applicationDirPath() + "/log.txt");
    logFile.open(QIODevice::Append | QIODevice::Text);
    if (logFile.write(QByteArray::number(type).append(": ").append(msg).append("\n")) < 0)
        qCritical() << "Error writig to logfile";
    if (logFile.isOpen()) logFile.close();

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
    qApp->processEvents();
    qInstallMsgHandler(myMessageOutput);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("QuteScoop");
    app.setOrganizationDomain("qutescoop.org");
    app.setApplicationName("QuteScoop");
    app.setApplicationVersion(VERSION_STRING);
    app.setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));

    // Overwrite logfile and start with VERSION_STRING
    QFile logFile(qApp->applicationDirPath() + "/log.txt");
    logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    logFile.write(QByteArray(VERSION_STRING.toAscii()).append("\n"));
    if (logFile.isOpen()) logFile.close();
    // catch all messages
    qRegisterMetaType<QtMsgType>("QtMsgType");
    qInstallMsgHandler(myMessageOutput);

    // some debugging
    qDebug() << "we are looking for locations that are nice to use for downloaded data and other stuff, especially on Mac and Linux";
    qDebug() << "here are some that might be useful:";
    qDebug() << "Home:" << QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    qDebug() << "Documents:" << QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    qDebug() << "Data:" << QDesktopServices::storageLocation(QDesktopServices::DataLocation);
                    // on Mac: /Users/<user>/Library/Application Support/QuteScoop/QuteScoop
                    // on Ubuntu: /home/<user>/.local/share/data/QuteScoop/QuteScoop
                    // on WinXP 32: C:\Dokumente und Einstellungen\<user>\Lokale Einstellungen\Anwendungsdaten\QuteScoop\QuteScoop
                    // on Win7 64: \Users\<user>\AppData\local\QuteScoop\QuteScoop

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
