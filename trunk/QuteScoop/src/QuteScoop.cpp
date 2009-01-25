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
#include "helpers.h"
#include "Window.h"

#define RUN_TEST

#ifdef RUN_TEST
#include <QDebug>
#include "Settings.h"
#include "Airac.h"
#include "NavData.h"

int runTest() {
	int i;
	NavData *navData = NavData::getInstance();
	const Airac airac = navData->getAirac();

	QString dep = "EDDN";
	QString dst = "LOWS";
	QString plan = "AKANU UL603 MUN DCT SBG";
	//QString plan = "AKANU TEGBA RENLO LUPOX TITIG MUN SBG";

	Airport* apDep = navData->airports()[dep];
	Airport* apDst = navData->airports()[dst];
	if(apDep == 0) qDebug() << "dep airport not found:" << dep;
	if(apDst == 0) qDebug() << "dst airport not found:" << dst;

	QList<Waypoint*> points = airac.resolveFlightplan(plan.split(' '), apDep->lat, apDep->lon);

	qDebug() << "-------------------------------------------";
	qDebug() << "Plan: " << dep << plan << dst;
	qDebug() << "resolved to" << points.size() << "waypoints";
	qDebug() << "-------------------------------------------";
	qDebug() << "Departure:" << apDep->name;
	float lat = apDep->lat;
	float lon = apDep->lon;
	for(i = 0; i < points.size(); i++) {
		double d = NavData::distance(lat, lon, points[i]->lat, points[i]->lon);
		double h = NavData::courseTo(lat, lon, points[i]->lat, points[i]->lon);
		qDebug() << "HDG" << h << "DST" << d << "nm to" << points[i]->id;
		lat = points[i]->lat;
		lon = points[i]->lon;
	}
	double d = NavData::distance(lat, lon, apDst->lat, apDst->lon);
	double h = NavData::courseTo(lat, lon, apDst->lat, apDst->lon);
	qDebug() << "HDG" << h << "DST" << d << "nm to" << apDst->name;
	qDebug() << "-------------------------------------------";

	return 0;
}
#endif

int main(int argc, char *argv[]) {

	QCoreApplication::setOrganizationName("QuteScoop");
	QCoreApplication::setOrganizationDomain("qutescoop.org");
	QCoreApplication::setApplicationName("QuteScoop");

	QApplication app(argc, argv);

#ifndef RUN_TEST
	Window *window = Window::getInstance();
	window->show();
	return app.exec();
#else
	return runTest();
#endif
}

QString lat2str(double lat) {
	QString result = "N";
	if (lat < 0) {
		result += "S";
		lat *= -1;
	}

	int lat1 = (int)lat;
	double lat2 = (lat - (int)lat) * 60.0;
	result += QString("%1 %2'")
	.arg(lat1, 2, 10, QChar('0'))
	.arg(lat2, 2, 'f', 3, QChar('0'));

	return result;
}

QString lon2str(double lon) {
	QString result = "E";
	if (lon < 0) {
		lon *= -1;
		result = "W";
	}

	int lon1 = (int)lon;
	double lon2 = (lon - (int)lon) * 60.0;
	result += QString("%1 %2'")
	.arg(lon1, 3, 10, QChar('0'))
	.arg(lon2, 2, 'f', 3, QChar('0'));

	return result;
}
