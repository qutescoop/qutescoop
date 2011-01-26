/**************************************************************************
 *  This file is part of QuteScoop. See README for license
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