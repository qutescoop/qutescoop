/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SEARCHRESULTMODEL_H_
#define SEARCHRESULTMODEL_H_

#include "_pch.h"

#include "MapObject.h"

class SearchResultModel: public QAbstractListModel {
        Q_OBJECT

    public:
        SearchResultModel(QObject *parent = 0): QAbstractListModel(parent) {}

        Qt::ItemFlags flags(const QModelIndex &index) const override;

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;
    public slots:
        void setSearchResults(const QList<MapObject*>& searchResult) { beginResetModel(); _content = searchResult; endResetModel(); }
        void modelClicked(const QModelIndex& index);

    private:
        QList<MapObject*> _content;
};

#endif /*SEARCHRESULTMODEL_H_*/
