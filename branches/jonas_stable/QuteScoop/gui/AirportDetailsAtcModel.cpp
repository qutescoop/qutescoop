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

#include "AirportDetailsAtcModel.h"

void AirportDetailsAtcModel::setClients(const QList<Controller*>& controllers) {
    this->controllers = controllers;
    reset();
}

QVariant AirportDetailsAtcModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Vertical)
        return QVariant();

    // orientation is Qt::Horizontal
    switch(section) {
    case 0: return QString("Callsign"); break;
    case 1: return QString("Freq"); break;
    case 2: return QString("Facility"); break;
    case 3: return QString("Name"); break;
    case 4: return QString("Rank"); break;
    case 5: return QString("Online"); break;
    case 6: return QString("Online Until"); break;
    case 7: return QString("Server"); break;
    }

    return QVariant();
}

QVariant AirportDetailsAtcModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(index.row() >= controllers.size())
        return QVariant();

    if(role == Qt::DisplayRole) {
        Controller* c = controllers[index.row()];
        switch(index.column()) {
        case 0: return c->label; break;
        case 1: return c->frequency.toDouble() > 199 ? QVariant(): c->frequency; break; //sort out observers without prim freq
        case 2: return c->facilityString(); break;
        case 3: return c->realName; break;
        case 4: return c->rank(); break;
        case 5: return c->onlineTime(); break;
        case 6: return c->assumeOnlineUntil.time().toString("hhmmZ"); break;
        case 7: return c->server; break;
        }
    }

    return QVariant();
}

void AirportDetailsAtcModel::modelSelected(const QModelIndex& index) {
    controllers[index.row()]->showDetailsDialog();
}
