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
    _server = server;

    _pingProcess = new QProcess(this);
    connect(_pingProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Ping::pingReadyRead);

    QStringList pingArgs;
#ifdef Q_OS_WIN
    pingArgs << "-n 1" << server;
#endif
#ifdef Q_OS_LINUX
    pingArgs << "-c1" << server;
#endif
#ifdef Q_OS_MAC
    pingArgs << "-c1" << server;
#endif
    qDebug() << "Ping::startPing() executing" << "ping" << pingArgs;
    _pingProcess->start("ping", pingArgs);
}
