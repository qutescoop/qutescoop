#ifndef AIRPORTDETAILSATCMODEL_H_
#define AIRPORTDETAILSATCMODEL_H_

#include "Controller.h"

class AirportDetailsAtcModel: public QAbstractTableModel {
    Q_OBJECT

    public:
        AirportDetailsAtcModel(QObject* parent = 0) : QAbstractTableModel(parent) {}

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

        virtual QVariant data(const QModelIndex &index, int role) const;
        virtual QVariant headerData(
        int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole
        ) const;

    public slots:
        void setClients(const QList<Controller*>& controllers);
        void modelSelected(const QModelIndex& index) const;

    private:
        QList<Controller*> _controllers;
};

#endif /*CLIENTLISTMODEL_H_*/
