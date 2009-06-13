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

#include "AirportDetailsDeparturesModel.h"

#include <QDebug>

void AirportDetailsDeparturesModel::setClients(const QList<Pilot*>& pilots) {
	this->pilots = pilots;
	reset();
}

QVariant AirportDetailsDeparturesModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole)
		return QVariant();

	if(orientation == Qt::Vertical)
		return QVariant();
    
	// orientation is Qt::Horizontal
	switch(section) {
        case 0: return QString("Callsign"); break;
        case 1: return QString("Type"); break;
        case 2: return QString("Name"); break;
        case 3: return QString("Outbound To"); break;
        case 4: return QString("Via"); break;
        case 5: return QString("Alt"); break;
        case 6: return QString("Speed"); break;
        case 7: return QString("Dist"); break;
        case 8: return QString("Status"); break;
    }
	
	return QVariant();
}

int AirportDetailsDeparturesModel::rowCount(const QModelIndex &parent) const {
	return pilots.count();
}

int AirportDetailsDeparturesModel::columnCount(const QModelIndex &parent) const {
	return 9;
}

QVariant AirportDetailsDeparturesModel::data(const QModelIndex &index, int role) const {
	if(!index.isValid())
		return QVariant();
	
	if(index.row() >= pilots.size())
		return QVariant();
	
	if(role == Qt::DisplayRole) {
		Pilot* p = pilots[index.row()];
		switch(index.column()) {
            case 0: 
                return p->label; break;
            case 1: 
                return p->aircraftType(); break;
            case 2: 
                return p->displayName(); break;
            case 3: 
                if(p->destAirport() != 0) return p->destAirport()->toolTip(); break;
            case 4: 
                return p->waypoints().first(); break;
            case 5:
                if(p->flightStatus() == Pilot::PREFILED) return "n/a";
                else return p->altitude; break;
            case 6:
                if(p->flightStatus() == Pilot::PREFILED) return "n/a";
                else return p->groundspeed; break;
            case 7:
                if(p->flightStatus() == Pilot::PREFILED) return "ETD " + p->planDeptime.mid(0, p->planDeptime.length() - 2) + ":" + p->planDeptime.right(2);
                else return (int)p->distanceFromDeparture(); break;
            case 8: 
                return p->flightStatusString().split("(")[0]; // we do only want a short string, not the details
                break;
        }
    }
	
	return QVariant();
}

void AirportDetailsDeparturesModel::modelSelected(const QModelIndex& index) {
	pilots[index.row()]->showDetailsDialog();
}
