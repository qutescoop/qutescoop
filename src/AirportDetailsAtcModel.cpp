/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "AirportDetailsAtcModel.h"

void AirportDetailsAtcModel::setClients(const QList<Controller*>& controllers) {
    beginResetModel();
    this->_controllers = controllers;
    endResetModel();
}

QVariant AirportDetailsAtcModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0: return QString("Callsign");
        case 1: return QString("Freq");
        case 2: return QString("Facility");
        case 3: return QString("ATIS");
        case 4: return QString("Name");
        case 5: return QString("Rank");
        case 6: return QString("Online");
        case 7: return QString("Until");
        }
    }
    return QVariant();
}

int AirportDetailsAtcModel::rowCount(const QModelIndex&) const {
    return _controllers.count();
}

int AirportDetailsAtcModel::columnCount(const QModelIndex&) const {
    return 8;
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
    } else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case 6:
        case 7:
            return Qt::AlignRight;
        case 5:
            return Qt::AlignHCenter;
        }

        return Qt::AlignLeft;
    } else if (role == Qt::DisplayRole) {
        switch(index.column()) {
        case 0: return c->label; break;
        case 1: return c->frequency.toDouble() > 199 ? QVariant(): c->frequency; break; //sort out observers without prim freq
        case 2: return c->facilityString(); break;
        case 3: return c->atisCode; break;
        case 4: return c->realName(); break;
        case 5: return c->rank(); break;
        case 6: return c->onlineTime(); break;
        case 7: return c->assumeOnlineUntil.time().toString("HHmm'z'"); break;
        }
    }

    return QVariant();
}

void AirportDetailsAtcModel::modelSelected(const QModelIndex& index) const {
    _controllers[index.row()]->showDetailsDialog();
}
