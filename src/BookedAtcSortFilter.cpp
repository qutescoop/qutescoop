/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "BookedAtcSortFilter.h"

#include "BookedAtcDialog.h"

bool BookedAtcSortFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    if (!this->_from.isNull() && !this->_to.isNull()) {
        if (sourceModel()->index(source_row, 0, source_parent).isValid()) {
            QDateTime starts =
                    sourceModel()->index(source_row, 5, source_parent)
                    .data(Qt::EditRole).toDateTime();
            QDateTime ends =
                    sourceModel()->index(source_row, 6, source_parent)
                    .data(Qt::EditRole).toDateTime();
            if (_to == _from && _from < ends) return
                    (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));
            if (_from < starts && _to > starts) return
                    (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));
            if (_from < ends && _to > starts) return
                    (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));
            return (false);
        }
    } else
        return(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));
    return (false);
}

void BookedAtcSortFilter::setDateTimeRange(QDateTime& dtfrom, QDateTime& dtto) {
    this->_from = dtfrom;
    this->_to = dtto;
    this->invalidateFilter();
}

