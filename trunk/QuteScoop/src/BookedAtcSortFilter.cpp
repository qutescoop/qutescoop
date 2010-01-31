/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
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

