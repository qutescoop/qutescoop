/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include <QApplication>
#include <QtGui>
#include "Window.h"
#include "logbrowser.h"

QPointer<LogBrowser> logBrowser;
void myMessageOutput(QtMsgType type, const char *msg)
{
    if(logBrowser)
        logBrowser->outputMessage( type, msg );
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // The LogBrowser shows all messages - debug/warning/critical/fatal
    //logBrowser = new LogBrowser;
    //qInstallMsgHandler(myMessageOutput);

    QCoreApplication::setOrganizationName("QuteScoop");
    QCoreApplication::setOrganizationDomain("qutescoop.org");
    QCoreApplication::setApplicationName("QuteScoop");

    app.setWindowIcon(QIcon(QPixmap(":/icons/qutescoop.png")));

    // splash screen
    QPixmap pixmap(":/splash/splash");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();
    splash->showMessage("Loading data...", Qt::AlignCenter, QColor(0, 24, 81));
    app.processEvents();

    // create main window
    Window *window = Window::getInstance();

    window->show();

    // startup finished
    splash->showMessage("all done...", Qt::AlignCenter, QColor(0, 24, 81));
    app.processEvents();
    splash->finish(window);

    return app.exec();
}

