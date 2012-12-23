/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "AirportDetailsAtcModel.h"

void AirportDetailsAtcModel::setClients(const QList<Controller*>& controllers) {
    this->_controllers = controllers;
    reset();
}

QVariant AirportDetailsAtcModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0: return QString("Callsign");
        case 1: return QString("Freq");
        case 2: return QString("Facility");
        case 3: return QString("Name");
        case 4: return QString("Rank");
        case 5: return QString("Online");
        case 6: return QString("Until");
        case 7: return QString("Server");
        }
    }
    return QVariant();
}

QVariant AirportDetailsAtcModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || index.row() >= _controllers.size())
        return QVariant();
    Controller* c = _controllers[index.row()];

    if(role == Qt::FontRole) {
        if (c->isFriend()) {
            QFont result;
            result.setBold(true);
            return result;
        }
        return QFont();
    } else if (role == Qt::DisplayRole) {
        switch(index.column()) {
        case 0: return c->label; break;
        case 1: return c->frequency.toDouble() > 199 ? QVariant(): c->frequency; break; //sort out observers without prim freq
        case 2: return c->facilityString(); break;
        case 3: return c->realName; break;
        case 4: return c->rank(); break;
        case 5: return c->onlineTime(); break;
        case 6: return c->assumeOnlineUntil.time().toString("HHmm'z'"); break;
        case 7: return c->server; break;
        }
    }

    return QVariant();
}

void AirportDetailsAtcModel::modelSelected(const QModelIndex& index) const {
    _controllers[index.row()]->showDetailsDialog();
}
