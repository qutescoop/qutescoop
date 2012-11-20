#include "NetworkManager.h"


//Single instance
NetworkManager *NetworkManagerInstance = 0;

NetworkManager* NetworkManager::getInstance(bool createIfNoInstance) {
    if((NetworkManagerInstance == 0) && createIfNoInstance)
        NetworkManagerInstance = new NetworkManager();
    return NetworkManagerInstance;
}

NetworkManager::NetworkManager() {
    connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(redirectCheck(QNetworkReply*)));
    if (Settings::useProxy())
        setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy, Settings::proxyServer(), Settings::proxyPort(), Settings::proxyUser(), Settings::proxyPassword()));
}

QNetworkReply* NetworkManager::httpRequest(QNetworkRequest request) {
    qDebug() << "NetworkManager::httpRequest() " << request.url().toString();
    return this->get(request);
}

void NetworkManager::redirectCheck(QNetworkReply *reply) {
    //qDebug() << "Networkmanager::redirectCheck()";
    //Get redirect headers
    QVariant possibleRedirectUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    //Check if we get fooled
    if(!possibleRedirectUrl.toUrl().isEmpty() && possibleRedirectUrl.toUrl() != _urlRedirectedTo)
        _urlRedirectedTo = possibleRedirectUrl.toUrl();
    else _urlRedirectedTo = QUrl();

    //redirected - make a new request
    if (!_urlRedirectedTo.isEmpty()) {
        this->get(QNetworkRequest(_urlRedirectedTo));
        qDebug() << "NetworkManager::redirectCheck() -- redirected to " << _urlRedirectedTo ;
    } else {
        emit requestFinished(reply);
        _urlRedirectedTo.clear();
        return;
    }
}
