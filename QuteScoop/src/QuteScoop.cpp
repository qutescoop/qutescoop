/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include <QApplication>
#include <QDebug>
#include <QtGui>
#include <QMetaType>
#include "Window.h"
#include "LogBrowserDialog.h"
#include "helpers.h"

void myMessageOutput(QtMsgType type, const char *msg)
{
    // LogBrowser output
    if(LogBrowserDialog::getInstance(false) != 0)
        LogBrowserDialog::getInstance()->outputMessage(type, msg);

    // log.txt output
    QFile logFile("log.txt");
    logFile.open(QIODevice::Append);
    if (logFile.write(QByteArray::number(type).append(": ").append(msg).append("\r\n")) < 0)
        qCritical() << "Error writig to logfile";
    if (logFile.isOpen()) logFile.close();

    // normal output
    qInstallMsgHandler(0);

    switch (type) {
    case QtDebugMsg:
        qDebug(msg);
        break;
    case QtWarningMsg:
        qWarning(msg);
        break;
    case QtCriticalMsg:
        qCritical(msg);
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
    QCoreApplication::setOrganizationName("QuteScoop");
    QCoreApplication::setOrganizationDomain("qutescoop.org");
    QCoreApplication::setApplicationName("QuteScoop");
    app.setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));

    // catch all messages
    QFile::remove("log.txt");
    QFile logFile("log.txt");
    logFile.open(QIODevice::WriteOnly);
    logFile.write(QByteArray(VERSION_STRING.toAscii()).append("\r\n"));
    qRegisterMetaType<QtMsgType>("QtMsgType");
    qInstallMsgHandler(myMessageOutput);

    // some debugging
    qDebug() << "we are looking for locations that are nice to use for downloaded data and other stuff, especially on Mac and Linux";
    qDebug() << "here are some that might be useful:";
    qDebug() << "Home:" << QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    qDebug() << "Data:" << QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    qDebug() << "Documents:" << QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);

    // splash screen
    QPixmap pixmap(":/splash/splash");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();
    splash->showMessage("Loading data...", Qt::AlignCenter, QColor(0, 24, 81));
    app.processEvents();

    // create main window
    Window *window = Window::getInstance();
    window->setWindowTitle(QString("QuteScoop %1").arg(VERSION_NUMBER));

    splash->showMessage("all done...", Qt::AlignCenter, QColor(0, 24, 81));
    splash->repaint();
    app.processEvents();
    window->show();

    splash->finish(window);

    return app.exec();
}

