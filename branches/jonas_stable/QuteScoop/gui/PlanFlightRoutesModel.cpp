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
#include "PlanFlightDialog.h"
#include <QtCore>
#include <QIcon>
#include <QMessageBox>

void PlanFlightRoutesModel::setClients(const QList<Route*>& newroutes) {
    routes = newroutes; 
	reset();
}

QVariant PlanFlightRoutesModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Vertical)
    	return QVariant();
    
	// orientation is Qt::Horizontal
	switch(section) {
    case 0: return QString(); break;
    case 1: return QString("Provider"); break;
    case 2: return QString("Dep"); break;
    case 3: return QString("Dest"); break;
    case 4: return QString("Route"); break;
    case 5: return QString("Dist"); break;
    case 6: return QString("FL>"); break;
    case 7: return QString("FL<"); break;
    case 8: return QString("Remarks"); break;
    case 9: return QString("Last changed"); break;
	}
	
	return QVariant();
}

QVariant PlanFlightRoutesModel::data(const QModelIndex &index, int role) const {
	if(!index.isValid())
		return QVariant();
	
	if(index.row() >= routes.size())
		return QVariant();
    
    if(role == Qt::DecorationRole) {
		Route* r = routes[index.row()];
		switch(index.column()) {
        case 0:
            if (r->provider == "generated")
                return QIcon(":/icons/qutescoop.png");
            else if (r->provider == "vroute")
                return QIcon(":/routeproviders/images/vroute.png");
            break;
        }
    } else if(role == Qt::DisplayRole) {
		Route* r = routes[index.row()];
		switch(index.column()) {
        case 1: return r->provider; break;
        case 2: return r->dep; break;
        case 3: return r->dest; break;
        case 4: return QString("%1").arg(r->flightPlan); break; //.left(10)).arg(r->flightPlan.right(10)); break;
        case 5: return QString("%1 NM").arg(r->routeDistance); break;
        case 6: return r->minFl; break;
        case 7: return r->maxFl; break;
        case 8: return r->comments; break;
        case 9:
            QDateTime lastChange = QDateTime::fromString(r->lastChange, "yyyyMMddhhmmss");
            if (lastChange.isValid()) return lastChange.date();
            else return r->lastChange;
            break;
		}
	}
	
	return QVariant();
}

void PlanFlightRoutesModel::modelSelected(const QModelIndex& index) {
    //routes[index.row()]->showDetailsDialog();
}

bool PlanFlightRoutesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    //qDebug() << "setData" << role << value;
}

Qt::ItemFlags PlanFlightRoutesModel::flags(const QModelIndex &index) const {
    // make column 4 edittable
    //if (index.column() == 4)
    //    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
