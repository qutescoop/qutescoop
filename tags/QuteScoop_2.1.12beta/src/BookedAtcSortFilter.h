/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef BOOKEDATCSORTFILTER_H
#define BOOKEDATCSORTFILTER_H

#include "_pch.h"

class BookedAtcSortFilter : public QSortFilterProxyModel {

    public:
        BookedAtcSortFilter(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
        void setDateTimeRange(QDateTime& dtfrom, QDateTime& dtto);

    protected:
        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

    private:
        QDateTime _from, _to;
};

#endif // BOOKEDATCSORTFILTER_H
