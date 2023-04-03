// TODO: remove

#ifndef SONDEDATA_H
#define SONDEDATA_H

#include "Station.h"
#include <QtOpenGL>
#include <QtNetwork>

class SondeData : public QObject {
        Q_OBJECT
    public:
        static SondeData *instance(bool createIfNoInstance = true);
        explicit SondeData(QObject *parent = 0);
        ~SondeData();

        GLuint windArrows(int alt1k, bool secondary = false);
        bool downloaded() { return !_stationRawData.isEmpty(); }
        void setRawData(QString);
        void decodeData();
        void invalidateWindLists();
        QHash<int, Station*> stationList;
    signals:
        void loaded();
    public slots:
        void load();
    private slots:
        void sondeDataProgress(qint64 prog, qint64 total);
        void processSondeData();
    private:
        QNetworkReply *_replySondeData;
        QString _rawData;
        QStringList _stationRawData;
        QHash<int, GLuint> _windList, _windListSecondary;
        GLuint _result;
};


#endif // SONDEDATA_H
