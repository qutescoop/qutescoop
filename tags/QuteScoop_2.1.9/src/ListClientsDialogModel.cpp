/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ListClientsDialogModel.h"

#include "Whazzup.h"
#include "NavData.h"

void ListClientsDialogModel::setClients(const QList<Client*>& clients) {
    qDebug() << "ListClientsDialogModel::setClients()";
    beginResetModel();
    this->clients = clients;
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
        case 4: return QString("TTG");
        case 5: return QString("Admin Rating"); //IVAO only
    }

    return QVariant();
}

QVariant ListClientsDialogModel::data(const QModelIndex &index, int role) const {
    // unnecessary due to the c == 0 check below
    //if(!index.isValid())
    //    return QVariant();

    Client* c = clients.value(index.row(), 0);
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
            case 2: return c->realName;
            case 3: return c->onlineTime();
            case 4:
                if (co != 0) {
                    if (co->assumeOnlineUntil.isValid())
                        return QTime().addSecs(
                           QDateTime::currentDateTimeUtc().secsTo(co->assumeOnlineUntil)
                        );
                    return QString();
                }
                if (p != 0)
                    return p->eet();
                break;
            case 5: return c->adminRating > 2? QString("%1").arg(c->adminRating): QString();
        }
    }

    return QVariant();
}

int ListClientsDialogModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return clients.count();
}

int ListClientsDialogModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return Whazzup::getInstance()->realWhazzupData().isIvao()? 6: 5;
}

void ListClientsDialogModel::modelSelected(const QModelIndex& index) {
    if (clients[index.row()] != 0)
        clients[index.row()]->showDetailsDialog();
}
