#ifndef METARMODEL_H_
#define METARMODEL_H_

#include "Airport.h"

#include <QtNetwork>

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
        void gotMetar(const QString &icao, const QString &encoded, const QString &decoded);

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
