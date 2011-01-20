/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef LISTCLIENTSSORTFILTER_H
#define LISTCLIENTSSORTFILTER_H

#include <QSortFilterProxyModel>
#include <QtCore>

class ListClientsSortFilter : public QSortFilterProxyModel {

public:
    ListClientsSortFilter(QObject *parent = 0) : QSortFilterProxyModel(parent) {}

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    
};

#endif // LISTCLIENTSSORTFILTER_H
