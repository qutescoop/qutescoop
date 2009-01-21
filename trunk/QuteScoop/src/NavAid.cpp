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

#include "NavAid.h"

NavAid::NavAid(const QStringList& stringList) {
	if(stringList.size() < 9)
		return;

	bool ok;

	type = stringList[0].toInt(&ok);
	if(!ok) return;
	lat = stringList[1].toDouble(&ok);
	if(!ok) return;
	lon = stringList[2].toDouble(&ok);
	if(!ok) return;
	altitude = stringList[3].toInt(&ok);
	if(!ok) return;
	frequency = stringList[4].toInt(&ok);
	if(!ok) return;
	name = stringList[7];

}

