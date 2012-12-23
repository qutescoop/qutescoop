/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "SearchVisitor.h"

SearchVisitor::SearchVisitor(const QString& searchStr) {
        _searchString = searchStr;
	QStringList tokens = searchStr.trimmed().replace(QRegExp("\\*"), ".*").split(QRegExp("[ \\,]+"), QString::SkipEmptyParts);
	if(tokens.size() == 1) {
		_regex = QRegExp("^" + tokens.first() + ".*", Qt::CaseInsensitive);
		return;
	}
	
	QString regExpStr = "^(" + tokens.first();
	for(int i = 1; i < tokens.size(); i++)
		regExpStr += "|" + tokens[i];
	regExpStr += ".*)";
	_regex = QRegExp(regExpStr, Qt::CaseInsensitive);
}

void SearchVisitor::visit(MapObject* object) {
	if(!object->matches(_regex))
		return;

	Pilot *p = dynamic_cast<Pilot*>(object);
	if(p != 0) {
		_pilots[p->label] = p;
                return;
	}

	Controller *c = dynamic_cast<Controller*>(object);
	if(c != 0) {
		if(c->isObserver())
			_observers[c->label] = c;
		else
			_controllers[c->label] = c;
		return;
	}
	
	_others[object->label] = object;
}

void SearchVisitor::checkAirlines()
{ 
    QHashIterator<QString, QString> i(AirlineCodes);
     while (i.hasNext()) {
         i.next();
         if (i.key().contains(_regex)) {
             _otherStrings[i.key()] = i.value();
         }

         if(i.value().contains(_regex)) {
             _otherStrings[i.key()] = i.value();
         }

     }
}

QList<MapObject*> SearchVisitor::result() const {
	QList<MapObject*> result;

        //airlines
        QList<QString> labels = _otherStrings.keys();
        qSort(labels);
        for(int i = 0; i < labels.size(); i++) {
            MapObject *object = new MapObject();
            object->label = labels[i];
            object->label.append("  ");
            object->label.append(_otherStrings[labels[i]]);
            result.append(object);
        }

	// airports
        labels = _others.keys();
	qSort(labels);
	for(int i = 0; i < labels.size(); i++)
                result.append(_others[labels[i]]);
	
	// controllers
	labels = _controllers.keys();
	qSort(labels);
	for(int i = 0; i < labels.size(); i++)
		result.append(_controllers[labels[i]]);
	
	// pilots
	labels = _pilots.keys();
	qSort(labels);
	for(int i = 0; i < labels.size(); i++)
		result.append(_pilots[labels[i]]);
	
	// observers
	labels = _observers.keys();
	qSort(labels);
	for(int i = 0; i < labels.size(); i++)
		result.append(_observers[labels[i]]);
	
	return result;
}
