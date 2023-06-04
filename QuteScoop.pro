# versiony things
GIT_HASH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)\\\""
DEFINES += GIT_HASH=$$GIT_HASH

GITHUB_HEAD_REF=$$(GITHUB_HEAD_REF)
isEmpty(GITHUB_HEAD_REF) {
	GIT_BRANCH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" branch --show-current)\\\""
} else {
	GIT_BRANCH="\\\"$$(GITHUB_HEAD_REF)\\\""
}
DEFINES += GIT_BRANCH=$$GIT_BRANCH

## this produces v2.3.0 / v2.3.0-6-g29966c2 / v2.3.0-6-g29966c2-dirty
## C/I sets these "long" versions as tags for pre-releases, so we exclude them here as bases
GIT_DESCRIBE="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" describe --tags --exclude '*-*-*' --dirty --always)\\\""

DEFINES += GIT_DESCRIBE=$$GIT_DESCRIBE
!build_pass:message("compiling version $$GIT_DESCRIBE, branch $$GIT_BRANCH")

# C++20
CONFIG += c++2a

TEMPLATE = app
CONFIG *= qt

CONFIG *= warn_on
TARGET = QuteScoop

DISTFILES += uncrustify.cfg

win32: {
    contains(QMAKE_TARGET.arch, x86_64):PLATFORM = "win64"
    else:PLATFORM = "win32"
}
macx: {
    PLATFORM = "macx64"
}
# linux
!macx:unix: {
    PLATFORM = "unix64"
}

# Qt paths
!build_pass:message(Qt lib: $$[QT_INSTALL_LIBS])
!build_pass:message(Qt bin: $$[QT_INSTALL_BINS])
!build_pass:message(Qt plugins: $$[QT_INSTALL_PLUGINS])

QT *= core gui network opengl xml
# in debug mode, we output to current directory
CONFIG(debug,release|debug) {
    !build_pass:message("DEBUG")
    DEBUGRELEASE = "debug"
    DESTDIR = ./
}

# in release mode, we include a 'make install' target and output to ./DIST-$PLATFORM
CONFIG(release,release|debug) {
    !build_pass:message("RELEASE")
    DEBUGRELEASE = "release"
    DESTDIR = ./DIST-$${PLATFORM}
    
    # Add a "make install" target for deploying Qt/compiler/QuteScoop files.

    rootFiles.path = $$DESTDIR
    rootFiles.files += ./README.md \
        ./COPYING

    unix:rootFiles.files += ./QuteScoop.sh \
        ./QuteScoop.desktop \
        ./src/qutescoop.png \
        ./picsToMovie.sh

    win32:rootFiles.files += ./lib/win64/libssl-1_1-x64.dll \
        ./lib/win64/libcrypto-1_1-x64.dll

    dataFiles.path = $$DESTDIR/data
    dataFiles.files += ./data/_notes.txt
    dataFiles.files += ./data/airports.dat
    dataFiles.files += ./data/controllerAirportsMapping.dat
    dataFiles.files += ./data/coastline.dat
    dataFiles.files += ./data/countries.dat
    dataFiles.files += ./data/countrycodes.dat
    dataFiles.files += ./data/dataversions.txt
    dataFiles.files += ./data/firdisplay.dat
    dataFiles.files += ./data/firlist.dat
    dataFiles.files += ./data/airlines.dat
    downloadedFiles.path = $$DESTDIR/downloaded
    downloadedFiles.files += ./downloaded/_notes.txt
    texturesFiles.path = $$DESTDIR/textures
    texturesFiles.files += ./textures/_notes.txt
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
    texturesFiles.files += ./textures/8192px-arctic-toposhaded.png
    texturesFiles.files += ./textures/8192px-topo.png
    texturesFiles.files += ./textures/10800px.png
    texturesFiles.files += ./textures/10800px-continents.png
    texturesFiles.files += ./textures/10800px-lights.png
    !build_pass:message("Run 'make install' to copy non-code files")
    
    # Adds an "install" target for make, executed by "make install"
    # (Can be added to QtCreator project also as build step)
    INSTALLS *= rootFiles \
        dataFiles \
        downloadedFiles \
        texturesFiles
}

# Notice: 32bit support dropped for OSX, all versions past 10.5 are able to execute 64bit-binaries
macx {
    CONFIG += app_bundle
    ICON = src/Dolomynum.icns
    CONFIG *= x86_64
    LIBS += -framework OpenGL
}
win32 {
    RC_FILE = src/windowsicon.rc
    LIBS += -lOpengl32
    LIBS += -lglu32
}
# OSX also considered as unix, therefore condition added to check
# if the platform is a "real" Unix
!macx:unix {
    ICON = src/images/qs-logo.png
    LIBS += -lGLU
}

# Input
FORMS = \
    src/dialogs/PilotDetails.ui \
    src/dialogs/ControllerDetails.ui \
    src/dialogs/AirportDetails.ui \
    src/dialogs/PreferencesDialog.ui \
    src/dialogs/PlanFlightDialog.ui \
    src/dialogs/BookedAtcDialog.ui \
    src/dialogs/ListClientsDialog.ui\
    src/dialogs/StaticSectorsDialog.ui \
    src/dialogs/Window.ui
HEADERS += \
    src/helpers.h \
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
    src/MapScreen.h\
    src/LineReader.h \
    src/SectorReader.h \
    src/Sector.h \
    src/FileReader.h \
    src/Controller.h \
    src/Client.h \
    src/BookedController.h \
    src/Airline.h \
    src/Airway.h \
    src/Airport.h \
    src/Airac.h \
    src/dialogs/Window.h \
    src/SearchVisitor.h \
    src/SearchResultModel.h \
    src/dialogs/PreferencesDialog.h \
    src/dialogs/PlanFlightDialog.h \
    src/dialogs/PilotDetails.h \
    src/MetarSearchVisitor.h \
    src/MetarModel.h \
    src/MapObjectVisitor.h \
    src/GLWidget.h \
    src/FriendsVisitor.h \
    src/dialogs/ControllerDetails.h \
    src/ClientSelectionWidget.h \
    src/dialogs/ClientDetails.h \
    src/BookedAtcDialogModel.h \
    src/dialogs/BookedAtcDialog.h \
    src/AirportDetailsDeparturesModel.h \
    src/AirportDetailsAtcModel.h \
    src/AirportDetailsArrivalsModel.h \
    src/dialogs/AirportDetails.h \
    src/Route.h \
    src/PlanFlightRoutesModel.h \
    src/BookedAtcSortFilter.h \
    src/ListClientsDialogModel.h \
    src/dialogs/ListClientsDialog.h \
    src/Ping.h \
    src/GuiMessage.h \
    src/Launcher.h \
    src/dialogs/StaticSectorsDialog.h \
    src/Net.h \
    src/JobList.h \
    src/MetarDelegate.h \
    src/Platform.h
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
    src/MapObjectVisitor.cpp \
    src/MapScreen.cpp\
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
    src/dialogs/Window.cpp \
    src/SearchVisitor.cpp \
    src/SearchResultModel.cpp \
    src/dialogs/PreferencesDialog.cpp \
    src/dialogs/PlanFlightDialog.cpp \
    src/dialogs/PilotDetails.cpp \
    src/MetarSearchVisitor.cpp \
    src/MetarModel.cpp \
    src/GLWidget.cpp \
    src/FriendsVisitor.cpp \
    src/dialogs/ControllerDetails.cpp \
    src/ClientSelectionWidget.cpp \
    src/dialogs/ClientDetails.cpp \
    src/BookedAtcDialogModel.cpp \
    src/dialogs/BookedAtcDialog.cpp \
    src/AirportDetailsDeparturesModel.cpp \
    src/AirportDetailsAtcModel.cpp \
    src/AirportDetailsArrivalsModel.cpp \
    src/dialogs/AirportDetails.cpp \
    src/Route.cpp \
    src/PlanFlightRoutesModel.cpp \
    src/BookedAtcSortFilter.cpp \
    src/ListClientsDialogModel.cpp \
    src/dialogs/ListClientsDialog.cpp \
    src/Ping.cpp \
    src/GuiMessage.cpp \
    src/Launcher.cpp \
    src/dialogs/StaticSectorsDialog.cpp \
    src/Net.cpp \
    src/JobList.cpp \
    src/MetarDelegate.cpp \
    src/Platform.cpp
RESOURCES += src/Resources.qrc

# Report DESTDIR to user
!build_pass:message("Compiled $$TARGET will be put to $$DESTDIR")

# temp files
MOC_DIR = ./.cache/$${PLATFORM}-$${DEBUGRELEASE}
UI_DIR = ./.cache/$${PLATFORM}-$${DEBUGRELEASE}
OBJECTS_DIR = ./.cache/$${PLATFORM}-$${DEBUGRELEASE}
RCC_DIR = ./.cache/$${PLATFORM}-$${DEBUGRELEASE}

