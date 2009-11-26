/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
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

#include "Airport.h"
#include "Tessellator.h"
#include "helpers.h"
#include "AirportDetails.h"
#include "Window.h"
#include "Settings.h"

#include <QList>

Airport::Airport() {
	appDisplayList = 0;
	twrDisplayList = 0;
	gndDisplayList = 0;
	appBorderDisplayList = 0;
	showFlightLines = false;
	
	resetWhazzupStatus();
}

Airport::Airport(const QStringList& list) {
	appDisplayList = 0;
	twrDisplayList = 0;
	gndDisplayList = 0;
	appBorderDisplayList = 0;
	showFlightLines = false;

	resetWhazzupStatus();

	if(list.size() != 6)
		return;
	
	label = list[0];
	name = list[1];
	city = list[2];
	countryCode = list[3];
	lat = list[4].toDouble();
	lon = list[5].toDouble();
}

Airport::~Airport() {
	if(appDisplayList != 0) glDeleteLists(appDisplayList, 1);
	if(twrDisplayList != 0) glDeleteLists(twrDisplayList, 1);
	if(gndDisplayList != 0) glDeleteLists(gndDisplayList, 1);
	if(appBorderDisplayList != 0) glDeleteLists(appBorderDisplayList, 1);
}

void Airport::resetWhazzupStatus() {
	active = false;
	towers.clear();
	approaches.clear();
	grounds.clear();
	arrivals.clear();
	departures.clear();
}

void Airport::addArrival(Pilot* client) {
	if(!arrivals.contains(client)) {
		arrivals.append(client);
		active = true;
	}
}

void Airport::addDeparture(Pilot* client) {
	if(!departures.contains(client)) {
		departures.append(client);
		active = true;
	}
}

const GLuint& Airport::getAppBorderDisplayList() {
	if(appBorderDisplayList == 0)
		createAppDisplayLists();

	return appBorderDisplayList;
}

const GLuint& Airport::getAppDisplayList() {
	if(appDisplayList == 0)
		createAppDisplayLists();
	
	return appDisplayList;
}

void Airport::createAppDisplayLists() {
	// prepare points for the circle
	GLdouble circle_distort = cos(lat * Pi180);
	QList<QPair<double, double> > points;
	for(int i = 0; i <= 360; i += 10) {		
		double x = lat + Nm2Deg(40) * circle_distort * cos(i * Pi180);
		double y = lon + Nm2Deg(40) * sin(i * Pi180);
		points.append(QPair<double, double>(x, y));
	}
	
	// This draws an APP controller 'big filled circle' at x, y coordinates
	appDisplayList = glGenLists(1);
	glNewList(appDisplayList, GL_COMPILE);
	
	QColor colorMiddle = Settings::appCenterColor();
	QColor colorBorder = Settings::appMarginColor();
	QColor borderLine = Settings::appBorderLineColor();

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(colorMiddle.redF(), colorMiddle.greenF(), colorMiddle.blueF(), colorMiddle.alphaF());
	glVertex3f(SX(lat, lon), SY(lat, lon), SZ(lat, lon));
	glColor4f(colorBorder.redF(), colorBorder.greenF(), colorBorder.blueF(), colorBorder.alphaF());
	for(int i = 0; i < points.size(); i++) {
		VERTEX(points[i].first, points[i].second);
	}
	glEnd();
	glEndList();
	
	// border line
	appBorderDisplayList = glGenLists(1);
	glNewList(appBorderDisplayList, GL_COMPILE);	
	glLineWidth(Settings::appBorderLineStrength());
	glBegin(GL_LINE_LOOP);
	glColor4f(borderLine.redF(), borderLine.greenF(), borderLine.blueF(), borderLine.alphaF());
	for (int i = 0; i < points.size(); i++) {
		VERTEX(points[i].first, points[i].second);
	}
	glEnd();
	glEndList();
}

const GLuint& Airport::getTwrDisplayList() {
	if(twrDisplayList != 0)
		return twrDisplayList;

	QColor colorMiddle = Settings::twrCenterColor();
	QColor colorBorder = Settings::twrMarginColor();

	// This draws a TWR controller 'small filled circle' at x, y coordinates
	twrDisplayList = glGenLists(1);
	glNewList(twrDisplayList, GL_COMPILE);

	GLdouble circle_distort = cos(lat * Pi180);

	QList<QPair<double, double> > points;
	for(int i = 0; i <= 360; i += 20) {		
		double x = lat + Nm2Deg(22) * circle_distort * cos(i * Pi180);
		double y = lon + Nm2Deg(22) * sin(i * Pi180);
		points.append(QPair<double, double>(x, y));
	}
	
	glBegin(GL_TRIANGLE_FAN);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glColor4f(colorMiddle.redF(), colorMiddle.greenF(), colorMiddle.blueF(), colorMiddle.alphaF());
	glVertex3f(SX(lat, lon), SY(lat, lon), SZ(lat, lon));
	glColor4f(colorBorder.redF(), colorBorder.greenF(), colorBorder.blueF(), colorBorder.alphaF());
	for(int i = 0; i < points.size(); i++) {
		VERTEX(points[i].first, points[i].second);
	}
	glEnd();	
	glEndList();
	
	return twrDisplayList;
}

const GLuint& Airport::getGndDisplayList() {
	if(gndDisplayList != 0)
		return gndDisplayList;

	gndDisplayList = glGenLists(1);
	glNewList(gndDisplayList, GL_COMPILE);

	GLdouble circle_distort = cos(lat * Pi180);
	GLdouble s1 = circle_distort * 0.07;
	GLdouble s3 = circle_distort * 0.25;

	QColor color = Settings::gndFillColor();
	
    glBegin(GL_POLYGON);
    	glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        // first point is in center to avoid problems with the concave shape
    	VERTEX(lat, lon);

		// draw a star shape
		VERTEX(lat + s3, lon);
		VERTEX(lat + s1, lon + 0.07);
		VERTEX(lat,      lon + 0.25);
		VERTEX(lat - s1, lon + 0.07);
		VERTEX(lat - s3, lon);
		VERTEX(lat - s1, lon - 0.07);
		VERTEX(lat,      lon - 0.25);
		VERTEX(lat + s1, lon - 0.07);
		VERTEX(lat + s3, lon);
	glEnd();

	color = Settings::gndBorderLineColor();
	glBegin(GL_LINE_STRIP);
		// draw the border for the star
		glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
		glLineWidth(Settings::gndBorderLineStrength());
		VERTEX(lat + s3, lon);
		VERTEX(lat + s1, lon + 0.07);
		VERTEX(lat,      lon + 0.25);
		VERTEX(lat - s1, lon + 0.07);
		VERTEX(lat - s3, lon);
		VERTEX(lat - s1, lon - 0.07);
		VERTEX(lat,      lon - 0.25);
		VERTEX(lat + s1, lon - 0.07);
		VERTEX(lat + s3, lon);
    glEnd();
    glEndList();
	return gndDisplayList;
}

void Airport::addApproach(Controller* client) {
	if(!approaches.contains(client)) {
		approaches.append(client);
		active = true;
	}
}

void Airport::addTower(Controller* client) {
	if(!towers.contains(client)) {
		towers.append(client);
		active = true;
	}
}

void Airport::addGround(Controller* client) {
	if(!grounds.contains(client)) {
		grounds.append(client);
		active = true;
	}	
}


void Airport::showDetailsDialog() {
	AirportDetails *infoDialog = AirportDetails::getInstance();

	infoDialog->refresh(this);
	infoDialog->show();
	infoDialog->raise();
	infoDialog->activateWindow();
	infoDialog->setFocus();
}

QList<Controller*> Airport::getAllControllers() const {
	QList<Controller*> result;
	
	result += grounds;
	result += towers;
	result += approaches;
	
	return result;
}

bool Airport::matches(const QRegExp& regex) const {
	if(name.contains(regex)) return true;
	if(city.contains(regex)) return true;
	if(countryCode.contains(regex)) return true;
	return MapObject::matches(regex);
}

QString Airport::mapLabel() const {
	QString result = label + " ";

	if(arrivals.isEmpty()) result += "-/";
	else result += QString("%1/").arg(arrivals.size());
	
	if(departures.isEmpty()) result += "-";
	else result += QString("%1").arg(departures.size());

	return result;
}

void Airport::toggleFlightLines() {
	setDisplayFlightLines(!showFlightLines);
}

void Airport::setDisplayFlightLines(bool show) {
	showFlightLines = show;
	for(int i = 0; i < arrivals.size(); i++)
		arrivals[i]->displayLineToDest = show;
	for(int i = 0; i < departures.size(); i++)
		departures[i]->displayLineFromDep = show;
}

void Airport::refreshAfterUpdate() {
	if(showFlightLines)
		setDisplayFlightLines(true);
}

QString Airport::toolTip() const {
	return QString("%1 (%2, %3)").arg(label).arg(city).arg(countryCode);
}
