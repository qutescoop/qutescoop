/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "AirportDetailsDeparturesModel.h"

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
        case 4: return QString("T"); break;
        case 5: return QString("Via"); break;
        case 6: return QString("Alt"); break;
        case 7: return QString("GS"); break;
        case 8: return QString("Dist"); break;
        case 9: return QString("Delay"); break;
        case 10: return QString("Status"); break;
    }

    return QVariant();
}

int AirportDetailsDeparturesModel::rowCount(const QModelIndex &parent) const {
    return pilots.count();
}

int AirportDetailsDeparturesModel::columnCount(const QModelIndex &parent) const {
    return 11;
}

QVariant AirportDetailsDeparturesModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(index.row() >= pilots.size())
        return QVariant();

    Pilot* p = pilots[index.row()];

    if(role == Qt::FontRole) {
        QFont result;
        if (p->flightStatus() == Pilot::PREFILED) {
            result.setItalic(true);
            return result;
        }
        if (p->isFriend()) {
            result.setBold(true);
            return result;
        }
        return QFont();
    } else if(role == Qt::DisplayRole) {
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
                return p->planFlighttype; break;
            case 5:
                return p->waypoints().first(); break;
            case 6:
                return (p->altitude == 0? QString(""): QString("%1").arg(p->altitude)); break;
            case 7:
                return (p->groundspeed == 0? QString(""): QString("%1").arg(p->groundspeed)); break;
            case 8:
                if(p->flightStatus() == Pilot::PREFILED) return "ETD " + p->planDeptime.mid(0, p->planDeptime.length() - 2) + ":" + p->planDeptime.right(2);
                else return (p->distanceFromDeparture() < 3? 0: (int)p->distanceFromDeparture()); break;
            case 9:
                return p->delayStr();
                break;
            case 10:
                return p->flightStatusShortString();
                break;
        }
    }

    return QVariant();
}

void AirportDetailsDeparturesModel::modelSelected(const QModelIndex& index) {
    pilots[index.row()]->showDetailsDialog();
}
