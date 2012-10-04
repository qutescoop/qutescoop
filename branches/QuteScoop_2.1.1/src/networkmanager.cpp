#include "networkmanager.h"


//Singel instance
NetworkManager *NewokeManagerInstance = 0;

NetworkManager* NetworkManager::getInstance(bool createIfNoInstance) {
    if(NewokeManagerInstance == 0)
        if (createIfNoInstance)
            NewokeManagerInstance = new NetworkManager;
    return NewokeManagerInstance;
}

NetworkManager::NetworkManager()
{
    connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(redirectCheck(QNetworkReply*)));
    if (Settings::useProxy())
        setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy, Settings::proxyServer(), Settings::proxyPort(), Settings::proxyUser(), Settings::proxyPassword()));
}

void NetworkManager::httpRequest(QNetworkRequest request){
    this->get(request);
    qDebug() << "NetworkManager::httpRequest " << request.url().toString();
}

void NetworkManager::redirectCheck(QNetworkReply *reply){

    qDebug() << "Networkmanager::redirectCheck";
    //Get redirect headers
    QVariant possibleRedirectUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    //Check if we get fooled
    if(!possibleRedirectUrl.toUrl().isEmpty() && possibleRedirectUrl.toUrl() != _urlRedirectedTo)
        _urlRedirectedTo = possibleRedirectUrl.toUrl();
    else _urlRedirectedTo = QUrl();

    //redirected - make a new request
    if(!_urlRedirectedTo.isEmpty()){
        this->get(QNetworkRequest(_urlRedirectedTo));
        qDebug() << "NetworkManger::redirectCheck -- redirected to " << _urlRedirectedTo ;
    }
    //no redirect
    else {
        emit requestFinished(reply);
        _urlRedirectedTo.clear();
        return;
    }
}
