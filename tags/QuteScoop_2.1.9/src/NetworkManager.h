#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "_pch.h"
#include "Settings.h"

class NetworkManager: public QNetworkAccessManager
{
    Q_OBJECT
public:
    static NetworkManager* getInstance(bool createIfNoInstance = true);
    NetworkManager();

public slots:
    QNetworkReply *httpRequest(QNetworkRequest request);

signals:
    void requestFinished(QNetworkReply* reply );

private slots:
    void redirectCheck (QNetworkReply* reply);


private:
    QUrl _urlRedirectedTo;

};

#endif // NETWORKMANAGER_H
