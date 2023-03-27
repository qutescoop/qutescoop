/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "AirportDetailsDeparturesModel.h"
#include "Airport.h"

#include <QFont>

void AirportDetailsDeparturesModel::setClients(const QList<Pilot*>& pilots) {
    beginResetModel();
    this->_pilots = pilots;
    endResetModel();
}

QVariant AirportDetailsDeparturesModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0: return QString("Callsign");
        case 1: return QString("Type");
        case 2: return QString("Name");
        case 3: return QString("Outbound To");
        case 4: return QString("T");
        case 5: return QString("Via");
        case 6: return QString("Alt");
        case 7: return QString("GS");
        case 8: return QString("Dist");
        case 9: return QString("Delay");
        case 10: return QString("Status");
        }
    }
    return QVariant();
}

int AirportDetailsDeparturesModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 11;
}

int AirportDetailsDeparturesModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _pilots.count();
}

QVariant AirportDetailsDeparturesModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || index.row() >= _pilots.size())
        return QVariant();

    Pilot* p = _pilots[index.row()];

    if(role == Qt::FontRole) {
        QFont result;
        if (p->flightStatus() == Pilot::PREFILED)
            result.setItalic(true);
        if (p->isFriend())
            result.setBold(true);
        return result;
    } else if(role == Qt::DisplayRole) {
        switch(index.column()) {
        case 0:
            return p->label;
        case 1:
            return p->aircraftType();
        case 2:
            return p->displayName();
        case 3:
            return p->destAirport() != 0? p->destAirport()->toolTip(): QString();
        case 4:
            return p->planFlighttype;
        case 5:
            return p->waypoints().isEmpty()? QString(): p->waypoints().constFirst();
        case 6:
            return (p->altitude == 0? QString(""): p->humanAlt());
        case 7:
            return (p->groundspeed == 0? QString(""): QString("%1").arg(p->groundspeed));
        case 8:
            if(p->flightStatus() == Pilot::PREFILED)
                return "ETD " + p->planDeptime.mid(0, p->planDeptime.length() - 2) + ":" + p->planDeptime.right(2);
            else
                return (p->distanceFromDeparture() < 3? 0: (int)p->distanceFromDeparture());
        case 9:
            return p->delayStr();
        case 10:
            return p->flightStatusShortString();
        }
    }

    return QVariant();
}

void AirportDetailsDeparturesModel::modelSelected(const QModelIndex& index) const {
    _pilots[index.row()]->showDetailsDialog();
}
