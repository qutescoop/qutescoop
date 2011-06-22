#include "winddata.h"



WindData *windDataInstance = 0;
WindData *WindData::getInstance()
{
    if(windDataInstance == 0){
        windDataInstance = new WindData();
    }
    return windDataInstance;
}

WindData::WindData(QObject *parent) :
    QObject(parent)
{
    rawData.clear();
    stationRawData.clear();
    stationList.clear();
    mode = 3;
    status = -1;
}

void WindData::decodeData()
{
    qDebug() << "WindData::decodeData -- started";
    status = 1;
    if(stationList.isEmpty()) //Load station data from file
    {

        FileReader file(Settings::applicationDataDirectory("data/station.dat"));
        StationIDs.clear();

        while(!file.atEnd())
        {
           QString rawLine =  file.nextLine();
           QString workLine = rawLine;

           workLine.remove(6, 51);          //Station number
           workLine.resize(5);              //Stationnumbers in station.dat are 6 digits long but we get only 5
           int num = workLine.toInt();

           workLine = rawLine;              //Station name
           workLine.remove(34, 23);
           workLine.remove(0, 14);
           QString name = workLine;

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
           if(workLine[5] == 'W') east_west = -1;
           workLine.remove(5,1);
           double lon = workLine.toDouble();
           lon = lon*east_west;
           lon = lon/100;

           workLine = rawLine;              // Elev
           workLine.remove(54, 3);
           workLine.remove(0, 50);
           int elev = workLine.toInt();

           stationList[num] = Station(num, lat, lon, elev, name);
           StationIDs.append(num);
           //qDebug() << "WindData::decode -- wind stations num" << num << " lat:" << lat << " lon:" << lon;

        }
    }

    qDebug() << "WindData::decodeData -- stationdata loaded";

    stationRawData.clear();
    if(rawData.isEmpty()) return;
    stationRawData = rawData.split(QRegExp("=\\s+"));



    //infos about decoding radiosonde code see here: http://apollo.lsc.vsc.edu/classes/met1211L/raob.html#ident

    QStringList stationRawList;
    for(int i = stationRawData.size(); i > 0; i--)
    {
        stationRawList.clear();
        stationRawList = stationRawData.value(i-1).split(QRegExp("\\s+"));

        if(stationRawList[0] == "TTAA") mode = 0; // Temp + Wind
        if(stationRawList[0] == "TTBB") mode = 1; // temp only
        if(stationRawList[0] == "PPBB") mode = 2; // Wind

        if( mode == 0 )
        {

            int stationID = stationRawList[2].toInt();

            stationRawList.removeFirst();           //remove the station identification
            stationRawList.removeFirst();
            stationRawList.removeFirst();





            bool isLevel = false;
            while(1)
            {
                int alt = 0;
                int ralt;


                switch(stationRawList[0].left(2).toInt())
                {
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

                if(isLevel)
                {
                    int dir = 0;
                    int speed = 0;
                    int temp = 0;

                    if(stationRawList[2].contains("//"))
                    {
                        stationRawList.removeFirst();       //remove the processed group
                        if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                        if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                        if(stationRawList.isEmpty() || stationRawList.size() < 3) break;
                        continue;
                    }
                    if(stationRawList[2].size() < 5  || stationRawList[1].size() < 5 || stationRawList[0].size() < 5){
                        stationRawList.removeFirst();       //remove the processed group
                        if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                        if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                        if(stationRawList.isEmpty() || stationRawList.size() < 3) break;
                        continue;

                    }


                    if(stationRawList[1].left(3).toInt()%2 == 0)
                    {
                        temp = static_cast<int>(stationRawList[1].left(3).toInt()/10.f);
                    }
                    else
                    {
                        temp = static_cast<int>( 0 - stationRawList[1].left(3).toInt()/10.f);
                    }

                    if(stationRawList.value(2).left(3).toInt()%5 == 1)
                    {
                        speed += 100;
                        dir -= 1;
                    }

                    speed += stationRawList[2].right(2).toInt();

                    dir += stationRawList[2].left(3).toInt();

                    stationList[stationID].addWind(alt, dir, speed);
                    stationList[stationID].addTemp(alt, temp);
                    //qDebug() << "WindData::decode() -- ID:" << stationID << " alt:" << alt << " dir:" << dir << " speed:" << speed << " temp:" << temp;
                }

                stationRawList.removeFirst();       //remove the processed group
                if(!stationRawList.isEmpty()) stationRawList.removeFirst();
                if(!stationRawList.isEmpty()) stationRawList.removeFirst();

                if(stationRawList.isEmpty() || stationRawList.size() < 3) break;


            }
        }
    }

    qDebug() << "WindData::decodeData  -- radiosondedata decoded";

    for(int i = 1; i <= 40 ; i++)
    {
        WindList.append( createWindArrowList(i*1000));
    }
    //createWindArrowList(10000);


    qDebug() << "WindData::decodeData  -- finished";
    status = 0;
}

void WindData::setRawData(QString data)
{
    rawData = data;
}

int WindData::round(int a, double b)
{
    double temp = static_cast<double>(a);
    temp = temp/b;
    temp += 0.5;
    int result = static_cast<int>(temp);
    result = result*b;

    return result;
}

GLuint WindData::createWindArrowList(int alt) // alt in ft
{
    //GLuint result;
    result = 0;

    if (result == 0)
        result = glGenLists(1);

    glNewList(result, GL_COMPILE);

    for(int i = StationIDs.size(); i > 0; i--)
    {
        stationList[StationIDs[i-1]].getWindArrow(alt);
        //qDebug() << "ID: " << StationIDs[i-1] << " alt:" << alt;
    }
    glEndList();

    return result;
}

GLuint WindData::getWindArrows(int alt) // alt in 1000 ft
{
    //qDebug() << "WindData::getWindArrows -- alt:" << alt;
    if(alt < 0 || alt > 40) return 0;
    return WindList.value(alt);

}






