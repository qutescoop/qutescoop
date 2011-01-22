# #####################################################################
# This file is part of QuteScoop. See README for license
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

DESTDIR = ./
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
# CONFIG += warn_off
mac {
    CONFIG += app_bundle
    ICON = src/Dolomynum.icns
    CONFIG += x86
#    CONFIG += ppc
}
win32 {
    RC_FILE = src/windowsicon.rc
}

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
    src/Ping.h \
    src/Logbrowser.h \
    src/LogbrowserDialog.h
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
    src/Ping.cpp \
    src/Logbrowser.cpp \
    src/LogbrowserDialog.cpp
RESOURCES += src/Resources.qrc
OTHER_FILES += CHANGELOG \
    README \
    downloaded/+notes.txt \
    data/+notes.txt \
    data/dataversions.txt \
    screenshots/+notes.txt
