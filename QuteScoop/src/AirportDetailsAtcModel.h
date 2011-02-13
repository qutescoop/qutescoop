/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef AIRPORTDETAILSATCMODEL_H_
#define AIRPORTDETAILSATCMODEL_H_

#include "_pch.h"

#include "Controller.h"

class AirportDetailsAtcModel : public QAbstractTableModel {
    Q_OBJECT

public:
    AirportDetailsAtcModel(QObject *parent = 0) : QAbstractTableModel(parent) {}

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const { return controllers.count(); }
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const { return 8; }

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole) const;

public slots:
    void setClients(const QList<Controller*>& controllers);
    void modelSelected(const QModelIndex& index);

private:
    QList<Controller*> controllers;
};

#endif /*CLIENTLISTMODEL_H_*/
