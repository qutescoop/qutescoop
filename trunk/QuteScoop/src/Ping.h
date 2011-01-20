/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef PING_H
#define PING_H

#include <QObject>
#include <QProcess>

class Ping: public QObject
{
    Q_OBJECT

public:
    void startPing(QString server);

signals:
    void havePing(QString,int);

private slots:
    void pingReadyRead();

private:
    QProcess *pingProcess;
    QString server;
};

#endif // PING_H
