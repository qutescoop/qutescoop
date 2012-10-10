/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "PlanFlightRoutesModel.h"

#include "PlanFlightDialog.h"

void PlanFlightRoutesModel::setClients(const QList<Route*>& newroutes) {
    routes = newroutes;
    reset();
}

QVariant PlanFlightRoutesModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Vertical)
        return QVariant();

    // orientation is Qt::Horizontal
    switch(section) {
    case 0: return QString(); break;
    case 1: return QString("Provider"); break;
    case 2: return QString("Route"); break;
    case 3: return QString("Dist"); break;
    case 4: return QString("FL>"); break;
    case 5: return QString("FL<"); break;
    case 6: return QString("Remarks"); break;
    case 7: return QString("Last changed"); break;
    }

    return QVariant();
}

QVariant PlanFlightRoutesModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(index.row() >= routes.size())
        return QVariant();

    if(role == Qt::DecorationRole) {
        Route* r = routes[index.row()];
        switch(index.column()) {
        case 0:
            if (r->provider == "user")
                return QPixmap(":/icons/qutescoop.png").
                        scaled(QSize(50, 25), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            else if (r->provider == "vroute")
                return QPixmap(":/routeproviders/images/vroute.png").
                        scaled(QSize(50, 25), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            else if (r->provider == "VATroute")
                return QPixmap(":/routeproviders/images/vatroute.png").
                        scaled(QSize(50, 25), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            break;
        }
    } else if(role == Qt::DisplayRole) {
        Route* r = routes[index.row()];
        switch(index.column()) {
        case 1: return r->provider; break;
        case 2: return QString("%1").arg(r->route); break;
        case 3: return QString("%1").arg(r->routeDistance); break;
        case 4: return r->minFl; break;
        case 5: return r->maxFl; break;
        case 6: return r->comments; break;
        case 7:
            QDateTime lastChange = QDateTime::fromString(r->lastChange, "yyyyMMddHHmmss");
            if (lastChange.isValid()) return lastChange.date();
            else return r->lastChange;
            break;
        }
    }

    return QVariant();
}

void PlanFlightRoutesModel::modelSelected(const QModelIndex& index) {
    //routes[index.row()]->showDetailsDialog();
}

bool PlanFlightRoutesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    //qDebug() << "setData" << role << value;
    return true;
}

Qt::ItemFlags PlanFlightRoutesModel::flags(const QModelIndex &index) const {
    // make column 4 edittable
    //if (index.column() == 4)
    //    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
