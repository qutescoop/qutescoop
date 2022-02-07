/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef METARMODEL_H_
#define METARMODEL_H_

#include "_pch.h"

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
        void gotMetar(QString icao, QString encoded);

    public slots:
        void setAirports(const QList<Airport*>& airports);
        void modelClicked(const QModelIndex& index);
        void refresh();

    private slots:
        void metarReplyFinished();
        void gotMetarFor(Airport *airport);

    private:
        QHash<QUrl, Airport*> _downloadQueue;
        QNetworkReply *_metarReply;

        void downloadNextFromQueue();
        void downloadMetarFor(Airport* airport);

        QList<Airport*> _airportList;
        QList<Airport*> _metarList;
};

#endif /*METARMODEL_H_*/
