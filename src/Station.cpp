#include "Station.h"

Station::Station(int number, double lat, double lon, int elev, QString name) :
    _name(name),
    _number(number),
    _elev(elev),
    _lat(lat), _lon(lon)
{
}

QPair<int , int> Station::wind(int alt) const {
    return _windData[alt];
}

QHash< int , QPair<int , int > > Station::wind() const {
    return _windData;
}

void Station::addWind(int alt, int dir, int speed) {
    _windData[alt] = QPair<int, int> (dir , speed);
    //qDebug() << "Station::addWind -- " << "ID:" << number << " alt:" << alt << " dir:" << dir << " speed:" << speed << " lat:" << lat << " lon:" << lon << " name:" << name;
}

void Station::addTemp(int alt, int temp) {
    _tempData[alt] = temp;
}

// alt in ft
void Station::windArrow(int alt) const {
    renderWindStation(_windData[alt].first, _windData[alt].second);
    //qDebug() << "Station::getWindArrow  -- alt:" << alt;
}

void Station::renderWindStation(double deg, double knots) const {
    int n_Five = knots/5;
    if(static_cast<int>(knots) % 5 != 0) {
        float rest = static_cast<int>(knots) % 5;
        rest = rest*2;
        rest = rest/10;
        int roundRest = qRound(rest);
        if(roundRest == 1) n_Five++ ;
    }

    int speed = n_Five * 5;
    n_Five = 0;

    if(speed == 0) return;

    quint8 dist = 2 * Settings::windSize();
    int n_Fifty = qFloor(speed / 50.);
    int n_Ten = qFloor((speed - n_Fifty * 50) / 10.);
    n_Five = qFloor((speed - n_Fifty * 50 - n_Ten * 10) / 5.);

    //main line
    QPair<double, double> begin = NavData::pointDistanceBearing(_lat, _lon, dist, deg + 180);
    QPair<double, double> end = NavData::pointDistanceBearing(_lat, _lon, dist, deg);

    // background
    QPair<double, double> beginBg = NavData::pointDistanceBearing(_lat, _lon,
            dist + 0.5, deg + 180);
    QPair<double, double> endBg = NavData::pointDistanceBearing(_lat, _lon,
            dist + 0.5, deg);

    glColor3f(.1, .1, .1);
    glLineWidth(4);
    glBegin(GL_LINES);
    VERTEX(beginBg.first, beginBg.second);
    VERTEX(endBg.first, endBg.second);
    glEnd();

    int pos = 0;
    for(int i = 0; i < n_Fifty; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(_lat, _lon, dist - pos + .5 , deg);
        QPair<double, double> c = NavData::pointDistanceBearing(_lat, _lon,
                dist - pos - Settings::windSize() * .5 - .5, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                Settings::windSize() * 2., deg - 74);
        pos += Settings::windSize() * .8;

        glBegin(GL_TRIANGLES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        VERTEX(c.first, c.second);
        glEnd();
    }

    for(int i = 0; i < n_Ten; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(_lat, _lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                Settings::windSize() * 1.7, deg - 70);
        pos += Settings::windSize() * .6;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // set apart if only 5kts
    if (n_Fifty + n_Ten == 0)
        pos = Settings::windSize() * .6;
    for(int i = 0; i < n_Five; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(_lat, _lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                Settings::windSize() * .85, deg - 70);
        pos += Settings::windSize() * .6;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // foreground
    QColor color(Settings::windColor());
    glColor3f(color.redF(), color.greenF(), color.blueF());
    glLineWidth(2);
    glBegin(GL_LINES);
    VERTEX(begin.first, begin.second);
    VERTEX(end.first, end.second);
    glEnd();

    pos = 0;
    for(int i = 0; i < n_Fifty; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(_lat, _lon, dist - pos, deg);
        QPair<double, double> c = NavData::pointDistanceBearing(_lat, _lon,
                    dist - pos - Settings::windSize() * .5, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                    Settings::windSize() * 1.7, deg - 74);
        pos += Settings::windSize() * .8;

        glBegin(GL_TRIANGLES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        VERTEX(c.first, c.second);
        glEnd();
    }

    for(int i = 0; i < n_Ten; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(_lat, _lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                    Settings::windSize() * 1.7, deg - 70);
        pos += Settings::windSize() * .6;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // set apart if only 5kts
    if (n_Fifty + n_Ten == 0)
        pos = 18;
    for(int i = 0; i < n_Five; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(_lat, _lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                    Settings::windSize() * .85, deg - 70);
        pos += Settings::windSize() * .6;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }
}
