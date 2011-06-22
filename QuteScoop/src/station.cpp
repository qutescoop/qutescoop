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

    //qDebug() << "renderWind ID: " << number << " deg:" << deg << " dir:" << knots;
    if(knots <= 0) return;
    glLineWidth( 2);
    glBegin(GL_LINES);
    double dist = knots*2;
    double adist = qSqrt(2*(qPow(knots *0.7,2)));
    double ideg = deg+180; //inverste direction
    if(ideg >= 360) ideg -= 360;
    double ardeg = ideg-45;
    if(ardeg < 0) ardeg += 360;
    double aldeg = ideg+45;
    if(aldeg >= 360) aldeg -= 360;

    QPair<double, double> begin = NavData::pointDistanceBearing(lat, lon, dist, ideg);
    QPair<double, double> end = NavData::pointDistanceBearing(lat, lon, dist, deg);
    QPair<double, double> arrowRight = NavData::pointDistanceBearing(end.first, end.second, adist, ardeg);
    QPair<double, double> arrowLeft = NavData::pointDistanceBearing(end.first, end.second, adist , aldeg);


    //glLineWidth( 2);
    //glBegin(GL_LINES);  //main line

    QColor color(QColor::fromRgb(136, 255, 134));
    glColor3f(color.redF(), color.greenF(), color.blueF());
    VERTEX(begin.first, begin.second);
    VERTEX(end.first, end.second);


    //right arrow line
    //qglColor(QColor::fromRgb(136, 255, 134));
    VERTEX(end.first, end.second);
    VERTEX(arrowRight.first, arrowRight.second);


    //left arrow line
    //qglColor(QColor::fromRgb(136, 255, 134));
    VERTEX(end.first, end.second);
    VERTEX(arrowLeft.first, arrowLeft.second);
    glEnd();

}
