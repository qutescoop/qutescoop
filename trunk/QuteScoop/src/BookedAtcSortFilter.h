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
