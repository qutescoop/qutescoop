#include "Station.h"

Station::Station(int number, double lat, double lon, int elev, QString name) :
    name(name),
    number(number),
    elev(elev),
    lat(lat), lon(lon)
{
}

QPair<int , int> Station::getWind(int alt) const {
    return windData[alt];
}

QHash< int , QPair<int , int > > Station::getWind() const {
    return windData;
}

void Station::addWind(int alt, int dir, int speed) {
    windData[alt] = QPair<int, int> (dir , speed);
    //qDebug() << "Station::addWind -- " << "ID:" << number << " alt:" << alt << " dir:" << dir << " speed:" << speed << " lat:" << lat << " lon:" << lon << " name:" << name;
}

void Station::addTemp(int alt, int temp) {
    tempData[alt] = temp;
}

// alt in ft
void Station::getWindArrow(int alt) const {
    renderWindStation(windData[alt].first, windData[alt].second);
    //qDebug() << "Station::getWindArrow  -- alt:" << alt;
}

void Station::renderWindStation(double deg, double knots) const {
    int n_Five = knots/5;
    if(static_cast<int>(knots) % 5 != 0){
        float rest = static_cast<int>(knots) % 5;
        rest = rest*2;
        rest = rest/10;
        int roundRest = qRound(rest);
        if(roundRest == 1) n_Five++ ;
    }

    int speed = n_Five * 5;
    n_Five = 0;

    if(speed == 0) return;

    quint8 dist = 30;
    int n_Fifty = qFloor(speed / 50.);
    int n_Ten = qFloor((speed - n_Fifty * 50) / 10.);
    n_Five = qFloor((speed - n_Fifty * 50 - n_Ten * 10) / 5.);

    //main line
    QPair<double, double> begin = NavData::pointDistanceBearing(lat, lon, dist, deg + 180);
    QPair<double, double> end = NavData::pointDistanceBearing(lat, lon, dist, deg);

    // background
    QPair<double, double> beginBg = NavData::pointDistanceBearing(lat, lon, dist + 0.5, deg + 180);
    QPair<double, double> endBg = NavData::pointDistanceBearing(lat, lon, dist + 0.5, deg);

    glColor3f(.1, .1, .1);
    glLineWidth(4);
    glBegin(GL_LINES);
    VERTEX(beginBg.first, beginBg.second);
    VERTEX(endBg.first, endBg.second);
    glEnd();

    int pos = 0;
    for(int i = 0; i < n_Fifty; i++) {
        QPair<double, double> lineA = NavData::pointDistanceBearing(lat, lon, dist - pos + .5 , deg);
        QPair<double, double> lineB = NavData::pointDistanceBearing(lat, lon, dist - pos - 6.5, deg);
        QPair<double, double> out = NavData::pointDistanceBearing(lineA.first, lineA.second, 30, deg - 74);
        pos += 10;

        glBegin(GL_TRIANGLES);
        VERTEX(lineA.first, lineA.second);
        VERTEX(out.first, out.second);
        VERTEX(lineB.first, lineB.second);
        glEnd();
    }

    for(int i = 0; i < n_Ten; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second, 26, deg - 70);
        pos += 7;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // set apart if only 5kts
    if (n_Fifty + n_Ten == 0)
        pos = 7;
    for(int i = 0; i < n_Five; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second, 13, deg - 70);
        pos += 7;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // foreground
    QColor color(Settings::upperWindColor());
    glColor3f(color.redF(), color.greenF(), color.blueF());
    glLineWidth(2);
    glBegin(GL_LINES);
    VERTEX(begin.first, begin.second);
    VERTEX(end.first, end.second);
    glEnd();

    pos = 0;
    for(int i = 0; i < n_Fifty; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> c = NavData::pointDistanceBearing(lat, lon, dist - pos - 6, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second, 26, deg - 74);
        pos += 10;

        glBegin(GL_TRIANGLES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        VERTEX(c.first, c.second);
        glEnd();
    }

    for(int i = 0; i < n_Ten; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second, 26, deg - 70);
        pos += 7;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // set apart if only 5kts
    if (n_Fifty + n_Ten == 0)
        pos = 7;
    for(int i = 0; i < n_Five; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second, 13, deg - 70);
        pos += 7;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }
}
