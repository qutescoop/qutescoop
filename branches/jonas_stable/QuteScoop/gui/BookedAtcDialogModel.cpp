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

#include "BookedAtcDialogModel.h"

void BookedAtcDialogModel::setClients(const QList<BookedController*>& controllers) {
	this->controllers = controllers;
	reset();
}

QVariant BookedAtcDialogModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Vertical)
    	return QVariant();
    
	// orientation is Qt::Horizontal
	switch(section) {
        case 0: return QString("Callsign"); break;
        case 1: return QString("Position"); break;
        case 2: return QString("Country"); break;
        case 3: return QString("Name"); break;
        case 4: return QString("Date"); break;
        case 5: return QString("From [UTC]"); break;
        case 6: return QString("To [UTC]"); break;
        case 7: return QString("Info"); break;
	}
	
	return QVariant();
}

QVariant BookedAtcDialogModel::data(const QModelIndex &index, int role) const {
	if(!index.isValid())
		return QVariant();
	
	if(index.row() >= controllers.size())
		return QVariant();
	
    if(role == Qt::DisplayRole) {
        BookedController* c = controllers[index.row()];
        switch(index.column()) {
            case 0: return c->label; break;
            case 1: return c->facilityString(); break;
            case 2: return c->countryCode; break;
            case 3: return c->realName; break;
            case 4: return c->starts().toString("MM/dd (ddd)"); break;
            case 5: return c->starts().time(); break;
            case 6: return c->ends().time(); break;
            case 7: return c->bookingInfoStr; break;
        }
    } else if(role == Qt::EditRole) {
        BookedController* c = controllers[index.row()];
        switch(index.column()) {
            case 5: return c->starts(); break;
            case 6: return c->ends(); break;
        }
    }
	
	return QVariant();
}

int BookedAtcDialogModel::rowCount(const QModelIndex &parent) const { 
    return controllers.count(); 
}

int BookedAtcDialogModel::columnCount(const QModelIndex &parent) const { 
    return 8; 
} 


void BookedAtcDialogModel::modelSelected(const QModelIndex& index) {
    controllers[index.row()]->showDetailsDialog();
}
