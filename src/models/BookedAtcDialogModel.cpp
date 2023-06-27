#include "BookedAtcDialogModel.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>

void BookedAtcDialogModel::setClients(const QList<BookedController*> &controllers) {
    qDebug() << "BookedAtcDialogModel/setClients()";
    beginResetModel();
    this->_controllers = controllers;
    endResetModel();
}

QVariant BookedAtcDialogModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QString("Callsign");
            case 1: return QString("Facility");
            case 2: return QString("Name");
            case 3: return QString("Date");
            case 4: return QString("From");
            case 5: return QString("Until");
            case 6: return QString("Info");
        }
    }
    return QVariant();
}

int BookedAtcDialogModel::columnCount(const QModelIndex&) const {
    return 7;
}

QVariant BookedAtcDialogModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || (index.row() >= rowCount(index))) {
        return QVariant();
    }

    BookedController* c = _controllers.value(index.row(), nullptr);
    if (c == nullptr) {
        return QVariant();
    }

    if (role == Qt::FontRole) {
        QFont result;
        result.setBold(c->isFriend());
        return result;
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return c->callsign;
            case 1: return c->facilityString();
            case 2: return c->realName();
            case 3: return c->starts().toString("MM/dd (ddd)");
            case 4: return c->starts().time().toString("HHmm'z'");
            case 5: return c->ends().time().toString("HHmm'z'");
            case 6: return c->bookingInfoStr;
        }
        return QVariant();
    }

    if (role == Qt::UserRole) { // for sorting
        switch (index.column()) {
            case 3: return c->starts();
            case 4: return c->starts();
            case 5: return c->ends();
        }
        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

int BookedAtcDialogModel::rowCount(const QModelIndex&) const {
    return _controllers.count();
}
