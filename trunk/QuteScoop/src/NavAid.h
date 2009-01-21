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

#define NAV_Unknown			0x0000
#define NAV_Airport			0x0001
#define NAV_NDB				0x0002
#define NAV_VOR				0x0004
#define NAV_ILS				0x0008
#define NAV_Localizer		0x0010
#define NAV_GlideSlope		0x0020
#define NAV_OuterMarker		0x0040
#define NAV_MiddleMarker	0x0080
#define NAV_InnerMarker		0x0100
#define NAV_Fix				0x0200
#define NAV_DME				0x0400
#define NAV_LatLon			0x0800


class NavAid: public Waypoint {
public:
	NavAid(const QStringList& stringList);

private:
	int type;
	int altitude;
	int frequency;

};

#endif /* NAVAID_H_ */
