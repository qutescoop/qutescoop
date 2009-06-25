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

#ifndef SEARCHRESULTMODEL_H_
#define SEARCHRESULTMODEL_H_

#include <QAbstractListModel>
#include <QList>
#include "MapObject.h"

class SearchResultModel: public QAbstractListModel {
	Q_OBJECT
	
public:
	SearchResultModel(QObject *parent = 0): QAbstractListModel(parent) {}
	
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation,
	                         int role = Qt::DisplayRole) const;
public slots:
	void setData(const QList<MapObject*>& searchResult) { content = searchResult; reset(); }
	void modelDoubleClicked(const QModelIndex& index);
	void modelClicked(const QModelIndex& index);

private:
	QList<MapObject*> content;
};

#endif /*SEARCHRESULTMODEL_H_*/
