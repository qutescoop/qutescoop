/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "MetarModel.h"

#include "Settings.h"
#include "Whazzup.h"
#include "Window.h"

#define MAX_METARS 60

MetarModel::MetarModel(QObject *parent):
        QAbstractListModel(parent),
        _metarReply(0) {
}

int MetarModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if(_airportList.size() > MAX_METARS)
        return 1;

    return _metarList.size();
}

int MetarModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant MetarModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole) {
        if(_airportList.size() > MAX_METARS)
            return QString("Too many airports match your search (%1)").arg(_airportList.size());

        Airport* a = _metarList[index.row()];
        switch(index.column()) {
        case 0: return a->metar.encoded; break;
        }
    }

    if(role == Qt::ToolTipRole) {
        if(_airportList.size() > MAX_METARS)
            return QString("Too many airports match your search (%1)").arg(_airportList.size());

        Airport* a = _metarList[index.row()];
        switch(index.column()) {
        case 0: return a->metar.decodedHtml(); break;
        }
    }

    return QVariant();
}

QVariant MetarModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation != Qt::Horizontal)
        return QVariant();

    if (section != 0)
        return QVariant();

    QString ret;

    if (_metarList.isEmpty())
        ret.append("No METARs");
    else if (_metarList.size() == 1)
        ret.append("1 METAR");
    else
        ret.append(QString("%1 METARs").arg(_metarList.size()));

    if (_metarReply != 0 && _metarReply->isRunning())
        ret.append(" …");

    return ret;
}

void MetarModel::setAirports(const QList<Airport*>& airports)  {
    _airportList = airports;
    _metarList = airports;

    // remove all entries from metarList with invalid or dead METARs
    for(int i = _metarList.size() - 1; i >= 0; i--) {
        if(_metarList[i]->metar.needsRefresh() || _metarList[i]->metar.doesNotExist())
            _metarList.removeAt(i);
    }
    refresh();
}

void MetarModel::modelClicked(const QModelIndex& index) {
    Airport *a = _metarList[index.row()];
    if(a != 0 && Window::instance(false) != 0) {
        a->showDetailsDialog();
    }
}

void MetarModel::refresh() {
    _downloadQueue.clear();
    if(_airportList.size() > MAX_METARS) {
        // avoid hammering the server with gazillions of metar requests
        return;
    }

    foreach(Airport *airport, _airportList) {
        if(airport == 0) continue;

        if(!airport->metar.doesNotExist() && airport->metar.needsRefresh()) {
            downloadMetarFor(airport);
        } else {
            emit gotMetar(airport->label, airport->metar.encoded);
        }
    }
}

void MetarModel::downloadMetarFor(Airport* airport) {
    QString location = Whazzup::instance()->metarUrl(airport->label);
    if(location.isEmpty())
        return;

    QUrl url(location);

    qDebug() << "MetarModel::downloadMetarFor()" << airport->label << url;

    _downloadQueue.insert(url, airport);
    downloadNextFromQueue();
    emit headerDataChanged(Qt::Horizontal, 0, 0);
}

void MetarModel::downloadNextFromQueue()
{
    qDebug() << "MetarModel::downloadNextFromQueue() queue.count=" << _downloadQueue.count();

    if (_downloadQueue.isEmpty())
        return;

    if (_metarReply != 0 && !_metarReply->isFinished()) {
        qDebug() << "MetarModel::downloadNextFromQueue() _metarReply still running";
        return; // we will be called via downloaded() later
    }

    _metarReply = Net::g(_downloadQueue.keys().first());
    connect(_metarReply, SIGNAL(finished()), this, SLOT(metarReplyFinished()));
}

void MetarModel::metarReplyFinished() {
    qDebug() << "MetarModel::downloaded()" << _metarReply->url();
    disconnect(_metarReply, SIGNAL(finished()), this, SLOT(metarReplyFinished()));

    if(_metarReply->error() == QNetworkReply::NoError) {
        QString line = _metarReply->readAll().trimmed();

        Airport* airport = _downloadQueue.take(_metarReply->url());
        if (airport != 0) {
            if(!line.isEmpty())
                airport->metar = Metar(line, airport->label);
            if(!airport->metar.isNull() && airport->metar.isValid())
                gotMetarFor(airport);

            emit headerDataChanged(Qt::Horizontal, 0, 0);
        }
    }
    downloadNextFromQueue();
}

void MetarModel::gotMetarFor(Airport* airport) {
    if(_airportList.contains(airport)) {
        beginResetModel();
        if(!_metarList.contains(airport))
            _metarList.append(airport);
        qDebug() << "MetarModel::gotMetarFor()" << airport->label << ":" << airport->metar.encoded;
        emit gotMetar(airport->label, airport->metar.encoded);
        endResetModel();
    }
}
