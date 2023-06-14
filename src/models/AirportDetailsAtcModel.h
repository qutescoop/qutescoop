#ifndef AIRPORTDETAILSATCMODEL_H_
#define AIRPORTDETAILSATCMODEL_H_

#include "../Controller.h"
#include "items/AirportDetailsAtcModelItem.h"

class AirportDetailsAtcModel
    : public QAbstractItemModel {
    Q_OBJECT

    public:
        static const int nColumns = 8;

        explicit AirportDetailsAtcModel(QObject* parent = 0);
        ~AirportDetailsAtcModel();

        QVariant data(const QModelIndex &index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole
        ) const override;
        QModelIndex index(
            int row,
            int column,
            const QModelIndex &parent = QModelIndex()
        ) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        void setClients(QList<Controller*> controllers);
        void modelSelected(const QModelIndex& index) const;
        void writeExpandedState(const QModelIndex& index, bool isExpanded);

        bool isExpanded(AirportDetailsAtcModelItem*) const;

        QMap<QString, QVariant> m_atcExpandedByType;
    private:
        const QStringList typesSorted { "FSS", "CTR", "DEP", "APP", "TWR", "GND", "DEL", "ATIS", "OBS" };

        AirportDetailsAtcModelItem* rootItem;
};

#endif /*CLIENTLISTMODEL_H_*/
