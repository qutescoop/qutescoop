# #####################################################################
# This file is part of QuteScoop. See README for license
# #####################################################################
TEMPLATE = app
CONFIG *= qt
#CONFIG = debug
#CONFIG += release
CONFIG *= warn_off
TARGET = QuteScoop
win32:PLATFORM = "win32"
macx:PLATFORM = "macx"
unix:PLATFORM = "unix"

QT *= core gui network opengl xml

CONFIG(debug,release|debug) {
    !build_pass:message("DEBUG")
    DEBUGRELEASE = "debug"
    DESTDIR = ./

    # If precompiled headers are not possible, qmake should deactivate it.
    # If compiling/linking problems arise, this should be deactivated.
    CONFIG += precompile_header
    PRECOMPILED_HEADER = src/_pch.h
    precompile_header:!isEmpty(PRECOMPILED_HEADER) {
        !build_pass:message("Using precompiled headers.")
    }
}

# Included for release/dist
CONFIG(release,release|debug) {
    !build_pass:message("RELEASE")
    DEBUGRELEASE = "release"
    DESTDIR = ./DIST-$${PLATFORM}
    # Add a "make install" target for deploying Qt/compiler/QuteScoop files.
    # Qt library files
    myQtLib.path = $$DESTDIR
    myQtLib.files += $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}QtCore4.*
    myQtLib.files += $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}QtGui4.*
    myQtLib.files += $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}QtNetwork4.*
    myQtLib.files += $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}QtXml4.*
    myQtLib.files += $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}QtOpenGL4.*
    !build_pass:message("Library files added to 'install': $${myQtLib.files}")

    # Compiler libraries
    win32-g++ { # For MingW
        myCompilerLib.path = $$DESTDIR
        myCompilerLib.files += $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}mingwm10.dll
        myCompilerLib.files += $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}libgcc_s_dw2-1.dll
        !build_pass:message("MingW compiler libraries added to 'install': $${myCompilerLib.files}")
    } else {
        !build_pass:message("No compiler libraries added to 'install'")
    }

    # QuteScoop additional files
    rootFiles.path = $$DESTDIR
    rootFiles.files += README.html COPYING CHANGELOG
    unix: rootFiles.files += QuteScoop.sh
    dataFiles.path = $$DESTDIR/data
    dataFiles.files += ./data/+notes.txt
    dataFiles.files += ./data/airports.dat
    dataFiles.files += ./data/coastline.dat
    dataFiles.files += ./data/countries.dat
    dataFiles.files += ./data/countrycodes.dat
    dataFiles.files += ./data/dataversions.txt
    dataFiles.files += ./data/firdisplay.dat
    dataFiles.files += ./data/firdisplay.sup
    dataFiles.files += ./data/firlist.dat
    downloadedFiles.path = $$DESTDIR/downloaded
    downloadedFiles.files += ./downloaded/+notes.txt
    screenshotsFiles.path = $$DESTDIR/screenshots
    screenshotsFiles.files += ./screenshots/+notes.txt
    texturesFiles.path = $$DESTDIR/textures
    texturesFiles.files += ./textures/+notes.txt
    texturesFiles.files += ./textures/1024px-continents.png
    texturesFiles.files += ./textures/1024px-toposhaded.png
    texturesFiles.files += ./textures/1440px-elevation.png
    texturesFiles.files += ./textures/2048px.png
    texturesFiles.files += ./textures/2048px-color.png
    texturesFiles.files += ./textures/2048px-contrast.png
    texturesFiles.files += ./textures/2048px-lights.png
    texturesFiles.files += ./textures/2048px-toposhaded.png
    texturesFiles.files += ./textures/4096px.png
    texturesFiles.files += ./textures/4096px-color.png
    !build_pass:message("QuteScoop files added to 'install': $${rootFiles.files} $${dataFiles.files} \
        $${downloadedFiles.files} $${texturesFiles.files} $${screenShotFiles.files}")

    # Adds an "install" target for make, executed by "make install"
    # (Can be added to QtCreator project also as build step)
    INSTALLS *= rootFiles dataFiles downloadedFiles screenshotsFiles texturesFiles myQtLib myCompilerLib
}

macx { # could be "mac" also, I am not sure
    CONFIG += app_bundle
    ICON = src/Dolomynum.icns
    CONFIG *= x86
    CONFIG *= ppc
}
mac { # could be "macx" also, I am not sure
    CONFIG += app_bundle
    ICON = src/Dolomynum.icns
    CONFIG *= x86
    CONFIG *= ppc
}
win32 {
    RC_FILE = src/windowsicon.rc
}
unix {
}

# Input
FORMS = src/MainWindow.ui \
    src/PilotDetails.ui \
    src/ClientSelectionDialog.ui \
    src/ControllerDetails.ui \
    src/AirportDetails.ui \
    src/PreferencesDialog.ui \
    src/PlanFlightDialog.ui \
    src/BookedAtcDialog.ui \
    src/ListClientsDialog.ui

HEADERS += src/_pch.h \
    src/WhazzupData.h \
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
    src/ListClientsDialogModel.h \
    src/ListClientsDialog.h \
    src/Ping.h \
    src/LogBrowserDialog.h \
    src/GuiMessage.h

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
    src/ListClientsDialogModel.cpp \
    src/ListClientsDialog.cpp \
    src/Ping.cpp \
    src/LogBrowserDialog.cpp
RESOURCES += src/Resources.qrc
OTHER_FILES += CHANGELOG \
    README.html \
    downloaded/+notes.txt \
    data/+notes.txt \
    data/dataversions.txt \
    screenshots/+notes.txt \
    textures/+notes.txt

# temp files
MOC_DIR =       ./temp/$${PLATFORM}-$${DEBUGRELEASE}
UI_DIR =        ./temp/$${PLATFORM}-$${DEBUGRELEASE}
OBJECTS_DIR =   ./temp/$${PLATFORM}-$${DEBUGRELEASE}
RCC_DIR =       ./temp/$${PLATFORM}-$${DEBUGRELEASE}
