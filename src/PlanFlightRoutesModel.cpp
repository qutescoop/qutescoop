/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "PlanFlightRoutesModel.h"

#include "PlanFlightDialog.h"

void PlanFlightRoutesModel::setClients(const QList<Route*>& routes) {
    beginResetModel();
    _routes = routes;
    endResetModel();
}

QVariant PlanFlightRoutesModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return QVariant();

    // orientation is Qt::Horizontal
    switch (section) {
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
    if (!index.isValid() || index.row() >= _routes.size())
        return QVariant();

    if(role == Qt::DecorationRole) {
        Route* r = _routes[index.row()];
        switch(index.column()) {
            case 0:
                return QPixmap(
                            r->provider == "user"
                            ? ":/icons/qutescoop.png"
                            : r->provider == "vroute"? ":/routeproviders/images/vroute.png"
                                                     : ":/routeproviders/images/vatroute.png"
                                                       )
                        .scaled(QSize(50, 25), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    } else if(role == Qt::DisplayRole) {
        Route* r = _routes[index.row()];
        switch(index.column()) {
            case 1: return r->provider;
            case 2: return QString("%1").arg(r->route);
            case 3: return QString("%1").arg(r->routeDistance);
            case 4: return r->minFl;
            case 5: return r->maxFl;
            case 6: return r->comments;
            case 7:
                QDateTime lastChange = QDateTime::fromString(r->lastChange, "yyyyMMddHHmmss");
                if (lastChange.isValid()) return lastChange.date();
                else return r->lastChange;
        }
    }

    return QVariant();
}

bool PlanFlightRoutesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    //qDebug() << "setData" << role << value;
    return true;
}

Qt::ItemFlags PlanFlightRoutesModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index);
    // make column 4 edittable
    //if (index.column() == 4)
    //    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
