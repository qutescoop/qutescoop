// TODO: remove

#include "SondeData.h"
#include "Window.h"
#include "Net.h"
#include "GuiMessage.h"
#include "Settings.h"
#include "FileReader.h"

SondeData *windDataInstance = 0;
SondeData *SondeData::instance(bool createIfNoInstance) {
    if(windDataInstance == 0 && createIfNoInstance)
        windDataInstance = new SondeData();
    return windDataInstance;
}

SondeData::SondeData(QObject *parent) :
        QObject(parent) {
}

SondeData::~SondeData() {
    foreach(Station *s, stationList)
        delete s;
}

void SondeData::decodeData() {
    qDebug() << "WindData::decodeData()";
    // showing an hourglass cursor during long operations
    GuiMessages::progress("decodesonde",
                          "Decoding sonde (wind/temperature/dewpoint) data: processing stations...");
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

    if(stationList.isEmpty()) { // Load station data from file
        FileReader file(Settings::dataDirectory("data/station.dat"));
        while(!file.atEnd()) {
            QString rawLine =  file.nextLine();
            if(rawLine.isEmpty() || rawLine.startsWith(";")) {
                continue;
            }

            QRegExp rx("(.{7})" // id (6 digits, our data has only 5)
                       "(.{7})" // icao (unused)
                       "(.{20})"// name (unused)
                       "(.{3})" // countrycode (unused)
                       "(.{6})" // lat
                       "(.{7})" // lon
                       "(.*)"   // elev (unused)
            );
            if (rx.exactMatch(rawLine)) {
                const uint id = rx.capturedTexts()[1].left(5).toUInt();
                const QString latRaw = rx.capturedTexts()[5];
                const double lat = (latRaw[4] == 'N'? 1.: -1.) * latRaw.left(4).toUInt() / 100.;
                const QString lonRaw = rx.capturedTexts()[6];
                const double lon = (lonRaw[5] == 'E'? 1.: -1.) * lonRaw.left(5).toUInt() / 100.;
                if (id != 0 && !qFuzzyIsNull(lat) && !qFuzzyIsNull(lon))
                    stationList[id] = new Station(lat, lon);
//                qDebug() << "WindData::decodeData read" << id
//                         << "lat:" << lat << "lon:" << lon;
            } else
                qWarning() << "SondeData::decodeData() could not parse" << rawLine;
        }
    }

    qDebug() << "WindData::decodeData()" << stationList.count() << "stations loaded";

    _stationRawData.clear();
    if(_rawData.isEmpty())
        return;
    _stationRawData = _rawData.split(QRegExp("=\\s+"));

    // infos about decoding radiosonde code see here: http://apollo.lsc.vsc.edu/classes/met1211L/raob.html#ident

    QStringList stationRawList;
    for(int i = 0; i < _stationRawData.size(); i++) {
        GuiMessages::progress("decodesonde", i, _stationRawData.size());
        qApp->processEvents(); // keep GUI responsive
        stationRawList.clear();
        stationRawList = _stationRawData.value(i-1).split(QRegExp("\\s+"));

        //qDebug() << stationRawList;

        int _mode = -1; // 0 = TTAA; 1 = TTBB; 2 =  PPBB; 3 = UNKNOWN
        if(stationRawList[0] == "TTAA") _mode = 0; // Temp + Wind
        if(stationRawList[0] == "TTBB") _mode = 1; // temp only
        if(stationRawList[0] == "PPBB") _mode = 2; // Wind
        stationRawList.removeFirst(); //remove mode

        if (_mode == 0) {
            int stationID = stationRawList[1].toInt();

            if(!stationRawList.isEmpty()) stationRawList.removeFirst();
            if(!stationRawList.isEmpty()) stationRawList.removeFirst();
            if(stationRawList.isEmpty() || stationRawList.size() < 3) {
                continue;}

            bool isLevel = false;
            while (true) {
                int alt = 0, ralt;

                switch(stationRawList[0].left(2).toInt()) {
                    case 0:
                        ralt = stationRawList[0].right(3).toInt(); //alt in meters
                        ralt = ralt * 3.28;  // alt in feet
                        alt = qRound(ralt / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 92:
                        ralt = stationRawList[0].right(3).toInt(); //alt in meters
                        ralt = ralt * 3.28;  // alt in feet
                        alt = qRound(ralt / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 85:
                        ralt = stationRawList[0].right(3).toInt(); // alt in meters
                        ralt += 1000;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 70:
                        ralt = stationRawList[0].right(3).toInt();
                        ralt += 3000;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 50:
                        ralt = stationRawList[0].right(3).toInt();
                        ralt = ralt * 10;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 40:
                        ralt = stationRawList[0].right(3).toInt();
                        ralt = ralt * 10;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 30:
                        ralt = stationRawList[0].right(3).toInt();
                        ralt = ralt * 10;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 25:
                        ralt =stationRawList[0].right(3).toInt();
                        ralt = (ralt * 10) + 10000;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 20:
                        ralt =stationRawList[0].right(3).toInt();
                        ralt = (ralt * 10) + 10000;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 15:
                        ralt =stationRawList[0].right(3).toInt();
                        ralt = (ralt * 10) + 10000;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 10:
                        ralt =stationRawList[0].right(3).toInt();
                        ralt = (ralt * 10) + 10000;
                        alt = qRound(ralt * 3.28 / 1000.) * 1000;
                        isLevel = true;
                        break;
                    case 88:
                        isLevel = false;
                        break;
                    case 77:
                        isLevel = false;
                        break;
                    default:
                        alt= -1;     //unable to identify the altitude so disable via -1
                        isLevel = false;
                        break;
                }

                if(isLevel) {
                    int dir = 0, speed = 0, temp = 0;
                    double spread = 0.;

                    if(stationRawList[2].contains("//")) {
                        stationRawList.removeFirst();       //remove the processed group
                        if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                        if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                        if(stationRawList.isEmpty() || stationRawList.size() < 3) break;
                        continue;
                    }
                    if(stationRawList[2].size() < 5
                            || stationRawList[1].size() < 5
                            || stationRawList[0].size() < 5) {
                        stationRawList.removeFirst();       //remove the processed group
                        if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                        if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                        if(stationRawList.isEmpty() || stationRawList.size() < 3) break;
                        continue;
                    }

                    if(stationRawList[1].left(3).toInt() % 2 == 0)
                        temp = stationRawList[1].left(3).toInt() / 10.f;
                    else
                        temp = - stationRawList[1].left(3).toInt() / 10.f;

                    if(stationRawList[1].right(2).toInt() > 50)
                        spread = stationRawList[1].right(2).toDouble() - 50.f;
                    else
                        spread = stationRawList[1].right(2).toDouble() / 10.f;

                    if(stationRawList.value(2).left(3).toInt() % 5 == 1) {
                        speed += 100;
                        dir -= 1;
                    }

                    speed += stationRawList[2].right(2).toInt();

                    dir += stationRawList[2].left(3).toInt();

                    if (stationList.keys().contains(stationID)) {
                        stationList[stationID]->wind[alt] = QPair<quint16, quint16>(dir, speed);
                        stationList[stationID]->temp[alt] = temp;
                        stationList[stationID]->spread[alt] = spread;
//                        qDebug() << "WindData::decodeData decode" << stationID
//                                 << stationList[stationID].getName()
//                                 << "lat" << stationList[stationID].getLat()
//                                 << "lon" << stationList[stationID].getLon()
//                                 << "alt" << alt
//                                 << "dir" << dir << "speed" << speed << "temp" << temp;
                    } else {
//                        qDebug() << "data for station" << stationID << "available but we have no location"
//                                 << "from 'data/station.dat'";
                    }
                }

                stationRawList.removeFirst();       //remove the processed group
                if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                if(!stationRawList.isEmpty()) stationRawList.removeFirst();

                if(stationRawList.isEmpty() || stationRawList.size() < 3) break;
            }
        }
    }

    qDebug() << "WindData::decodeData() decoded" << _stationRawData.count() << "records";

    invalidateWindLists();

    // update GL if already created
    if (Settings::showSonde() && Window::instance(false) != 0) {
        qDebug() << "WindData::decodeData() updating OpenGL map";
        Window::instance()->mapScreen->glWidget->updateGL();
    }

    // showing an hourglass cursor during long operations
    qApp->restoreOverrideCursor();
    GuiMessages::remove("decodesonde");
}

void SondeData::setRawData(QString data) {
    _rawData = data;
}

/**
  @param alt1k altitude in 1000's of feet
  @param secondary not a primary wind station
**/
GLuint SondeData::windArrows(int alt1k, bool secondary) {
    //qDebug() << "WindData::windArrows() alt1k=" << alt1k;
    QHash<int, GLuint> &list = (secondary? _windListSecondary: _windList);

    // use lazy-generation to speed up things
    if (!list.keys().contains(alt1k)) {
        qDebug() << "WindData::windArrows() caching alt1k=" << alt1k
                 << "secondary=" << secondary;
        list[alt1k] = glGenLists(1);
        glNewList(list[alt1k], GL_COMPILE);
        foreach(const Station *s, stationList) {
            s->windArrow(alt1k * 1000, secondary);
//            qDebug() << "ID: " << s.name() << " alt1k:" << alt1k;
        }
        glEndList();
    }
    return list[alt1k];
}

void SondeData::invalidateWindLists() {
    qDebug() << "WindData::refreshLists()";
    // delete OpenGL list to not saturate memory
    foreach(const GLuint i, _windList)
        glDeleteLists(i, 1);
    _windList.clear();
    foreach(const GLuint i, _windListSecondary)
        glDeleteLists(i, 1);
    _windListSecondary.clear();
}

void SondeData::load() {
    if(_replySondeData != 0) _replySondeData = 0;

    QUrl url(Settings::sondeUrl());
    _replySondeData = Net::g(url);
    connect(_replySondeData, &QNetworkReply::downloadProgress, this, &SondeData::sondeDataProgress);
    connect(_replySondeData, &QNetworkReply::finished , this , &SondeData::processSondeData);

    GuiMessages::progress("loadsonde",
                          QString("Downloading sonde data from %1...").arg(url.toString()));
    qDebug() << "downloadSondeData()" << url;
}

void SondeData::sondeDataProgress(qint64 prog, qint64 total) {
    GuiMessages::progress("loadsonde", prog, total);
}

void SondeData::processSondeData() {
    qDebug() << "decodeSondeData() downloaded";
    _replySondeData->deleteLater();
    GuiMessages::remove("loadsonde");
    if(_replySondeData->error() != QNetworkReply::NoError) {
        GuiMessages::criticalUserInteraction(_replySondeData->errorString(),
                                             "Sonde data download");
        return;
    }
    setRawData(_replySondeData->readAll());
    decodeData();
    emit loaded();
}

