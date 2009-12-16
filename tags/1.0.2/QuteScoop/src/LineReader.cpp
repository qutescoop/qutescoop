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
#include "LineReader.h"

#include <QStringList>

const QList<QPair<double, double> >& LineReader::readLine() {
	currentLine.clear();

	if(atEnd()) return currentLine;

	do {
		QString line = nextLine();
		if(line == "end" || line.isNull()) {
			break;
		}
		
		QStringList list = line.split(':');
		if(list.size() != 2) continue;
		
		bool ok = true;
		double lat = list[0].toDouble(&ok);
		if(!ok) continue;
		double lon = list[1].toDouble(&ok);
		if(!ok) continue;
	
		currentLine.append(QPair<double, double>(lat, lon));
	} while(!atEnd());
	
	return currentLine;
}
