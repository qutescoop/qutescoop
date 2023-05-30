#ifndef PING_H
#define PING_H

#include <QtCore>

class Ping: public QObject {
    Q_OBJECT
    public:
        void startPing(QString server);
    signals:
        void havePing(QString, int);
    private slots:
        void pingReadyRead();
    private:
        QProcess* _pingProcess;
        QString _server;
};

#endif // PING_H
