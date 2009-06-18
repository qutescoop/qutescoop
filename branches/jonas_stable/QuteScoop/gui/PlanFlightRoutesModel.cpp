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

#include "PlanFlightRoutesModel.h"

void PlanFlightRoutesModel::setClients(const QList<Route*>& newroutes) {
    routes.clear(); 
	routes.append(newroutes); //fixme - dunno know if this is good style
	reset();
}

QVariant PlanFlightRoutesModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Vertical)
    	return QVariant();
    
	// orientation is Qt::Horizontal
	switch(section) {
	case 0: return QString("Provider"); break;
	case 1: return QString("Route"); break;
	case 2: return QString("Dist"); break;
	case 3: return QString("FL>"); break;
	case 4: return QString("FL<"); break;
	case 5: return QString("Remarks"); break;
	case 6: return QString("Last changed"); break;
	}
	
	return QVariant();
}

QVariant PlanFlightRoutesModel::data(const QModelIndex &index, int role) const {
	if(!index.isValid())
		return QVariant();
	
	if(index.row() >= routes.size())
		return QVariant();
	
	if(role == Qt::DisplayRole) {
		Route* r = routes[index.row()];
		switch(index.column()) {
		case 0: return r->provider; break;
		case 1: return r->flightPlan; break;
		case 2: return r->routeDistance; break;
		case 3: return r->minFl; break;
		case 4: return r->maxFl; break;
		case 5: return r->comments; break;
		case 6: return r->lastChange; break;
		}
	}
	
	return QVariant();
}

void PlanFlightRoutesModel::modelSelected(const QModelIndex& index) {
	//routes[index.row()]->showDetailsDialog();
}
