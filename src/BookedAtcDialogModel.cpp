/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "BookedAtcDialogModel.h"

#include "_pch.h"

#include "Window.h"

void BookedAtcDialogModel::setClients(const QList<BookedController*> &controllers) {
    qDebug() << "BookedAtcDialogModel/setClients()";
    beginResetModel();
    this->_controllers = controllers;
    endResetModel();
}

QVariant BookedAtcDialogModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
            case 0: return QString("Callsign");
            case 1: return QString("Facility");
            case 2: return QString("Name");
            case 3: return QString("Date");
            case 4: return QString("From");
            case 5: return QString("Until");
            case 6: return QString("Info");
            case 7: return QString("Link");
        }
    }
    return QVariant();
}

int BookedAtcDialogModel::columnCount(const QModelIndex&) const {
    return 8;
}

QVariant BookedAtcDialogModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || (index.row() >= rowCount(index)))
        return QVariant();

    if(role == Qt::DisplayRole) {
        BookedController* c = _controllers[index.row()];
        switch(index.column()) {
            case 0: return c->label;
            case 1: return c->facilityString();
            case 2: return c->realName;
            case 3: return c->starts().toString("MM/dd (ddd)");
            case 4: return c->starts().time().toString("HHmm'z'");
            case 5: return c->ends().time().toString("HHmm'z'");
            case 6: return c->bookingInfoStr;
            case 7: return c->link;
        }
    } else if(role == Qt::EditRole) { // we are faking "EditRole" to access raw data
        BookedController* c = _controllers[index.row()];
        switch(index.column()) {
            case 4: return c->starts(); break;
            case 5: return c->ends(); break;
            case 6: return c->bookingInfoStr; break;
            case 7: return c->link; break;
        }
    }

    return QVariant();
}

int BookedAtcDialogModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _controllers.count();
}

void BookedAtcDialogModel::modelSelected(const QModelIndex& index) const {
    if(_controllers[index.row()] != 0) {
        if(!_controllers[index.row()]->link.isEmpty()) {
            QUrl url = QUrl(_controllers[index.row()]->link, QUrl::TolerantMode);
            if(QMessageBox::question(qApp->activeWindow(),
                                     tr("Question"),
                                     tr("Open %1 in your browser?")
                                        .arg(url.toString()),
                                     QMessageBox::Yes | QMessageBox::No
            ) == QMessageBox::Yes) {
                if (url.isValid()) {
                    if(!QDesktopServices::openUrl(url))
                        QMessageBox::critical(qApp->activeWindow(),
                                              tr("Error"),
                                              tr("Could not invoke browser"));
                } else
                    QMessageBox::critical(qApp->activeWindow(),
                                          tr("Error"),
                                          tr("URL %1 is invalid").arg(url.toString()));
            }
        }
        _controllers[index.row()]->showDetailsDialog();
    }
}
