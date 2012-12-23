#include "WindData.h"
#include "Window.h"


WindData *windDataInstance = 0;
WindData *WindData::instance() {
    if(windDataInstance == 0)
        windDataInstance = new WindData();
    return windDataInstance;
}

WindData::WindData(QObject *parent) :
    QObject(parent)
{
    _rawData.clear();
    _stationRawData.clear();
    _stationList.clear();
    _mode = 3;
    _status = -1;
}

void WindData::decodeData() {
    qDebug() << "WindData::decodeData() -- started";
    // showing an hourglass cursor during long operations
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

    _status = 1;
    if(_stationList.isEmpty()) { // Load station data from file
        FileReader file(Settings::applicationDataDirectory("data/station.dat"));
        while(!file.atEnd()) {
            QString rawLine =  file.nextLine();
            QString workLine = rawLine;

            workLine.remove(6, 51);          //Station number
            workLine.resize(5);              //Stationnumbers in station.dat are 6 digits long but we get only 5
            int num = workLine.toInt();

            workLine = rawLine;              //Station name
            workLine.remove(34, 23);
            workLine.remove(0, 14);
            QString name = workLine.trimmed();

            workLine = rawLine;              // Lat
            workLine.remove(42, 15);
            workLine.remove(0, 37);
            int north_south = 1;
            if(workLine[4] == 'S') north_south = -1;
            workLine.remove(4,1);
            double lat = workLine.toDouble();
            lat = lat*north_south;
            lat = lat/100;

            workLine = rawLine;              // Lon
            workLine.remove(49, 8);
            workLine.remove(0, 43);
            int east_west = 1;
            if(workLine[5] == 'W')
                east_west = -1;
            workLine.remove(5,1);
            double lon = workLine.toDouble();
            lon = lon * east_west;
            lon = lon / 100;

            workLine = rawLine;              // Elev
            workLine.remove(54, 3);
            workLine.remove(0, 50);
            int elev = workLine.toInt();

            if (!(num == 0 && qFuzzyIsNull(lat) && qFuzzyIsNull(lon))) {
                _stationList[num] = Station(num, lat, lon, elev, name);
//                qDebug() << "WindData::decodeData read" << num << name
//                         << "lat:" << lat << "lon:" << lon << "elev:" << elev;
            }
        }
    }

    qDebug() << "WindData::decodeData() -- stationdata loaded";

    _stationRawData.clear();
    if(_rawData.isEmpty())
        return;
    _stationRawData = _rawData.split(QRegExp("=\\s+"));

    // infos about decoding radiosonde code see here: http://apollo.lsc.vsc.edu/classes/met1211L/raob.html#ident

    QStringList stationRawList;
    for(int i = _stationRawData.size(); i > 0; i--) {
        stationRawList.clear();
        stationRawList = _stationRawData.value(i-1).split(QRegExp("\\s+"));

        //qDebug() << stationRawList;

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
                int alt = 0;
                int ralt;

                switch(stationRawList[0].left(2).toInt()) {
                    case 0:
                        ralt = stationRawList[0].right(3).toInt(); //alt in meters
                        ralt = ralt*3.28;  // alt in feet
                        alt = round(ralt, 1000);
                        isLevel = true;
                        break;
                    case 92:
                        ralt = stationRawList[0].right(3).toInt(); //alt in meters
                        ralt = ralt*3.28;  // alt in feet
                        alt = round(ralt, 1000);
                        isLevel = true;
                        break;
                    case 85:
                        ralt = stationRawList[0].right(3).toInt(); // alt in meters
                        ralt += 1000;
                        alt = round(ralt*3.28, 1000);
                        isLevel = true;
                        break;
                    case 70:
                        ralt = stationRawList[0].right(3).toInt();
                        ralt += 3000;
                        alt = round(ralt*3.28, 1000);
                        isLevel = true;
                        break;
                    case 50:
                        ralt = stationRawList[0].right(3).toInt();
                        ralt = ralt*10;
                        alt = round(ralt*3.28, 1000);
                        isLevel = true;
                        break;
                    case 40:
                        ralt = stationRawList[0].right(3).toInt();
                        ralt = ralt*10;
                        alt = round(ralt*3.28, 1000);
                        isLevel = true;
                        break;
                    case 30:
                        ralt = stationRawList[0].right(3).toInt();
                        ralt = ralt*10;
                        alt = round(ralt*3.28, 1000);
                        isLevel = true;
                        break;
                    case 25:
                        ralt =stationRawList[0].right(3).toInt();
                        ralt = (ralt*10) + 10000;
                        alt = round(ralt*3.28, 1000);
                        isLevel = true;
                        break;
                    case 20:
                        ralt =stationRawList[0].right(3).toInt();
                        ralt = (ralt*10) + 10000;
                        alt = round(ralt*3.28, 1000);
                        isLevel = true;
                        break;
                    case 15:
                        ralt =stationRawList[0].right(3).toInt();
                        ralt = (ralt*10) + 10000;
                        alt = round(ralt*3.28, 1000);
                        isLevel = true;
                        break;
                    case 10:
                        ralt =stationRawList[0].right(3).toInt();
                        ralt = (ralt*10) + 10000;
                        alt = round(ralt*3.28, 1000);
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
                    int dir = 0;
                    int speed = 0;
                    int temp = 0;

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

                    if(stationRawList[1].left(3).toInt() % 2 == 0) {
                        temp = static_cast<int>(stationRawList[1].left(3).toInt()/10.f);
                    } else {
                        temp = static_cast<int>( 0 - stationRawList[1].left(3).toInt()/10.f);
                    }

                    if(stationRawList.value(2).left(3).toInt() % 5 == 1) {
                        speed += 100;
                        dir -= 1;
                    }

                    speed += stationRawList[2].right(2).toInt();

                    dir += stationRawList[2].left(3).toInt();

                    if (_stationList.keys().contains(stationID)) {
                        _stationList[stationID].addWind(alt, dir, speed);
                        _stationList[stationID].addTemp(alt, temp);
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

    qDebug() << "WindData::decodeData  -- radiosondedata decoded";

    for(quint8 i = 1; i <= 40 ; i++)
        _windList.append(createWindArrowList(i * 1000));

    qDebug() << "WindData::decodeData  -- finished";
    _status = 0;

    // update GL if already created
    if (Window::instance(false) != 0) {
        Window::instance(true)->mapScreen->glWidget->update();
    }

    // showing an hourglass cursor during long operations
    qApp->restoreOverrideCursor();
}

void WindData::setRawData(QString data) {
    _rawData = data;
}

int WindData::round(int a, double b) {
    double temp = static_cast<double>(a);
    temp = temp/b;
    temp += 0.5;
    int result = static_cast<int>(temp);
    result = result*b;

    return result;
}

GLuint WindData::createWindArrowList(int alt) { // alt in ft
    //GLuint result;
    _result = 0;

    if (_result == 0)
        _result = glGenLists(1);

    glNewList(_result, GL_COMPILE);

    foreach(const Station s, _stationList) {
//  for(int i = stationIds.size(); i > 0; i--) {
        s.windArrow(alt);
        //qDebug() << "ID: " << StationIDs[i-1] << " alt:" << alt;
    }
    glEndList();

    return _result;
}

GLuint WindData::windArrows(int alt) { // alt in 1000 ft
    //qDebug() << "WindData::getWindArrows -- alt:" << alt;
    if(alt < 0 || alt > 40) return 0;
    return _windList.value(alt);
}

void WindData::refreshLists() {
    _windList.clear();

    for(int i = 1; i <= 40 ; i++)
        _windList.append( createWindArrowList(i*1000));
    qDebug() << "WindData::refreshLists -- done";
}
