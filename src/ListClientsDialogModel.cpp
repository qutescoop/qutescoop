/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ListClientsDialogModel.h"

#include "Whazzup.h"
#include "NavData.h"

void ListClientsDialogModel::setClients(const QList<Client*>& clients) {
    qDebug() << "ListClientsDialogModel::setClients()";
    beginResetModel();
    this->_clients = clients;
    endResetModel();
    qDebug() << "ListClientsDialogModel::setClients() -- finished";
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
        case 4: return QString("ATC:\nvisibility\nrange [NM]");
        case 5: return QString("Pilot:\nTTG");
        case 6: return QString("Pilot:\nroute DEP");
        case 7: return QString("Pilot:\nroute DEST");
        case 8: return QString("Pilot:\naircraft");
        case 9: return QString("Pilot:\nflight type");
        case 10: return QString("Pilot:\nroute dist [NM]");
        case 11: return QString("Pilot:\naircraft (FP)");
        case 12: return QString("Pilot:\nFP remarks");
    }
    return QVariant();
}

QVariant ListClientsDialogModel::data(const QModelIndex &index, int role) const {
    // unnecessary due to the c == 0 check below
    //if(!index.isValid())
    //    return QVariant();

    Client* c = _clients.value(index.row(), 0);
    if (c == 0) return QVariant();

    if(role == Qt::FontRole) {
        if (c->isFriend()) {
            QFont result;
            result.setBold(true);
            return result;
        }
        return QFont();
    } else if (role == Qt::DisplayRole) {
        Controller *co = dynamic_cast <Controller*> (c);
        Pilot *p = dynamic_cast <Pilot*> (c);
        switch(index.column()) {
            case 0: return c->label;
            case 1: return c->rank();
            case 2: return c->realName();
            case 3: return c->onlineTime();
            case 4: return co != 0? QString("%1").arg(co->visualRange, 4, 'f', 0, ' '): QString();
            case 5:
                if (co != 0) {
                    if (co->assumeOnlineUntil.isValid()) {
                            return QString("%1 hrs").arg(
                                QTime().addSecs(QDateTime::currentDateTimeUtc().secsTo(co->assumeOnlineUntil)).toString("H:mm")
                            );
                        }
                    return QString();
                }
                if (p != 0)
                    return QString("%1 hrs").arg(p->eet().toString("H:mm"));
                break;
            case 6:
                if (p != 0 && p->depAirport() != 0)
                    return p->depAirport()->label;
                return QString();
            case 7:
                if (p != 0 && p->destAirport() != 0)
                    return p->destAirport()->label;
                return QString();
            case 8: return p != 0? p->aircraftType(): QString();
            case 9: return p != 0? p->planFlighttypeString(): QString();
            case 10:
                if (p != 0 && p->depAirport() != 0 && p->destAirport() != 0)
                    return QString("%1").arg(
                            NavData::distance(p->depAirport()->lat, p->depAirport()->lon,
                                              p->destAirport()->lat, p->destAirport()->lon),
                            5, 'f', 0, ' '
                    );
                return QString();
            case 11: return p != 0? p->planAircraft: QString();
            case 12: return p != 0? p->planRemarks: QString();
        }
    }

    return QVariant();
}

int ListClientsDialogModel::rowCount(const QModelIndex&) const {
    return _clients.count();
}

int ListClientsDialogModel::columnCount(const QModelIndex&) const {
    return 13;
}

void ListClientsDialogModel::modelSelected(const QModelIndex& index) {
    if (_clients[index.row()] != 0)
        _clients[index.row()]->showDetailsDialog();
}
