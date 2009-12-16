/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
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

#include "Ping.h"

void Ping::pingReadyRead() {
    QRegExp findMs = QRegExp("time\\W?=\\W?(\\d*\\.?\\d*)\\W?ms", Qt::CaseInsensitive);
    if (findMs.indexIn(pingProcess->readAll()) > 0) {
        int ping = (int) findMs.cap(1).toDouble();
        emit havePing(server, ping);
    } else {
        emit havePing(server, -1);
    }
}

void Ping::startPing(QString server) {
    this->server = server;

    pingProcess = new QProcess(this);
    connect(pingProcess, SIGNAL(readyRead()), this, SLOT(pingReadyRead()));

#ifdef Q_WS_WIN
    QString pingCmd = QString("ping %1 -n 1").arg(server);
#endif
#ifdef Q_WS_X11
    QString pingCmd = QString("ping %1 -c1").arg(server);
#endif
#ifdef Q_WS_MAC
    QString pingCmd = QString("ping %1 -c1").arg(server);
#endif
    pingProcess->start(pingCmd);
}
