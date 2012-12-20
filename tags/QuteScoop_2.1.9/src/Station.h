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
    Station() {}
    Station(int num, double lat, double lon, int elev , QString name = "");

    void setName(QString s) { name = s; }
    QString getName() const { return name; }

    double getLat() const { return lat; }
    double getLon() const { return lon; }
    int getElev() const { return elev; }

    void getWindArrow(int alt) const;

    void addWind(int alt, int dir, int speed);
    void addTemp(int alt, int temp);

    QPair<int, int> getWind(int alt) const;
    QHash< int , QPair < int , int > > getWind() const;

private:
    void renderWindStation(double deg, double knots) const;

    QString name;
    int number, elev;
    double lat, lon;

    QHash< int, QPair < int, int > > windData;  // < alt(feet) , < dir, speed >>
    QHash< int, int> tempData ;// <alt(feet), temp(degree C)>
};

#endif // STATION_H
