/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "BookedAtcSortFilter.h"
#include "BookedAtcDialog.h"
#include <QDebug>
#include <QDateTime>

bool BookedAtcSortFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    if (!this->from.isNull() && !this->to.isNull()) {
        if (sourceModel()->index(source_row, 0, source_parent).isValid()) {
            QDateTime starts = sourceModel()->index(source_row, 5, source_parent).data(Qt::EditRole).toDateTime();
            QDateTime ends = sourceModel()->index(source_row, 6, source_parent).data(Qt::EditRole).toDateTime();
            if (to == from && from < ends) return 
                    // "all" selected which corresponds to spinbox value 0
                    (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)); 
            if (from < starts && to > starts) return 
                    (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));
            if (from < ends && to > starts) return 
                    (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));
            return (false);
        }
    } else {
        return(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));    
    }
    return (false);
}

void BookedAtcSortFilter::setDateTimeRange(QDateTime& dtfrom, QDateTime& dtto) {
    this->from = dtfrom;
    this->to = dtto;
    this->invalidateFilter();
}

