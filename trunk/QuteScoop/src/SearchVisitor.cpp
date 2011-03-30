/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "SearchVisitor.h"

SearchVisitor::SearchVisitor(const QString& searchStr) {
	QStringList tokens = searchStr.trimmed().replace(QRegExp("\\*"), ".*").split(QRegExp("[ \\,]+"), QString::SkipEmptyParts);
	if(tokens.size() == 1) {
		regex = QRegExp("^" + tokens.first() + ".*", Qt::CaseInsensitive);
		return;
	}
	
	QString regExpStr = "^(" + tokens.first();
	for(int i = 1; i < tokens.size(); i++)
		regExpStr += "|" + tokens[i];
	regExpStr += ".*)";
	regex = QRegExp(regExpStr, Qt::CaseInsensitive);
}

void SearchVisitor::visit(MapObject* object) {
	if(!object->matches(regex))
		return;

	Pilot *p = dynamic_cast<Pilot*>(object);
	if(p != 0) {
		pilots[p->label] = p;
		return;
	}

	Controller *c = dynamic_cast<Controller*>(object);
	if(c != 0) {
		if(c->isObserver())
			observers[c->label] = c;
		else
			controllers[c->label] = c;
		return;
	}
	
	others[object->label] = object;
}

QList<MapObject*> SearchVisitor::result() const {
	QList<MapObject*> result;

	// airports
	QList<QString> labels = others.keys();
	qSort(labels);
	for(int i = 0; i < labels.size(); i++)
		result.append(others[labels[i]]);
	
	// controllers
	labels = controllers.keys();
	qSort(labels);
	for(int i = 0; i < labels.size(); i++)
		result.append(controllers[labels[i]]);
	
	// pilots
	labels = pilots.keys();
	qSort(labels);
	for(int i = 0; i < labels.size(); i++)
		result.append(pilots[labels[i]]);
	
	// observers
	labels = observers.keys();
	qSort(labels);
	for(int i = 0; i < labels.size(); i++)
		result.append(observers[labels[i]]);
	
	return result;
}