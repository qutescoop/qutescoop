#include "PlanFlightRoutesModel.h"

#include <QPixmap>

void PlanFlightRoutesModel::setClients(const QList<Route*>& routes) {
    beginResetModel();
    _routes = routes;
    endResetModel();
}

QVariant PlanFlightRoutesModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    // orientation is Qt::Horizontal
    switch (section) {
        case 0: return ""; break;
        case 1: return "Provider"; break;
        case 2: return "Route"; break;
        case 3: return "Dist"; break;
        case 4: return "FL>"; break;
        case 5: return "FL<"; break;
        case 6: return "Remarks"; break;
        case 7: return "Last changed"; break;
    }

    return QVariant();
}

PlanFlightRoutesModel::PlanFlightRoutesModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int PlanFlightRoutesModel::rowCount(const QModelIndex&) const {
    return _routes.count();
}

int PlanFlightRoutesModel::columnCount(const QModelIndex&) const {
    return 8;
}

QVariant PlanFlightRoutesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= _routes.size()) {
        return QVariant();
    }

    if (role == Qt::DecorationRole) {
        Route* r = _routes[index.row()];
        switch (index.column()) {
            case 0:
                return QPixmap(
                    r->provider == "user"
                            ? ":/icons/qutescoop.png"
                            : ":/routeproviders/images/vroute.png"
                )
                    .scaled(QSize(50, 25), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    } else if (role == Qt::DisplayRole) {
        Route* r = _routes[index.row()];
        switch (index.column()) {
            case 1: return r->provider;
            case 2: return QString("%1").arg(r->route);
            case 3: return QString("%1").arg(r->routeDistance);
            case 4: return r->minFl;
            case 5: return r->maxFl;
            case 6: return r->comments;
            case 7:
                QDateTime lastChange = QDateTime::fromString(r->lastChange, "yyyyMMddHHmmss");
                if (lastChange.isValid()) {
                    return lastChange.date();
                } else {
                    return r->lastChange;
                }
        }
    }

    return QVariant();
}

bool PlanFlightRoutesModel::setData(const QModelIndex&, const QVariant&, int) {
    //qDebug() << "setData" << role << value;
    return true;
}

Qt::ItemFlags PlanFlightRoutesModel::flags(const QModelIndex&) const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
