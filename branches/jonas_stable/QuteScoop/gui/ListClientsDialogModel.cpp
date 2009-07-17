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

#include "ListClientsDialogModel.h"
#include "Whazzup.h"
#include "Window.h"
#include "NavData.h"

void ListClientsDialogModel::setClients(const QList<Client*>& clients) {
    this->clients = clients;
    reset();
}

QVariant ListClientsDialogModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Vertical)
        return QVariant();

    // orientation is Qt::Horizontal
    switch(section) {
        case 0: return QString("Callsign");
        case 1: return QString("Rating");
        case 2: return QString("Name");
        case 3: return QString("Online");
        case 4: return QString("Dist from Here");
        case 5: return QString("Server");
        case 6: return QString("Controller Info / Flight Status"); //if Controller/Pilot
        case 7: return QString("Admin Rating"); //IVAO
    }

    return QVariant();
}

QVariant ListClientsDialogModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(index.row() >= clients.size())
        return QVariant();

    Client* c = clients[index.row()];
    if(role == Qt::FontRole) {
        if (c->isFriend()) {
            QFont result;
            result.setBold(true);
            return result;
        }
        return QFont();
    } else if (role == Qt::DisplayRole) {
        Controller *co = dynamic_cast<Controller*>(c);
        Pilot *p = dynamic_cast<Pilot*>(c);
        switch(index.column()) {
            case 0: return c->label;
            case 1: return c->rank();
            case 2: return c->realName;
            case 3: return c->onlineTime();
            case 4:
                return (int) NavData::distance(
                                c->lat, c->lon,
                                Window::getInstance()->glWidget->currentPosition().first,
                                Window::getInstance()->glWidget->currentPosition().second)
                             ;
            case 5: return c->server;
            case 6: // Controller Info / Flight Status
                if(co != 0)
                    return co->atisMessage;
                if(p != 0)
                    return p->flightStatusShortString();
                return QString();
            case 7: return c->adminRating > 2? QString("%1").arg(c->adminRating): QString();
        }
    }

    return QVariant();
}

int ListClientsDialogModel::rowCount(const QModelIndex &parent) const {
    return clients.count();
}

int ListClientsDialogModel::columnCount(const QModelIndex &parent) const {
    return Whazzup::getInstance()->realWhazzupData().isIvao()? 8: 7;
}


void ListClientsDialogModel::modelSelected(const QModelIndex& index) {
    clients[index.row()]->showDetailsDialog();
}
