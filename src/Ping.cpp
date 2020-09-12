/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Ping.h"

void Ping::pingReadyRead() {
    QRegExp findMs = QRegExp("(\\d*\\.?\\d*)\\W?ms", Qt::CaseInsensitive);
    if (findMs.indexIn(_pingProcess->readAll()) > 0) {
        int ping = (int) findMs.cap(1).toDouble();
        emit havePing(_server, ping);
    } else {
        emit havePing(_server, -1);
    }
}

void Ping::startPing(QString server) {
    this->_server = server;

    _pingProcess = new QProcess(this);
    connect(_pingProcess, SIGNAL(finished(int)), this, SLOT(pingReadyRead()));

#ifdef Q_WS_WIN
    QString pingCmd = QString("ping -n 1 %1").arg(server);
#endif
#ifdef Q_OS_LINUX
    QString pingCmd = QString("ping -c1 %1").arg(server);
#endif
#ifdef Q_WS_MAC
    QString pingCmd = QString("ping -c1 %1").arg(server);
#endif
    qDebug() << "Ping::startPing() executing" << pingCmd;
    _pingProcess->start(pingCmd);
}
