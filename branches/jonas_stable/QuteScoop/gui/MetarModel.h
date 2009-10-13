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

#ifndef METARMODEL_H_
#define METARMODEL_H_

#include <QAbstractListModel>
#include <QHttp>

#include "Metar.h"
#include "Airport.h"

class MetarModel: public QAbstractListModel {
    Q_OBJECT

public:
    MetarModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole) const;

signals:
    void gotMetar(QString icao);

public slots:
    void setData(const QList<Airport*>& airports);
    void modelClicked(const QModelIndex& index);
    void refresh();

private slots:
    void downloaded(int id, bool error);
    void gotMetarFor(Airport *airport);

private:
    class DownloadQueue {
    public:
        Airport *airport;
        QBuffer *buffer;
    };
    QHash<int, DownloadQueue> downloadQueue;

    void abortAll();
    void downloadMetarFor(Airport* airport);

    QList<Airport*> airportList;
    QList<Airport*> metarList;

    QHttp downloader;
};

#endif /*METARMODEL_H_*/
