#ifndef STATION_H
#define STATION_H

#include <QHash>
#include <QPair>

#include "NavData.h"
#include "helpers.h"
#include "Settings.h"
#include "MapObject.h"

class Station: public MapObject {
    public:
        Station(double lat, double lon, int elev = 0, QString icao = "", QString name = "");

        void windArrow(int alt, bool secondary) const;

        virtual QString mapLabel() const;

        int elev;
        QString icao;
        QHash<int, QPair<quint16, quint16> > wind;  // < alt(feet) , < dir, speed >>
        QHash<int, qint8> temp ;// <alt(feet), temp(degree C)>
        QHash<int, double> spread ;// <alt(feet), spread(degree C)>
    private:
        void renderWindStation(double deg, double knots, bool secondary) const;
};

#endif // STATION_H
