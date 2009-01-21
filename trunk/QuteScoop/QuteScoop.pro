######################################################################
#  This file is part of QuteScoop.
#  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
#
#  QuteScoop is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  QuteScoop is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
######################################################################
 
TEMPLATE = app
FORMS = MainWindow.ui PilotDetails.ui ClientSelectionDialog.ui ControllerDetails.ui AirportDetails.ui PreferencesDialog.ui
TARGET = 
DEPENDPATH += ./src ./gui
INCLUDEPATH += ./src ./gui
QT += network opengl
CONFIG += debug
#CONFIG += release

mac {
	ICON = gui/Dolomynum.icns
	#CONFIG += x86 ppc
}

win32 {
	RC_FILE = gui/windowsicon.rc
	#CONFIG += console
}

# Input
HEADERS += GLWidget.h Window.h LineReader.h Client.h \
			FileReader.h FirReader.h Fir.h Tessellator.h \
			Airport.h Whazzup.h WhazzupData.h PilotDetails.h \
			ClientDetails.h Controller.h Pilot.h ClientSelectionWidget.h \
			ControllerDetails.h MapObject.h AirportDetails.h \
			AirportDetailsAtcModel.h AirportDetailsArrivalsModel.h \
			NavData.h AirportDetailsDeparturesModel.h Airway.h \
			PreferencesDialog.h Settings.h SearchResultModel.h \
			MapObjectVisitor.h SearchVisitor.h Waypoint.h MetarModel.h Metar.h \
			MetarSearchVisitor.h FriendsVisitor.h Airac.h NavAid.h

SOURCES += GLWidget.cpp QuteScoop.cpp Window.cpp LineReader.cpp \
			Client.cpp FileReader.cpp FirReader.cpp Fir.cpp \
			Tessellator.cpp Airport.cpp Whazzup.cpp WhazzupData.cpp \
			PilotDetails.cpp ClientDetails.cpp Controller.cpp Pilot.cpp \
			ClientSelectionWidget.cpp ControllerDetails.cpp MapObject.cpp \
			AirportDetails.cpp AirportDetailsAtcModel.cpp \
			AirportDetailsArrivalsModel.cpp NavData.cpp Airway.cpp \
			AirportDetailsDeparturesModel.cpp NavAid.cpp \
			PreferencesDialog.cpp Settings.cpp SearchResultModel.cpp \
			SearchVisitor.cpp Waypoint.cpp MetarModel.cpp Metar.cpp \
			MetarSearchVisitor.cpp FriendsVisitor.cpp Airac.cpp
			
