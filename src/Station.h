#ifndef STATION_H
#define STATION_H

#include <QHash>
#include <QPair>

#include "NavData.h"
#include "helpers.h"
#include "Settings.h"

class Station
{
public:
    Station();
    Station( int num, double la, double lo, int ele , QString nam="none");

    void setName(QString s) { name = s; }
    QString getName() { return name; }

    int getNumber() { return number; }
    double getLat() { return lat; }
    double getLon() { return lon; }
    int getElev() { return elev; }

    void getWindArrow(int alt);

    void addWind(int alt, int dir, int speed);
    void addTemp(int alt, int temp);

    QPair<int, int> getWind(int alt);
    QHash< int , QPair < int , int > > getWind();
    //GLunit getWindArror(int alt);

private:
    void renderWindStation(double deg, double knots);

    QString name;
    int number, elev;
    double lat, lon;

    QHash< int, QPair < int, int > > windData;  // < alt(feet) , < dir, speed >>
    QHash< int, int> tempData ;// <alt(feet), temp(degree C)>

};

#endif // STATION_H
