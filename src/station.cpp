#include "station.h"

Station::Station()
{

}

Station::Station(int num, double la, double lo, int ele, QString nam)
{
    number = num;
    lat = la;
    lon = lo;
    elev = ele;
    name = nam;
}

QPair<int , int> Station::getWind(int alt)
{
    return windData[alt];
}

QHash< int , QPair<int , int > > Station::getWind()
{
    return windData;
}

/*GLuint Station::getWind(int alt)
{

}*/

void Station::addWind(int alt, int dir, int speed)
{
    windData[alt] = QPair<int, int> (dir , speed);
    //qDebug() << "Station::addWind -- " << "ID:" << number << " alt:" << alt << " dir:" << dir << " speed:" << speed << " lat:" << lat << " lon:" << lon << " name:" << name;
}

void Station::addTemp(int alt, int temp)
{
    tempData[alt] = temp;
}

void Station::getWindArrow(int alt) // alt in ft
{
    renderWindStation(windData[alt].first, windData[alt].second);
    //qDebug() << "Station::getWindArrow  -- alt:" << alt;
}

void Station::renderWindStation(double deg, double knots)
{

    int n_Five = knots/5;
    if(static_cast<int>(knots)%5 != 0){
        float rest = static_cast<int>(knots)%5;
        rest = rest*2;
        rest = rest/10;
        int roundRest = qRound(rest);
        if(roundRest == 1) n_Five++ ;
    }

    int speed = n_Five*5;
    n_Five = 0;

    if(speed == 0) return;


    int n_Fifty = qFloor(speed / 50.);
    int n_Ten = qFloor((speed - n_Fifty*50) / 10.);
    n_Five = qFloor((speed - n_Fifty*50 - n_Ten*10) / 5.);


    glLineWidth( 2);
    glBegin(GL_LINES);

    //main line
    double dist = 30;
    double ideg = deg+180; //invers direction
    if(ideg >= 360) ideg -= 360;
    int ortoDeg = ideg-90; // 90° to invers direction
    if(ortoDeg < 0) ortoDeg += 360;

    QPair<double, double> begin = NavData::pointDistanceBearing(lat, lon, dist, ideg);
    QPair<double, double> end = NavData::pointDistanceBearing(lat, lon, dist, deg);

    QColor color(Settings::upperWindColor());
    glColor3f(color.redF(), color.greenF(), color.blueF());
    VERTEX(begin.first, begin.second);
    VERTEX(end.first, end.second);
    glEnd();


    int pos = 0;
    for(int drawFifty = n_Fifty; drawFifty > 0; drawFifty--)
    {                                                                                           //            B
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);       // C ->  ____/_| <- A
        QPair<double, double> c = NavData::pointDistanceBearing(lat, lon, dist -8 -pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second, 26, ortoDeg);

        glBegin(GL_TRIANGLES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        VERTEX(c.first, c.second);
        glEnd();

        pos += 18;
    }

    glLineWidth( 1);

    for(int drawTen = n_Ten; drawTen > 0; drawTen--)
    {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist -pos + 6, deg);    //             B
        QPair<double, double> c = NavData::pointDistanceBearing(lat, lon, dist -pos, deg);        //   C-> _____/.  <- A(only needed to calculate B)
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second, 26, ortoDeg);

        glBegin(GL_LINES);
        VERTEX(c.first, c.second);
        VERTEX(b.first, b.second);
        glEnd();

        pos += 8;

    }

    for(int drawFive = n_Five; drawFive > 0; drawFive--)
    {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist -pos + 3, deg);    //             B
        QPair<double, double> c = NavData::pointDistanceBearing(lat, lon, dist -pos, deg);        //   C-> _____/.  <- A(only needed to calculate B)
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second, 13, ortoDeg);

        glBegin(GL_LINES);
        VERTEX(c.first, c.second);
        VERTEX(b.first, b.second);
        glEnd();

        pos += 8;
    }
}
