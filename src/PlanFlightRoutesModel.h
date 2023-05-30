#ifndef PLANFLIGHTSROUTEMODEL_H_
#define PLANFLIGHTSROUTEMODEL_H_

#include "Route.h"

class PlanFlightRoutesModel : public QAbstractTableModel {
        Q_OBJECT

    public:
        PlanFlightRoutesModel(QObject *parent = 0) : QAbstractTableModel(parent)
        {}

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const {
            Q_UNUSED(parent);
            return _routes.count();
        }
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const {
            Q_UNUSED(parent);
            return 8;
        }

        virtual QVariant data(const QModelIndex &index, int role) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    public slots:
        void setClients(const QList<Route*>& routes);
        //    void modelSelected(const QModelIndex& index);

    private:
        QList<Route*> _routes;
};

#endif
