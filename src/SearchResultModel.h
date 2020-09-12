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

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

        virtual QVariant data(const QModelIndex &index, int role) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const;
    public slots:
        void setData(const QList<MapObject*>& searchResult) { beginResetModel(); _content = searchResult; endResetModel(); }
        void modelDoubleClicked(const QModelIndex& index);
        void modelClicked(const QModelIndex& index);

    private:
        QList<MapObject*> _content;
};

#endif /*SEARCHRESULTMODEL_H_*/
