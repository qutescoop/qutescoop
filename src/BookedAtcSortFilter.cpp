#include "BookedAtcSortFilter.h"

bool BookedAtcSortFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (this->_from.isValid() && this->_to.isValid()) {
        if (sourceModel()->index(source_row, 0, source_parent).isValid()) {
            QDateTime starts = sourceModel()->index(source_row, 4, source_parent).data(Qt::EditRole).toDateTime().toUTC();
            QDateTime ends = sourceModel()->index(source_row, 5, source_parent).data(Qt::EditRole).toDateTime().toUTC();
            if (
                (_to == _from && _from <= ends) // _to == _from means for: ever
                || (_from <= starts && _to >= starts)
                || (_from <= ends && _to >= starts)
            ) {
                return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
            }
            return false;
        }
    } else
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    return false;
}

bool BookedAtcSortFilter::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    auto leftData = sourceModel()->data(left, Qt::EditRole);
    auto rightData = sourceModel()->data(right, Qt::EditRole);

    if (leftData.userType() == QMetaType::QDateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
    }
    return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
}

void BookedAtcSortFilter::setDateTimeRange(QDateTime& from, QDateTime& to)
{
    this->_from = from;
    this->_to = to;
    this->invalidateFilter();
}
