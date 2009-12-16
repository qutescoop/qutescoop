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

#include <QDebug>
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

void SearchVisitor::visit(MapObject* object)
{
	if(!object->matches(regex))
		return;

	Pilot *p = dynamic_cast<Pilot*>(object);
	if(p != 0) {
		pilots[p->label] = p;
		return;
	}

	Controller *c = dynamic_cast<Controller*>(object);
	if(c != 0) {
		if(c->isObserver()) {
			observers[c->label] = c;
		} else {
			controllers[c->label] = c;
		}
		return;
	}
	
	others[object->label] = object;
}

QList<MapObject*> SearchVisitor::result() {
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
