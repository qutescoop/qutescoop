#ifndef NET_H
#define NET_H

#include "_pch.h"
#include "Settings.h"

class Net : public QNetworkAccessManager {
        Q_OBJECT
    public:
        static Net *instance(bool createIfNoInstance = true);
        Net();

        // convenience functions: Net::g() vs. Net::instance()->get()
        static QNetworkReply *h(const QNetworkRequest &request);
        static QNetworkReply *g(const QUrl &url);
        static QNetworkReply *g(const QNetworkRequest &request);
        static QNetworkReply *p(const QNetworkRequest &request, QIODevice *data);
        static QNetworkReply *p(const QNetworkRequest &request, const QByteArray &data);
};

#endif // NET_H
