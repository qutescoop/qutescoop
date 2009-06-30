# #####################################################################
# This file is part of QuteScoop.
# Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
# QuteScoop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# QuteScoop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with QuteScoop. If not, see <http://www.gnu.org/licenses/>
# #####################################################################
TEMPLATE = app
FORMS = gui/MainWindow.ui \
    gui/PilotDetails.ui \
    gui/ClientSelectionDialog.ui \
    gui/ControllerDetails.ui \
    gui/AirportDetails.ui \
    gui/PreferencesDialog.ui \
    gui/PlanFlightDialog.ui \
    gui/BookedAtcDialog.ui
TARGET = 
DEPENDPATH += ./src \
    ./gui
INCLUDEPATH += ./src \
    ./gui
MOC_DIR = ./temp
UI_DIR = ./temp
OBJECTS_DIR = ./temp
RCC_DIR = ./temp
QT += network \
    opengl \
    xml
# CONFIG += debug
# CONFIG += release
#CONFIG += warn_off

mac { 
    ICON = gui/Dolomynum.icns
    CONFIG += x86 \
        ppc
}
win32:RC_FILE = gui/windowsicon.rc
# Input
HEADERS += WhazzupData.h \
    Whazzup.h \
    Waypoint.h \
    Tessellator.h \
    Settings.h \
    Pilot.h \
    NavData.h \
    NavAid.h \
    Metar.h \
    MapObject.h \
    LineReader.h \
    helpers.h \
    FirReader.h \
    Fir.h \
    FileReader.h \
    Controller.h \
    Client.h \
    BookedController.h \
    Airway.h \
    Airport.h \
    Airac.h \
    Window.h \
    SearchVisitor.h \
    SearchResultModel.h \
    PreferencesDialog.h \
    PlanFlightDialog.h \
    PilotDetails.h \
    MetarSearchVisitor.h \
    MetarModel.h \
    MapObjectVisitor.h \
    GLWidget.h \
    FriendsVisitor.h \
    ControllerDetails.h \
    ClientSelectionWidget.h \
    ClientDetails.h \
    BookedAtcDialogModel.h \
    BookedAtcDialog.h \
    AirportDetailsDeparturesModel.h \
    AirportDetailsAtcModel.h \
    AirportDetailsArrivalsModel.h \
    AirportDetails.h \
    src/Route.h \
    gui/PlanFlightRoutesModel.h \
    gui/BookedAtcSortFilter.h \
    src/helpers.h
SOURCES += WhazzupData.cpp \
    Whazzup.cpp \
    Waypoint.cpp \
    Tessellator.cpp \
    Settings.cpp \
    QuteScoop.cpp \
    Pilot.cpp \
    NavData.cpp \
    NavAid.cpp \
    Metar.cpp \
    MapObject.cpp \
    LineReader.cpp \
    FirReader.cpp \
    Fir.cpp \
    FileReader.cpp \
    Controller.cpp \
    Client.cpp \
    BookedController.cpp \
    Airway.cpp \
    Airport.cpp \
    Airac.cpp \
    Window.cpp \
    SearchVisitor.cpp \
    SearchResultModel.cpp \
    PreferencesDialog.cpp \
    PlanFlightDialog.cpp \
    PilotDetails.cpp \
    MetarSearchVisitor.cpp \
    MetarModel.cpp \
    GLWidget.cpp \
    FriendsVisitor.cpp \
    ControllerDetails.cpp \
    ClientSelectionWidget.cpp \
    ClientDetails.cpp \
    BookedAtcDialogModel.cpp \
    BookedAtcDialog.cpp \
    AirportDetailsDeparturesModel.cpp \
    AirportDetailsAtcModel.cpp \
    AirportDetailsArrivalsModel.cpp \
    AirportDetails.cpp \
    src/Route.cpp \
    gui/PlanFlightRoutesModel.cpp \
    gui/BookedAtcSortFilter.cpp
RESOURCES += Resources.qrc
