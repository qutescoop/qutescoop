#include "Net.h"


//Single instance
Net *networkManagerInstance = 0;
Net *Net::instance(bool createIfNoInstance) {
    if((networkManagerInstance == 0) && createIfNoInstance)
            networkManagerInstance = new Net();
    return networkManagerInstance;
}

Net::Net() : QNetworkAccessManager() {
    if (Settings::useProxy())
            setProxy(QNetworkProxy(
                     QNetworkProxy::DefaultProxy, Settings::proxyServer(),
                     Settings::proxyPort(), Settings::proxyUser(),
                     Settings::proxyPassword()));
}

QNetworkReply *Net::h(const QNetworkRequest &request) {
    return Net::instance()->head(request);
}

QNetworkReply *Net::g(const QNetworkRequest &request) {
    return Net::instance()->get(request);
}

QNetworkReply *Net::p(const QNetworkRequest &request, QIODevice *data) {
    return Net::instance()->post(request, data);
}

QNetworkReply *Net::p(const QNetworkRequest &request, const QByteArray &data) {
    return Net::instance()->post(request, data);
}
