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

#ifndef NAVAID_H_
#define NAVAID_H_

#include "Waypoint.h"

class NavAid: public Waypoint {
public:
	NavAid(const QStringList& stringList);

	enum Type {
		NDB = 2,
		VOR = 3,
		ILS_LOC = 4,
		LOC = 5,
		GS = 6,
		OM = 7,
		MM = 8,
		IM = 9,
		DME_NO_FREQ = 12,
		DME = 13
	};

private:
	Type type;
	int altitude;
	int frequency;
	int range;
	float heading;
	QString name;
};

#endif /* NAVAID_H_ */
