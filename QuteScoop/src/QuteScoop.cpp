/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#include <QApplication>
#include <QtGui>
#include "Window.h"

//--------------------------------------------------------------
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

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

