#include "AirportDetailsArrivalsModel.h"

#include "src/Airport.h"
#include "src/NavData.h"
#include "src/Settings.h"

#include <QFont>

void AirportDetailsArrivalsModel::setClients(const QList<Pilot*>& pilots) {
    beginResetModel();
    this->_pilots = pilots;
    endResetModel();
}

QVariant AirportDetailsArrivalsModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
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

int AirportDetailsArrivalsModel::columnCount(const QModelIndex&) const {
    return 11;
}

int AirportDetailsArrivalsModel::rowCount(const QModelIndex&) const {
    return _pilots.count();
}

QVariant AirportDetailsArrivalsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= _pilots.size()) {
        return QVariant();
    }
    Pilot* p = _pilots[index.row()];

    if (index.column() == 8) {
        const bool isFilteredArrival = Settings::filterTraffic() && (
            (p->distanceToDestination() < Settings::filterDistance())
            || (p->eet().hour() + p->eet().minute() / 60. < Settings::filterArriving())
        )
            && (p->flightStatus() != Pilot::FlightStatus::BLOCKED && p->flightStatus() != Pilot::FlightStatus::GROUND_ARR);

        if (isFilteredArrival) {
            if (role == Qt::ForegroundRole) {
                return QGuiApplication::palette().window();
            } else if (role == Qt::BackgroundRole) {
                return QGuiApplication::palette().text();
            }
        }
    }

    if (role == Qt::FontRole) {
        QFont result;
        if (p->flightStatus() == Pilot::PREFILED) {
            result.setItalic(true);
        }
        if (p->isFriend()) {
            result.setBold(true);
        }

        return result;
    }

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case 6:
            case 7:
            case 8:
            case 9:
                return Qt::AlignRight;
            case 4:
            case 10:
                return Qt::AlignHCenter;
        }

        return Qt::AlignLeft;
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return p->callsign;
            case 1:
                return p->aircraftType();
            case 2:
                return p->displayName();
            case 3:
                return p->depAirport() != 0? p->depAirport()->toolTip(): "";
            case 4:
                return p->planFlighttype;
            case 5:
                return p->waypoints().isEmpty()? "": p->waypoints().constLast();
            case 6:
                return p->altitude == 0? QString(""): p->humanAlt();
            case 7:
                if (p->distanceToDestination() < 3) {
                    return "-";
                } else {
                    return QString("%1 NM").arg((int) p->distanceToDestination());
                }
            case 8:
                if (p->flightStatus() == Pilot::GROUND_ARR || p->flightStatus() == Pilot::BLOCKED) {
                    return "--:-- hrs";
                } else if (!p->eet().toString("H:mm").isEmpty()) {
                    return QString("%1 hrs").arg(p->eet().toString("H:mm"));
                } else {
                    return "";
                }
            case 9:
                return p->delayString();
            case 10:
                return p->flightStatusShortString();
        }

        return QVariant();
    }

    if (role == Qt::UserRole) { // used for sorting
        switch (index.column()) {
            case 7:
                return p->distanceToDestination();
            case 8:
                auto eta = p->eta();
                if (!eta.isValid()) {
                    return 0;
                }
                return eta.toSecsSinceEpoch();
        }

        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

void AirportDetailsArrivalsModel::modelSelected(const QModelIndex& index) const {
    if (_pilots[index.row()]->hasPrimaryAction()) {
        _pilots[index.row()]->primaryAction();
    }
}
