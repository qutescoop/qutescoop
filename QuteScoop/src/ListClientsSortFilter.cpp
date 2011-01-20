/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ListClientsSortFilter.h"
#include <QDebug>
#include <QDateTime>

bool ListClientsSortFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    return(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));
}
