#ifndef LISTCLIENTSDIALOGMODEL_H_
#define LISTCLIENTSDIALOGMODEL_H_

#include "Client.h"

class ListClientsDialogModel : public QAbstractTableModel {
        Q_OBJECT

    public:
        ListClientsDialogModel(QObject *parent = 0) : QAbstractTableModel(parent), _clients(QList<Client*>()) {}

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

        virtual QVariant data(const QModelIndex &index, int role) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const;

    public slots:
        void setClients(const QList<Client*>& clients);
        void modelSelected(const QModelIndex& index);

    private:
        QList<Client*> _clients;
};

#endif /*LISTCLIENTSDIALOGMODEL_H_*/
