#include "Net.h"

#include "Settings.h"

//Single instance
Net* netInstance = 0;
Net* Net::instance(bool createIfNoInstance) {
    if ((netInstance == 0) && createIfNoInstance) {
        netInstance = new Net();
    }
    return netInstance;
}

Net::Net()
    : QNetworkAccessManager() {
    if (Settings::useProxy()) {
        setProxy(
            QNetworkProxy(
                QNetworkProxy::DefaultProxy, Settings::proxyServer(),
                Settings::proxyPort(), Settings::proxyUser(),
                Settings::proxyPassword()
            )
        );
    }
}

QNetworkReply* Net::h(QNetworkRequest &request) {
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return Net::instance()->head(request);
}

QNetworkReply* Net::g(const QUrl &url) {
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return Net::instance()->get(request);
}

QNetworkReply* Net::g(QNetworkRequest &request) {
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return Net::instance()->get(request);
}

QNetworkReply* Net::p(QNetworkRequest &request, QIODevice* data) {
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return Net::instance()->post(request, data);
}

QNetworkReply* Net::p(QNetworkRequest &request, const QByteArray &data) {
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return Net::instance()->post(request, data);
}
