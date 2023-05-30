#include "AirportDetailsArrivalsModel.h"

#include "Airport.h"

#include <QFont>

void AirportDetailsArrivalsModel::setClients(const QList<Pilot*>& pilots) {
    beginResetModel();
    this->_pilots = pilots;
    endResetModel();
}

QVariant AirportDetailsArrivalsModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
            case 0: return QString("Callsign");
            case 1: return QString("Type");
            case 2: return QString("Name");
            case 3: return QString("Arriving From");
            case 4: return QString("T");
            case 5: return QString("Via");
            case 6: return QString("Alt");
            case 7: return QString("Dist");
            case 8: return QString("TTG");
            case 9: return QString("Delay");
            case 10: return QString("Status");
        }
    }
    return QVariant();
}

int AirportDetailsArrivalsModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 11;
}

int AirportDetailsArrivalsModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _pilots.count();
}

QVariant AirportDetailsArrivalsModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || index.row() >= _pilots.size()) {
        return QVariant();
    }
    Pilot* p = _pilots[index.row()];

    if(role == Qt::FontRole) {
        QFont result;
        if(p->flightStatus() == Pilot::PREFILED) {
            result.setItalic(true);
        }
        if(p->isFriend()) {
            result.setBold(true);
        }
        return result;
    } else if(role == Qt::TextAlignmentRole) {
        switch(index.column()) {
            case 6:
            case 7:
            case 8:
            case 9:
                return Qt::AlignRight;
            case 10:
                return Qt::AlignHCenter;
        }

        return Qt::AlignLeft;
    } else if(role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0:
                return p->label;
            case 1:
                return p->aircraftType();
            case 2:
                return p->displayName();
            case 3:
                return p->depAirport() != 0? p->depAirport()->toolTip(): QString();
            case 4:
                return p->planFlighttype;
            case 5:
                return p->waypoints().isEmpty()? QString(): p->waypoints().constLast();
            case 6:
                return (p->altitude == 0? QString(""): p->humanAlt());
            case 7:
                if(p->flightStatus() == Pilot::PREFILED) {
                    return QString();
                } else {
                    return QString("%1 NM").arg(p->distanceToDestination() < 3? 0: (int)p->distanceToDestination());
                }
            case 8:
                if(p->flightStatus() == Pilot::GROUND_ARR || p->flightStatus() == Pilot::BLOCKED) {
                    return "--:-- hrs";
                } else if(!p->eet().toString("H:mm").isEmpty()) {
                    return QString("%1 hrs").arg(p->eet().toString("H:mm"));
                } else {
                    return QString();
                }
            case 9:
                return p->delayStr();
            case 10:
                return p->flightStatusShortString();
        }
    } else if(role == Qt::UserRole) { // used for sorting
        switch(index.column()) {
            case 7:
                return p->flightStatus() == Pilot::PREFILED? -1: p->distanceToDestination();
        }

        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

void AirportDetailsArrivalsModel::modelSelected(const QModelIndex& index) const {
    _pilots[index.row()]->showDetailsDialog();
}
