# #####################################################################
# This file is part of QuteScoop. See README for license
# #####################################################################

# QuteScoop version
VERSION = "2.2-dev" # should not include spaces
VER_STR = '\\"$${VERSION}\\"'
DEFINES += VERSION_NUMBER=\"$$VER_STR\" # complex escaping to preserve
                                            # string through qmake -> console ->
                                            # compiler

# fixes version incompatibilities
QMAKE_CXXFLAGS += "-fno-sized-deallocation"

TEMPLATE = app
CONFIG *= qt

# CONFIG *= debug
# CONFIG *= release
CONFIG *= warn_on
TARGET = QuteScoop

# Let's make sure we do not mix up 32 and 64bit binaries. Provide
# 32bit installers or clearly mark them as 64bit to avaoid confusion.
# Hint: Setting up a virtual machine (e.g. in VirtualBox) is from my
# experience much easier than cross-compiling 32bit from a 64bit host.
win32: {
    contains(QMAKE_TARGET.arch, x86_64):PLATFORM = "win64"
    else:PLATFORM = "win32"
}
macx: {
    # QMAKE_TARGET.arch is only available on Windows?!
    QMAKE_TARGET.arch = $$QMAKE_HOST.arch
    contains(QMAKE_TARGET.arch, x86_64):PLATFORM = "macx64"
    else:PLATFORM = "macx32"
}
!macx:unix: {
    # QMAKE_TARGET.arch is only available on Windows?!
    linux-g++:QMAKE_TARGET.arch = $$QMAKE_HOST.arch
    # allow for 32bit cross-compiling on 64bit with g++ (_HOST and _TARGET different)
    linux-g++-32:QMAKE_TARGET.arch = x86
    linux-g++-64:QMAKE_TARGET.arch = x86_64

    contains(QMAKE_TARGET.arch, x86_64):PLATFORM = "unix64"
    else:PLATFORM = "unix32"
}

# Qt libraries
# finding Qt's paths:
#message(Qt version: $$[QT_VERSION])
#message(Qt is installed in $$[QT_INSTALL_PREFIX])
#message(Documentation: $$[QT_INSTALL_DOCS])
#message(Header files: $$[QT_INSTALL_HEADERS])
message(Libraries: $$[QT_INSTALL_LIBS])
message(Binary files (executables): $$[QT_INSTALL_BINS])
message(Plugins: $$[QT_INSTALL_PLUGINS])
#message(Data files: $$[QT_INSTALL_DATA])
#message(Translation files: $$[QT_INSTALL_TRANSLATIONS])
#message(Settings: $$[QT_INSTALL_SETTINGS])
#message(Examples: $$[QT_INSTALL_EXAMPLES])
#message(Demonstrations: $$[QT_INSTALL_DEMOS])

QT *= core gui network opengl xml
# in debug mode, we output to current directory
CONFIG(debug,release|debug) { 
    !build_pass:message("DEBUG")
    DEBUGRELEASE = "debug"
    DESTDIR = ./
    
    # If precompiled headers are not possible, qmake should deactivate it.
    # If compiling/linking problems arise, this should be deactivated.
    # uncomment to activate:
    #CONFIG += precompile_header
    PRECOMPILED_HEADER = src/_pch.h
    precompile_header:!isEmpty(PRECOMPILED_HEADER):!build_pass:message("Using precompiled headers.")
}

# in release mode, we include a 'make install' target and output to ./DIST-$PLATFORM
CONFIG(release,release|debug) { 
    !build_pass:message("RELEASE")
    DEBUGRELEASE = "release"
    DESTDIR = ./DIST-$${PLATFORM}
    
    # Add a "make install" target for deploying Qt/compiler/QuteScoop files.

    # QuteScoop additional files
    rootFiles.path = $$DESTDIR
    rootFiles.files += ./README.html \
        ./COPYING \
        ./CHANGELOG

    unix:rootFiles.files += ./QuteScoop.sh \
        ./QuteScoop.desktop \
        ./src/qutescoop.png \
        ./picsToMovie.sh
    dataFiles.path = $$DESTDIR/data
    dataFiles.files += ./data/_notes.txt
    dataFiles.files += ./data/airports.dat
    dataFiles.files += ./data/coastline.dat
    dataFiles.files += ./data/countries.dat
    dataFiles.files += ./data/countrycodes.dat
    dataFiles.files += ./data/dataversions.txt
    dataFiles.files += ./data/firdisplay.dat
    dataFiles.files += ./data/firdisplay.sup
    dataFiles.files += ./data/firlist.dat
    dataFiles.files += ./data/airlines.dat
    dataFiles.files += ./data/station.dat
    dataFiles.files += ./data/cloudmirrors.dat
    dataFiles.files += "./data/clouds/_notes.txt"
    downloadedFiles.path = $$DESTDIR/downloaded
    downloadedFiles.files += ./downloaded/_notes.txt
    screenshotsFiles.path = $$DESTDIR/screenshots
    screenshotsFiles.files += ./screenshots/_notes.txt
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
    cloudsFiles.path = $$DESTDIR/textures/clouds
    cloudsFiles.files += ./textures/clouds/_notes.txt
    !build_pass:message("QuteScoop files added to 'install': $${rootFiles.files} $${dataFiles.files} $${downloadedFiles.files} $${texturesFiles.files} $${screenShotFiles.files}.")
    !build_pass:message("Run 'make install' to copy them to the correct locations")
    
    # Adds an "install" target for make, executed by "make install"
    # (Can be added to QtCreator project also as build step)
    INSTALLS *= rootFiles \
        dataFiles \
        downloadedFiles \
        screenshotsFiles \
        texturesFiles \
        cloudsFiles 
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
    LIBS += -lGLU
}

# Input
FORMS = \
    src/PilotDetails.ui \
    src/ControllerDetails.ui \
    src/AirportDetails.ui \
    src/PreferencesDialog.ui \
    src/PlanFlightDialog.ui \
    src/BookedAtcDialog.ui \
    src/ListClientsDialog.ui\
    src/SectorView.ui \
    src/Window.ui
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
    src/MapScreen.h\
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
    src/GuiMessage.h \
    src/Station.h \
    src/Launcher.h \
    src/SectorView.h \
    src/Net.h \
    src/SondeData.h \
    src/JobList.h \
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
    src/LogBrowserDialog.cpp \
    src/GuiMessage.cpp \
    src/Station.cpp \
    src/Launcher.cpp \
    src/SectorView.cpp \
    src/Net.cpp \
    src/SondeData.cpp \
    src/JobList.cpp \
    src/Platform.cpp
RESOURCES += src/Resources.qrc
OTHER_FILES += CHANGELOG \
    README.html \
    data/_notes.txt \
    data/dataversions.txt \
    data/station.dat \
    data/firlist.dat \
    data/firdisplay.dat \
    data/firdisplay.sup \
    data/countrycodes.dat \
    data/countries.dat \
    data/coastline.dat \
    data/cloudmirrors.dat \
    data/airports.dat \
    data/airlines.dat \
    QuteScoop-upload.pri \
    downloaded/_notes.txt \
    screenshots/_notes.txt \
    textures/_notes.txt \
    textures/clouds/_notes.txt
macx:OTHER_FILES += \
    macbundle.sh
unix:OTHER_FILES += \
    QuteScoop.sh \
    QuteScoop.desktop \
    picsToMovie.sh

# Report DESTDIR to user
!build_pass:message("Compiled $$TARGET will be put to $$DESTDIR")

# temp files
MOC_DIR = ./temp/$${PLATFORM}-$${DEBUGRELEASE}
UI_DIR = ./temp/$${PLATFORM}-$${DEBUGRELEASE}
OBJECTS_DIR = ./temp/$${PLATFORM}-$${DEBUGRELEASE}
RCC_DIR = ./temp/$${PLATFORM}-$${DEBUGRELEASE}

# include 'make installer-*' targets which build installers for Linux & Win
include("QuteScoop-makeInstaller.pri")
