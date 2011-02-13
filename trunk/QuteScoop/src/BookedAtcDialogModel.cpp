/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "BookedAtcDialogModel.h"

#include "_pch.h"

#include "Window.h"

void BookedAtcDialogModel::setClients(const QList<BookedController*> &controllers) {
    qDebug() << "BookedAtcDialogModel/setClients()";
    beginResetModel();
    this->controllers = controllers;
    endResetModel();
}

QVariant BookedAtcDialogModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Vertical)
        return QVariant();

    // orientation is Qt::Horizontal
    switch(section) {
        case 0: return QString("Callsign"); break;
        case 1: return QString("Facility"); break;
        case 2: return QString("Country"); break;
        case 3: return QString("Name"); break;
        case 4: return QString("Date"); break;
        case 5: return QString("From"); break;
        case 6: return QString("Until"); break;
        case 7: return QString("Info"); break;
        case 8: return QString("Link"); break;
    }

    return QVariant();
}

QVariant BookedAtcDialogModel::data(const QModelIndex &index, int role) const {
    // keep GUI responsive
    //qApp->processEvents();

    if(!index.isValid())
        return QVariant();

    if((index.row() > rowCount(index)) || (index.column() > columnCount(index)))
        return QVariant();

    if(role == Qt::DisplayRole) {
        BookedController* c = controllers[index.row()];
        switch(index.column()) {
            case 0: return c->label; break;
            case 1: return c->facilityString(); break;
            case 2: return c->countryCode; break;
            case 3: return c->realName; break;
            case 4: return c->starts().toString("MM/dd (ddd)"); break;
            case 5: return c->starts().time().toString("HHmm'z'"); break;
            case 6: return c->ends().time().toString("HHmm'z'"); break;
            case 7: return c->bookingInfoStr; break;
            case 8: return c->link; break;
        }
    } else if(role == Qt::EditRole) {
        BookedController* c = controllers[index.row()];
        switch(index.column()) {
            case 5: return c->starts(); break;
            case 6: return c->ends(); break;
            case 7: return c->bookingInfoStr; break;
            case 8: return c->link; break;
        }
    }

    return QVariant();
}

int BookedAtcDialogModel::rowCount(const QModelIndex &parent) const {
    return controllers.count();
}

int BookedAtcDialogModel::columnCount(const QModelIndex &parent) const {
    return 9;
}

void BookedAtcDialogModel::modelSelected(const QModelIndex& index) {
    if(controllers[index.row()] != 0) {
        if(!controllers[index.row()]->link.isEmpty()) {
            QUrl url = QUrl(controllers[index.row()]->link, QUrl::TolerantMode);
            if(QMessageBox::question(qApp->activeWindow(), tr("Question"), tr("Open %1 in your browser?").arg(url.toString()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                if (url.isValid()) {
                    if(!QDesktopServices::openUrl(url))
                        QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("Could not invoke browser"));
                } else
                    QMessageBox::critical(qApp->activeWindow(), tr("Error"), tr("URL %1 is invalid").arg(url.toString()));
            }
        }
        controllers[index.row()]->showDetailsDialog();
    }
}
