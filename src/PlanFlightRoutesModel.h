#ifndef PLANFLIGHTSROUTEMODEL_H_
#define PLANFLIGHTSROUTEMODEL_H_

#include "Route.h"

class PlanFlightRoutesModel
    : public QAbstractTableModel {
    Q_OBJECT

    public:
        PlanFlightRoutesModel(QObject* parent = 0);

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole
        ) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    public slots:
        void setClients(const QList<Route*>& routes);

    private:
        QList<Route*> _routes;
};

#endif
