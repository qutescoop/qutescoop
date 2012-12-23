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
        Station(int num, double _lat, double _lon, int _elev , QString _name = "");

        void setName(QString s) { _name = s; }
        QString name() const { return _name; }

        double lat() const { return _lat; }
        double lon() const { return _lon; }
        int elev() const { return _elev; }

        void windArrow(int alt) const;

        void addWind(int alt, int dir, int speed);
        void addTemp(int alt, int temp);

        QPair<int, int> wind(int alt) const;
        QHash< int , QPair < int , int > > wind() const;

    private:
        void renderWindStation(double deg, double knots) const;

        QString _name;
        int _number, _elev;
        double _lat, _lon;

        QHash< int, QPair < int, int > > _windData;  // < alt(feet) , < dir, speed >>
        QHash< int, int> _tempData ;// <alt(feet), temp(degree C)>
};

#endif // STATION_H
