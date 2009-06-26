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

#ifndef NAVDATA_H_
#define NAVDATA_H_

#include <QHash>
#include "Airport.h"
#include "Fir.h"
#include "Airac.h"

class NavData
{
public:
	static NavData* getInstance();

	const QHash<QString, Airport*>& airports() const;
	const QList<Airport*>& airportsTrafficSorted() const;
	const QHash<QString, Fir*>& firs() const;
	QList<Airport*> airportsAt(double lat, double lon, double maxDist);

	const Airac& getAirac() const { return airac; };

	static double distance(double lat1, double lon1, double lat2, double lon2);
	static void distanceTo(double lat, double lon, double dist, double heading, double *latTo, double *lonTo);
	static double courseTo(double lat1, double lon1, double lat2, double lon2);
	static void greatCirclePlotTo(double lat1, double lon1,
									double lat2, double lon2,
									double fraction,
									double *lat, double *lon);

	QString countryName(const QString& countryCode) const { return countryCodes[countryCode]; }

	void updateData(const WhazzupData& whazzupData);
	void accept(MapObjectVisitor* visitor);

private:
	NavData();

	void loadDatabase(const QString& directory);
	void loadAirports(const QString& filename);
	void loadFirs();
	void loadCountryCodes(const QString& filename);

	QHash<QString, Airport*> airportMap;
	QList<Airport*> airportsListTrafficSorted; // holds airports sorted by congestion descending
	QHash<QString, Fir*> firMap;
	QHash<QString, QString> countryCodes;

	Airac airac;
};

#endif /*NAVDATA_H_*/
