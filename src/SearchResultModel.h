#ifndef SEARCHRESULTMODEL_H_
#define SEARCHRESULTMODEL_H_

#include "MapObject.h"

class SearchResultModel: public QAbstractListModel {
    Q_OBJECT

    public:
        SearchResultModel(QObject* parent = 0);

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QVariant headerData(
        int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole
        ) const override;
    public slots:
        void setSearchResults(const QList<MapObject*>& searchResult);
        void modelClicked(const QModelIndex& index);

    private:
        QList<MapObject*> _content;
};

#endif /*SEARCHRESULTMODEL_H_*/
