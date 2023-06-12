#ifndef BOOKEDATCSORTFILTER_H_
#define BOOKEDATCSORTFILTER_H_

#include <QtCore>

class BookedAtcSortFilter
    : public QSortFilterProxyModel {

    public:
        BookedAtcSortFilter(QObject* parent = 0)
            : QSortFilterProxyModel(parent) {}
        void setDateTimeRange(QDateTime& dtfrom, QDateTime& dtto);

    protected:
        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    private:
        QDateTime _from, _to;
};

#endif // BOOKEDATCSORTFILTER_H
