/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Ping.h"

void Ping::pingReadyRead() {
    QRegExp findMs = QRegExp("(\\d*\\.?\\d*)\\W?ms", Qt::CaseInsensitive);
    if (findMs.indexIn(pingProcess->readAll()) > 0) {
        int ping = (int) findMs.cap(1).toDouble();
        emit havePing(server, ping);
    } else {
        emit havePing(server, -1);
    }
    delete pingProcess;
}

void Ping::startPing(QString server) {
    this->server = server;

    pingProcess = new QProcess(this);
    connect(pingProcess, SIGNAL(finished(int)), this, SLOT(pingReadyRead()));

#ifdef Q_WS_WIN
    QString pingCmd = QString("ping -n 1 %1").arg(server);
#endif
#ifdef Q_WS_X11
    QString pingCmd = QString("ping -c1 %1").arg(server);
#endif
#ifdef Q_WS_MAC
    QString pingCmd = QString("ping -c1 %1").arg(server);
#endif
    pingProcess->start(pingCmd);
}
