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
FORMS = src/MainWindow.ui \
    src/PilotDetails.ui \
    src/ClientSelectionDialog.ui \
    src/ControllerDetails.ui \
    src/AirportDetails.ui \
    src/PreferencesDialog.ui \
    src/PlanFlightDialog.ui \
    src/BookedAtcDialog.ui \
    src/ListClientsDialog.ui
# DESTDIR = bin/
# DEPENDPATH += ./src
INCLUDEPATH += ./src
MOC_DIR = ./temp
UI_DIR = ./temp
OBJECTS_DIR = ./temp
RCC_DIR = ./temp
QT += network \
    opengl \
    xml

# CONFIG += debug
# CONFIG += release
CONFIG += warn_off
mac {
    ICON = gui/Dolomynum.icns
    CONFIG += x86 \
        ppc
}
win32:RC_FILE = gui/windowsicon.rc

# Input
HEADERS += src/WhazzupData.h \
    src/Whazzup.h \
    src/Waypoint.h \
    src/Tessellator.h \
    src/Settings.h \
    src/Pilot.h \
    src/NavData.h \
    src/NavAid.h \
    src/Metar.h \
    src/MapObject.h \
    src/LineReader.h \
    src/helpers.h \
    src/SectorReader.h \
    src/Sector.h \
    src/FileReader.h \
    src/Controller.h \
    src/Client.h \
    src/BookedController.h \
    src/Airway.h \
    src/Airport.h \
    src/Airac.h \
    src/Window.h \
    src/SearchVisitor.h \
    src/SearchResultModel.h \
    src/PreferencesDialog.h \
    src/PlanFlightDialog.h \
    src/PilotDetails.h \
    src/MetarSearchVisitor.h \
    src/MetarModel.h \
    src/MapObjectVisitor.h \
    src/GLWidget.h \
    src/FriendsVisitor.h \
    src/ControllerDetails.h \
    src/ClientSelectionWidget.h \
    src/ClientDetails.h \
    src/BookedAtcDialogModel.h \
    src/BookedAtcDialog.h \
    src/AirportDetailsDeparturesModel.h \
    src/AirportDetailsAtcModel.h \
    src/AirportDetailsArrivalsModel.h \
    src/AirportDetails.h \
    src/Route.h \
    src/PlanFlightRoutesModel.h \
    src/BookedAtcSortFilter.h \
    src/helpers.h \
    src/ListClientsSortFilter.h \
    src/ListClientsDialogModel.h \
    src/ListClientsDialog.h \
    src/Ping.h
SOURCES += src/WhazzupData.cpp \
    src/Whazzup.cpp \
    src/Waypoint.cpp \
    src/Tessellator.cpp \
    src/Settings.cpp \
    src/QuteScoop.cpp \
    src/Pilot.cpp \
    src/NavData.cpp \
    src/NavAid.cpp \
    src/Metar.cpp \
    src/MapObject.cpp \
    src/LineReader.cpp \
    src/SectorReader.cpp \
    src/Sector.cpp \
    src/FileReader.cpp \
    src/Controller.cpp \
    src/Client.cpp \
    src/BookedController.cpp \
    src/Airway.cpp \
    src/Airport.cpp \
    src/Airac.cpp \
    src/Window.cpp \
    src/SearchVisitor.cpp \
    src/SearchResultModel.cpp \
    src/PreferencesDialog.cpp \
    src/PlanFlightDialog.cpp \
    src/PilotDetails.cpp \
    src/MetarSearchVisitor.cpp \
    src/MetarModel.cpp \
    src/GLWidget.cpp \
    src/FriendsVisitor.cpp \
    src/ControllerDetails.cpp \
    src/ClientSelectionWidget.cpp \
    src/ClientDetails.cpp \
    src/BookedAtcDialogModel.cpp \
    src/BookedAtcDialog.cpp \
    src/AirportDetailsDeparturesModel.cpp \
    src/AirportDetailsAtcModel.cpp \
    src/AirportDetailsArrivalsModel.cpp \
    src/AirportDetails.cpp \
    src/Route.cpp \
    src/PlanFlightRoutesModel.cpp \
    src/BookedAtcSortFilter.cpp \
    src/ListClientsSortFilter.cpp \
    src/ListClientsDialogModel.cpp \
    src/ListClientsDialog.cpp \
    src/Ping.cpp
RESOURCES += src/Resources.qrc
OTHER_FILES += CHANGELOG \
    README \
    downloaded/+notes.txt \
    data/+notes.txt
