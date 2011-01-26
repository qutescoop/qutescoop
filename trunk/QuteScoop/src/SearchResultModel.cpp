/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "SearchResultModel.h"
#include "Window.h"
#include <QFont>

int SearchResultModel::rowCount(const QModelIndex &parent) const {
	return content.count();
}

int SearchResultModel::columnCount(const QModelIndex &parent) const {
	return 1;
} 

QVariant SearchResultModel::data(const QModelIndex &index, int role) const {
	if(!index.isValid())
		return QVariant();
	
	if(index.row() >= content.size())
		return QVariant();
	
	if(role == Qt::DisplayRole) {
		MapObject* o = content[index.row()];
		switch(index.column()) {
            case 0: return o->toolTip(); break;
		}
	}

    if(role == Qt::FontRole) {
		QFont result;
        //prefiled italic
        if(dynamic_cast<Pilot*>(content[index.row()])) {
            Pilot *p = dynamic_cast<Pilot*>(content[index.row()]);
            if(p->flightStatus() == Pilot::PREFILED) {
				result.setItalic(true);
            }
        }
        
        //friends bold
		Client *c = dynamic_cast<Client*>(content[index.row()]);
		if(c != 0) {
			if(c->isFriend()) {
				result.setBold(true);
			}
		}
		return result;
	}
	
	return QVariant();
}

QVariant SearchResultModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Vertical)
		return QVariant();

	if (section != 0)
		return QVariant();

	if (content.isEmpty())
		return QString("No Results");
	if (content.size() == 1)
		return QString("1 Result");
	return QString("%1 Results").arg(content.size());
}

void SearchResultModel::modelDoubleClicked(const QModelIndex& index) { // double-click to center
    double lat = content[index.row()]->lat;
    double lon = content[index.row()]->lon;

    if (Window::getInstance(false) != 0)
        Window::getInstance(true)->showOnMap(lat, lon);
}

void SearchResultModel::modelClicked(const QModelIndex& index) { // one click to bring up the Details, as is the case on the map
    content[index.row()]->showDetailsDialog();
}