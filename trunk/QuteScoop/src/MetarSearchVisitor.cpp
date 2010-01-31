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

#include "MetarSearchVisitor.h"

void MetarSearchVisitor::visit(MapObject *object) {
	Airport *a = dynamic_cast<Airport*>(object);
	if(a == 0) return;
	if(a->label.contains(regex))
		airportMap[a->label] = a;
}

QList<Airport*> MetarSearchVisitor::airports() {
	QList<Airport*> res;
	
	QList<QString> labels = airportMap.keys();
	qSort(labels);	
	for(int i = 0; i < labels.size(); i++)
		res.append(airportMap[labels[i]]);
	
	return res;
}

QList<MapObject*> MetarSearchVisitor::result() {
	QList<MapObject*> res;
	QList<Airport*> airpts = airports();
	for(int i = 0; i < airpts.size(); i++) res.append(airpts[i]);
	return res;
}
