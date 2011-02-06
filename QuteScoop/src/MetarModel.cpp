/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include <QUrl>
#include <QDebug>

#include "MetarModel.h"
#include "Settings.h"
#include "Whazzup.h"
#include "Window.h"

#define MAX_METARS 60

MetarModel::MetarModel(QObject *parent):
    QAbstractListModel(parent)
{
    connect(&downloader, SIGNAL(requestFinished(int, bool)), this, SLOT(downloaded(int, bool)));
}

int MetarModel::rowCount(const QModelIndex &parent) const {
    if(airportList.size() > MAX_METARS)
        return 1;

    return metarList.size();
}

int MetarModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant MetarModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole) {
        if(airportList.size() > MAX_METARS)
            return QString("Too many airports match your search (%1)").arg(airportList.size());

        Airport* a = metarList[index.row()];
        switch(index.column()) {
        case 0: return a->metar.encoded; break;
        }
    }

    return QVariant();
}

QVariant MetarModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return QVariant();

    if (section != 0)
        return QVariant();

    if (metarList.isEmpty())
        return QString("No Metars");

    if (metarList.size() == 1)
        return QString("1 Metar");

    return QString("%1 Metars").arg(metarList.size());
}

void MetarModel::setData(const QList<Airport*>& airports)  {
    airportList = airports;
    metarList = airports;

    // remove all entries from metarList with invalid or dead METARs
    for(int i = metarList.size() - 1; i >= 0; i--) {
        if(metarList[i]->metar.needsRefresh() || metarList[i]->metar.doesNotExist())
            metarList.removeAt(i);
    }
    refresh();
}

void MetarModel::refresh() {
    abortAll();
    Settings::applyProxySetting(&downloader);

    if(airportList.size() > MAX_METARS) {
        // avoid hammering the server with gazillions of metar requests
        reset();
        return;
    }

    for(int i = 0; i < airportList.size(); i++) {
        if(airportList[i] == 0) continue;

        if(!airportList[i]->metar.doesNotExist() && airportList[i]->metar.needsRefresh()) {
            downloadMetarFor(airportList[i]);
        }
    }
    reset();
}

void MetarModel::downloadMetarFor(Airport* airport) {
    QString location = Whazzup::getInstance()->getAtisLink(airport->label);
    if(location.isEmpty())
        return;

    QUrl url(location);

    downloader.setHost(url.host(), url.port() != -1 ? url.port() : 80);
    if (!url.userName().isEmpty())
        downloader.setUser(url.userName(), url.password());
    QString querystr = url.path() + "?" + url.encodedQuery();

    DownloadQueue dq;
    dq.airport = airport;
    dq.buffer = new QBuffer;
    dq.buffer->open(QBuffer::ReadWrite);
    int requestId = downloader.get(querystr, dq.buffer);
    downloadQueue[requestId] = dq;
}

void MetarModel::modelClicked(const QModelIndex& index) {
    Airport *a = metarList[index.row()];
    if(a == 0) return;

    if (Window::getInstance(false) != 0)
        Window::getInstance(true)->updateMetarDecoder(a->label,
            a->metar.encoded + "<hr>" + a->metar.decodedHtml());
}

void MetarModel::abortAll() {
    downloader.abort();
}

void MetarModel::downloaded(int id, bool error) {
    if(!downloadQueue.contains(id))
        return;

    DownloadQueue dq = downloadQueue[id];
    downloadQueue.remove(id);

    if(!error) {
        dq.buffer->seek(0);
        QString line = QString(dq.buffer->readLine()).trimmed();
        if(!line.isEmpty())
            dq.airport->metar = Metar(line);
        if(!dq.airport->metar.isNull() && dq.airport->metar.isValid())
            gotMetarFor(dq.airport);
    }

    delete dq.buffer;
}

void MetarModel::gotMetarFor(Airport* airport) {
    if(airportList.contains(airport)) {
        if(!metarList.contains(airport))
            metarList.append(airport);
        qDebug() << "got metar for" << airport->label << ":" << airport->metar.encoded;
        reset();
        emit gotMetar(airport->label);
    }
}
