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
		Client *c = dynamic_cast<Client*>(content[index.row()]);
		if(c != 0) {
			if(c->isFriend()) {
				QFont result;
				result.setBold(true);
				return result;
			}
		}
		return QFont();
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

void SearchResultModel::modelDoubleClicked(const QModelIndex& index) {
	content[index.row()]->showDetailsDialog();
}

void SearchResultModel::modelClicked(const QModelIndex& index) {
	double lat = content[index.row()]->lat;
	double lon = content[index.row()]->lon;
	
	Window::getInstance()->showOnMap(lat, lon);
}
