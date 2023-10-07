#ifndef NET_H
#define NET_H

#include <QtCore>
#include <QtNetwork>

class Net
    : public QNetworkAccessManager {
    Q_OBJECT
    public:
        static Net* instance(bool createIfNoInstance = true);
        Net();

        // convenience functions: Net::g() vs. Net::instance()->get()
        static QNetworkReply* h(QNetworkRequest &request);
        static QNetworkReply* g(const QUrl &url);
        static QNetworkReply* g(QNetworkRequest &request);
        static QNetworkReply* p(QNetworkRequest &request, QIODevice* data);
        static QNetworkReply* p(QNetworkRequest &request, const QByteArray &data);
};

#endif // NET_H
