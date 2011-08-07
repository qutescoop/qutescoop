/**************************************************************************
 *  This file is part of QuteScoop. See README for license
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
