#include "AirportDetailsDeparturesModel.h"

#include "../Airport.h"

#include <QFont>

void AirportDetailsDeparturesModel::setClients(const QList<Pilot*>& pilots) {
    beginResetModel();
    this->_pilots = pilots;
    endResetModel();
}

QVariant AirportDetailsDeparturesModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QString("Callsign");
            case 1: return QString("Type");
            case 2: return QString("Name");
            case 3: return QString("Outbound To");
            case 4: return QString("T");
            case 5: return QString("Via");
            case 6: return QString("Dist");
            case 7: return QString("Delay");
            case 8: return QString("Status");
        }
    }
    return QVariant();
}

int AirportDetailsDeparturesModel::columnCount(const QModelIndex&) const {
    return 9;
}

int AirportDetailsDeparturesModel::rowCount(const QModelIndex&) const {
    return _pilots.count();
}

QVariant AirportDetailsDeparturesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= _pilots.size()) {
        return QVariant();
    }

    Pilot* p = _pilots[index.row()];

    if (role == Qt::FontRole) {
        QFont result;
        if (p->flightStatus() == Pilot::PREFILED) {
            result.setItalic(true);
        }
        if (p->isFriend()) {
            result.setBold(true);
        }
        return result;
    } else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case 6:
            case 7:
                return Qt::AlignRight;
            case 4:
            case 8:
                return Qt::AlignHCenter;
        }

        return Qt::AlignLeft;
    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return p->callsign;
            case 1:
                return p->aircraftType();
            case 2:
                return p->displayName();
            case 3:
                return p->destAirport() != 0? p->destAirport()->toolTip(): "";
            case 4:
                return p->planFlighttype;
            case 5:
                return p->waypoints().isEmpty()? "": p->waypoints().constFirst();
            case 6:
                if (p->flightStatus() == Pilot::PREFILED) {
                    return "ETD " + p->planDeptime.mid(0, p->planDeptime.length() - 2) + ":" + p->planDeptime.right(2);
                } else {
                    return QString("%1 NM").arg(p->distanceFromDeparture() < 3? 0: (int) p->distanceFromDeparture());
                }
            case 7:
                return p->delayString();
            case 8:
                return p->flightStatusShortString();
        }
    } else if (role == Qt::UserRole) { // used for sorting
        switch (index.column()) {
            case 6:
                return p->flightStatus() == Pilot::PREFILED? -1: p->distanceFromDeparture();
        }

        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

void AirportDetailsDeparturesModel::modelSelected(const QModelIndex& index) const {
    if (_pilots[index.row()]->hasPrimaryAction()) {
        _pilots[index.row()]->primaryAction();
    }
}
