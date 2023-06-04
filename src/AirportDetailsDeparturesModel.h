#ifndef AIRPORTDETAILSDEPARTURESMODEL_H_
#define AIRPORTDETAILSDEPARTURESMODEL_H_

#include "Pilot.h"

class AirportDetailsDeparturesModel
    : public QAbstractTableModel {
    Q_OBJECT

    public:
        AirportDetailsDeparturesModel(QObject* parent = 0)
            : QAbstractTableModel(parent) {}

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole
        ) const override;

    public slots:
        void setClients(const QList<Pilot*>& pilots);
        void modelSelected(const QModelIndex& index) const;

    private:
        QList<Pilot*> _pilots;
};

#endif /*AIRPORTDETAILSDEPARTURESMODEL_H_*/
