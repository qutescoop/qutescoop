#include "Station.h"

Station::Station(double lat, double lon,
                 int elev, QString icao, QString label) :
    MapObject(), elev(elev), icao(icao) {
    this->label = label;
    this->lat = lat;
    this->lon = lon;
}

QString Station::mapLabel() const {
    if (temp.keys().contains(Settings::sondeAlt_1k() * 1000))
        return QString::fromUtf8("%1/%2°C")
                .arg(temp[Settings::sondeAlt_1k() * 1000])
                .arg(spread[Settings::sondeAlt_1k() * 1000], 0, 'f', 0);
    return "";
}

/**
  @param alt in ft
  @param secondary not a primary wind station
**/
void Station::windArrow(int alt, bool secondary) const {
    const double deg = wind[alt].first;
    const double kts = wind[alt].second;

    if(kts == 0) return;

    const quint8 n_Fifty = qFloor(kts / 50.);
    const quint8 n_Ten =   qFloor((kts - n_Fifty * 50) / 10.);
    const quint8 n_Five =  qFloor((kts - n_Fifty * 50 - n_Ten * 10) / 5.);

    const quint8 dist = ((n_Five + n_Ten) * .6 + n_Fifty + 2.)
            * Settings::windArrowSize() * .5;

    //main line
    QPair<double, double> begin = NavData::pointDistanceBearing(lat, lon, dist, deg + 180);
    QPair<double, double> end = NavData::pointDistanceBearing(lat, lon, dist, deg);

    // background
    QPair<double, double> beginBg = NavData::pointDistanceBearing(lat, lon,
            dist + 0.5, deg + 180);
    QPair<double, double> endBg = NavData::pointDistanceBearing(lat, lon,
            dist + 0.5, deg);

    glColor3f(.1, .1, .1);
    glLineWidth(4);
    glBegin(GL_LINES);
    VERTEX(beginBg.first, beginBg.second);
    VERTEX(endBg.first, endBg.second);
    glEnd();

    glLineWidth(2);
    int pos = 0;
    for(int i = 0; i < n_Fifty; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> c = NavData::pointDistanceBearing(lat, lon,
                dist - pos - Settings::windArrowSize() * .7, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                Settings::windArrowSize() * 1.8, deg - 74);
        pos += Settings::windArrowSize();

        glBegin(GL_LINE_STRIP);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        VERTEX(c.first, c.second);
        glEnd();
    }

    glLineWidth(4);
    for(int i = 0; i < n_Ten; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                Settings::windArrowSize() * 1.8, deg - 70);
        pos += Settings::windArrowSize() * .6;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // set apart if only 5kts
    if (n_Fifty + n_Ten == 0)
        pos = Settings::windArrowSize() * .6;
    for(int i = 0; i < n_Five; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                Settings::windArrowSize() * .9, deg - 70);
        pos += Settings::windArrowSize() * .6;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // foreground
    QColor color(Settings::windColor());
    if (secondary)
        color = color.darker();
    glColor3f(color.redF(), color.greenF(), color.blueF());
    glLineWidth(2);
    glBegin(GL_LINES);
    VERTEX(begin.first, begin.second);
    VERTEX(end.first, end.second);
    glEnd();

    pos = 0;
    for(int i = 0; i < n_Fifty; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> c = NavData::pointDistanceBearing(lat, lon,
                    dist - pos - Settings::windArrowSize() * .7, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                    Settings::windArrowSize() * 1.7, deg - 74);
        pos += Settings::windArrowSize();

        glBegin(GL_TRIANGLES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        VERTEX(c.first, c.second);
        glEnd();
    }

    for(int i = 0; i < n_Ten; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                    Settings::windArrowSize() * 1.7, deg - 70);
        pos += Settings::windArrowSize() * .6;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }

    // set apart if only 5kts
    if (n_Fifty + n_Ten == 0)
        pos = Settings::windArrowSize() * .6;
    for(int i = 0; i < n_Five; i++) {
        QPair<double, double> a = NavData::pointDistanceBearing(lat, lon, dist - pos, deg);
        QPair<double, double> b = NavData::pointDistanceBearing(a.first, a.second,
                    Settings::windArrowSize() * .85, deg - 70);
        pos += Settings::windArrowSize() * .6;

        glBegin(GL_LINES);
        VERTEX(a.first, a.second);
        VERTEX(b.first, b.second);
        glEnd();
    }
}
