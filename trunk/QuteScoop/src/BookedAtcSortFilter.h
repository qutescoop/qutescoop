/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef BOOKEDATCSORTFILTER_H
#define BOOKEDATCSORTFILTER_H

#include <QSortFilterProxyModel>
#include <QtCore>

class BookedAtcSortFilter : public QSortFilterProxyModel {

public:
    BookedAtcSortFilter(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
    void setDateTimeRange(QDateTime& dtfrom, QDateTime& dtto);
    //Q_PROPERTY(QDateTime range READ range WRITE setRange);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    
private:
    QDateTime from;
    QDateTime to;
};

#endif // BOOKEDATCSORTFILTER_H
