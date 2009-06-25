/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#include "AirportDetailsArrivalsModel.h"

#include <QtAlgorithms>
#include <QDebug>

void AirportDetailsArrivalsModel::setClients(const QList<Pilot*>& pilots) {
	this->pilots = pilots;
	reset();
}

QVariant AirportDetailsArrivalsModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Vertical)
		return QVariant();

	// orientation is Qt::Horizontal
	switch (section) {
        case 0: return QString("Callsign"); break;
        case 1: return QString("Type"); break;
        case 2: return QString("Name"); break;
        case 3: return QString("Arriving From"); break;
        case 4: return QString("Via"); break;
        case 5: return QString("Alt"); break;
        case 6: return QString("Speed"); break;
        case 7: return QString("Dist"); break;
        case 8: return QString("Expected"); break;
        case 9: return QString("Delay"); break;
        case 10: return QString("Status"); break;
    }

	return QVariant();
}

int AirportDetailsArrivalsModel::rowCount(const QModelIndex &parent) const {
	return pilots.count();
}

int AirportDetailsArrivalsModel::columnCount(const QModelIndex &parent) const {
    return 11;
}

QVariant AirportDetailsArrivalsModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.row() >= pilots.size())
		return QVariant();
	
    Pilot* p = pilots[index.row()];
	
    if(role == Qt::FontRole) {
        if (p->flightStatus() == Pilot::PREFILED) {
            QFont result;
            result.setItalic(true);
            return result;
        }
		return QFont();
	} else if (role == Qt::DisplayRole) {
		switch (index.column()) {
            case 0:
                return p->label; break;
            case 1:
                return p->aircraftType(); break;
            case 2:
                return p->displayName(); break;
            case 3:
                if (p->depAirport() != 0) return p->depAirport()->toolTip(); break;
            case 4:
                return p->waypoints().last(); break;
            case 5:
                return (p->altitude == 0? QString(""): QString("%1").arg(p->altitude)); break;
            case 6:
                return (p->groundspeed == 0? QString(""): QString("%1").arg(p->groundspeed)); break;
            case 7:
                if(p->flightStatus() == Pilot::PREFILED)
                    return "n/a";
                else
                    return (p->distanceToDestination() < 3? 0: (int)p->distanceToDestination());
                break;
            case 8:
                if (p->flightStatus() == Pilot::GROUND_ARR | p->flightStatus() == Pilot::BLOCKED)
                    return "--:--";
                else if(!p->ete().toString("H:mm").isEmpty())
                    return p->ete().toString("H:mm");
                else
                    return "n/a";
                break;
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

void AirportDetailsArrivalsModel::modelSelected(const QModelIndex& index) {
	pilots[index.row()]->showDetailsDialog();
}
