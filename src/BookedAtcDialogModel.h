#ifndef BOOKEDATCDIALOGMODEL_H_
#define BOOKEDATCDIALOGMODEL_H_

#include "BookedController.h"

#include <QAbstractTableModel>

class BookedAtcDialogModel
    : public QAbstractTableModel {
    Q_OBJECT

    public:
        BookedAtcDialogModel(QObject* parent = 0)
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
        void setClients(const QList<BookedController*>& controllers);

    private:
        QList<BookedController*> _controllers;
};

#endif /*BOOKEDATCDIALOGMODEL_H_*/
