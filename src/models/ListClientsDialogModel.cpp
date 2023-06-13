#include "ListClientsDialogModel.h"

#include "../Airport.h"
#include "../NavData.h"

void ListClientsDialogModel::setClients(const QList<Client*>& clients) {
    qDebug();
    beginResetModel();
    _clients = clients;
    endResetModel();
    qDebug() << "-- finished";
}

QVariant ListClientsDialogModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    // orientation is Qt::Horizontal
    switch (section) {
        case 0: return "Callsign";
        case 1: return "Rating";
        case 2: return "Name";
        case 3: return "Online";
        case 4: return "Livestream";
        case 5: return "ATC:\nvisibility\nrange [NM]";
        case 6: return "Pilot:\nTTG";
        case 7: return "Pilot:\nroute DEP";
        case 8: return "Pilot:\nroute DEST";
        case 9: return "Pilot:\naircraft";
        case 10: return "Pilot:\nflight type";
        case 11: return "Pilot:\nroute dist [NM]";
        case 12: return "Pilot:\naircraft (FP)";
        case 13: return "Pilot:\nFP remarks";
        case 14: return "ATC:\ncontroller\ninfo";
    }
    return QVariant();
}

QVariant ListClientsDialogModel::data(const QModelIndex &index, int role) const {
    // unnecessary due to the c == 0 check below
    //if(!index.isValid())
    //    return QVariant();

    Client* c = _clients.value(index.row(), 0);
    if (c == 0) {
        return QVariant();
    }

    if (role == Qt::FontRole) {
        if (c->isFriend()) {
            QFont result;
            result.setBold(true);
            return result;
        }
        return QFont();
    } else if (role == Qt::DisplayRole) {
        Controller* co = dynamic_cast <Controller*> (c);
        Pilot* p = dynamic_cast <Pilot*> (c);
        switch (index.column()) {
            case 0: return c->callsign;
            case 1: return c->rank();
            case 2: return c->realName();
            case 3: return c->onlineTime();
            case 4: return c->livestreamString();
            case 5: return co != 0? QString("%1").arg(co->visualRange, 4, 'f', 0, ' '): "";
            case 6:
                if (co != 0) {
                    if (co->assumeOnlineUntil.isValid()) {
                        return QString("%1 hrs").arg(
                            QTime().addSecs(QDateTime::currentDateTimeUtc().secsTo(co->assumeOnlineUntil)).toString("H:mm")
                        );
                    }
                    return "";
                }
                if (p != 0) {
                    return QString("%1 hrs").arg(p->eet().toString("H:mm"));
                }
                break;
            case 7:
                if (p != 0 && p->depAirport() != 0) {
                    return p->depAirport()->id;
                }
                return "";
            case 8:
                if (p != 0 && p->destAirport() != 0) {
                    return p->destAirport()->id;
                }
                return "";
            case 9: return p != 0? p->aircraftType(): "";
            case 10: return p != 0? p->planFlighttypeString(): "";
            case 11:
                if (p != 0 && p->depAirport() != 0 && p->destAirport() != 0) {
                    return QString("%1").arg(
                        NavData::distance(
                            p->depAirport()->lat, p->depAirport()->lon,
                            p->destAirport()->lat, p->destAirport()->lon
                        ),
                        5, 'f', 0, ' '
                    );
                }
                return "";
            case 12: return p != 0? p->planAircraft: "";
            case 13: return p != 0? p->planRemarks: "";
            case 14: return co != 0? QString(co->atisMessage).replace("\n", " – "): "";
        }
    }

    return QVariant();
}

int ListClientsDialogModel::rowCount(const QModelIndex&) const {
    return _clients.count();
}

int ListClientsDialogModel::columnCount(const QModelIndex&) const {
    return 15;
}

void ListClientsDialogModel::modelSelected(const QModelIndex& index) {
    if (_clients[index.row()] != 0) {
        MapObject* m = dynamic_cast<MapObject*>(_clients[index.row()]);
        if (m != 0 && m->hasPrimaryAction()) {
            m->primaryAction();
        }
    }
}
