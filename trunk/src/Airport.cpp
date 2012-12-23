/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Airport.h"

#include "Tessellator.h"
#include "helpers.h"
#include "AirportDetails.h"
#include "Settings.h"

Airport::Airport() {
    _appDisplayList = 0;
    _appBorderDisplayList = 0;
    _twrDisplayList = 0;
    _gndDisplayList = 0;
    _delDisplayList = 0;
    showFlightLines = false;

    resetWhazzupStatus();
}

Airport::Airport(const QStringList& list) {
    _appDisplayList = 0;
    _appBorderDisplayList = 0;
    _twrDisplayList = 0;
    _gndDisplayList = 0;
    _delDisplayList = 0;
    showFlightLines = false;

    resetWhazzupStatus();

    if(list.size() != 6)
        return;

    label = list[0];
    name = list[1];
    city = list[2];
    countryCode = list[3];
    lat = list[4].toDouble();
    lon = list[5].toDouble();
}

Airport::~Airport() {
    if(_appDisplayList != 0) glDeleteLists(_appDisplayList, 1);
    if(_appBorderDisplayList != 0) glDeleteLists(_appBorderDisplayList, 1);
    if(_twrDisplayList != 0) glDeleteLists(_twrDisplayList, 1);
    if(_gndDisplayList != 0) glDeleteLists(_gndDisplayList, 1);
    if(_delDisplayList != 0) glDeleteLists(_delDisplayList, 1);
}

void Airport::resetWhazzupStatus() {
    active = false;
    towers.clear();
    approaches.clear();
    grounds.clear();
    deliveries.clear();
    arrivals.clear();
    departures.clear();
    numFilteredArrivals = 0;
    numFilteredDepartures = 0;
}

void Airport::addArrival(Pilot* client) {
    arrivals.insert(client);
    active = true;
}

void Airport::addDeparture(Pilot* client) {
    departures.insert(client);
    active = true;
}

const GLuint& Airport::appBorderDisplayList() {
    if(_appBorderDisplayList != 0)
        return _appBorderDisplayList;

    QColor borderLine = Settings::appBorderLineColor();
    _appBorderDisplayList = glGenLists(1);
    glNewList(_appBorderDisplayList, GL_COMPILE);
    glLineWidth(Settings::appBorderLineStrength());
    glBegin(GL_LINE_LOOP);
    glColor4f(borderLine.redF(), borderLine.greenF(), borderLine.blueF(), borderLine.alphaF());
    GLdouble circle_distort = qCos(lat * Pi180);
    for(int i = 0; i <= 360; i += 10) {
        double x = lat + Nm2Deg(40) * circle_distort * qCos(i * Pi180);
        double y = lon + Nm2Deg(40) * qSin(i * Pi180);
        VERTEX(x, y);
    }
    glEnd();
    glEndList();
    return _appBorderDisplayList;
}

const GLuint& Airport::appDisplayList() {
    if(_appDisplayList != 0)
        return _appDisplayList;

    _appDisplayList = glGenLists(1);
    glNewList(_appDisplayList, GL_COMPILE);

    QColor colorMiddle = Settings::appCenterColor();
    QColor colorBorder = Settings::appMarginColor();

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(colorMiddle.redF(), colorMiddle.greenF(), colorMiddle.blueF(), colorMiddle.alphaF());
    VERTEX(lat, lon);
    glColor4f(colorBorder.redF(), colorBorder.greenF(), colorBorder.blueF(), colorBorder.alphaF());
    GLdouble circle_distort = qCos(lat * Pi180);
    for(int i = 0; i <= 360; i += 10) {
        double x = lat + Nm2Deg(40) * circle_distort * qCos(i * Pi180);
        double y = lon + Nm2Deg(40) * qSin(i * Pi180);
        VERTEX(x, y);
    }
    glEnd();
    glEndList();

    return _appDisplayList;
}

const GLuint& Airport::twrDisplayList() {
    if(_twrDisplayList != 0)
        return _twrDisplayList;

    QColor colorMiddle = Settings::twrCenterColor();
    QColor colorBorder = Settings::twrMarginColor();

    // This draws a TWR controller 'small filled circle' at x, y coordinates
    _twrDisplayList = glGenLists(1);
    glNewList(_twrDisplayList, GL_COMPILE);

    GLdouble circle_distort = qCos(lat * Pi180);

    QList<QPair<double, double> > points;
    for(int i = 0; i <= 360; i += 20) {
        double x = lat + Nm2Deg(22) * circle_distort * qCos(i * Pi180);
        double y = lon + Nm2Deg(22) * qSin(i * Pi180);
        points.append(QPair<double, double>(x, y));
    }

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(colorMiddle.redF(), colorMiddle.greenF(), colorMiddle.blueF(), colorMiddle.alphaF());
    VERTEX(lat, lon);
    glColor4f(colorBorder.redF(), colorBorder.greenF(), colorBorder.blueF(), colorBorder.alphaF());
    for(int i = 0; i < points.size(); i++) {
        VERTEX(points[i].first, points[i].second);
    }
    glEnd();
    glEndList();

    return _twrDisplayList;
}

const GLuint& Airport::gndDisplayList() {
    if(_gndDisplayList != 0)
        return _gndDisplayList;

    _gndDisplayList = glGenLists(1);
    glNewList(_gndDisplayList, GL_COMPILE);

    GLdouble circle_distort = qCos(lat * Pi180);
    GLdouble s1 = circle_distort * 0.07;
    GLdouble s3 = circle_distort * 0.25;

    QColor color = Settings::gndFillColor();

    glBegin(GL_POLYGON);
        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        // first point is in center to avoid problems with the concave shape
        VERTEX(lat, lon);

        // draw a star shape
        VERTEX(lat + s3, lon);
        VERTEX(lat + s1, lon + 0.07);
        VERTEX(lat,      lon + 0.25);
        VERTEX(lat - s1, lon + 0.07);
        VERTEX(lat - s3, lon);
        VERTEX(lat - s1, lon - 0.07);
        VERTEX(lat,      lon - 0.25);
        VERTEX(lat + s1, lon - 0.07);
        VERTEX(lat + s3, lon);
    glEnd();

    if (Settings::gndBorderLineStrength() > 0.) {
        color = Settings::gndBorderLineColor();
        glBegin(GL_LINE_STRIP);
            // draw the border for the star
            glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
            glLineWidth(Settings::gndBorderLineStrength());
            VERTEX(lat + s3, lon);
            VERTEX(lat + s1, lon + 0.07);
            VERTEX(lat,      lon + 0.25);
            VERTEX(lat - s1, lon + 0.07);
            VERTEX(lat - s3, lon);
            VERTEX(lat - s1, lon - 0.07);
            VERTEX(lat,      lon - 0.25);
            VERTEX(lat + s1, lon - 0.07);
            VERTEX(lat + s3, lon);
        glEnd();
    }
    glEndList();
    return _gndDisplayList;
}

const GLuint& Airport::delDisplayList() {
    if(_delDisplayList != 0)
        return _delDisplayList;

    QColor color = Settings::gndFillColor();
    QColor borderLine = Settings::gndBorderLineColor();

    // This draws a DEL controller 'small filled circle' at x, y coordinates
    _delDisplayList = glGenLists(1);
    glNewList(_delDisplayList, GL_COMPILE);

    GLdouble circle_distort = qCos(lat * Pi180);

    QList<DoublePair> points;
    for(int i = 0; i <= 360; i += 20) {
        double x = lat + Nm2Deg(8) * circle_distort * qCos(i * Pi180);
        double y = lon + Nm2Deg(8) * qSin(i * Pi180);
        points.append(DoublePair(x, y));
    }

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    VERTEX(lat, lon);
    foreach(const DoublePair &p, points)
        VERTEX(p.first, p.second);
    glEnd();
    // Border Line
    glLineWidth(Settings::gndBorderLineStrength());
    glBegin(GL_LINE_LOOP);
    glColor4f(borderLine.redF(), borderLine.greenF(), borderLine.blueF(), borderLine.alphaF());
    foreach(const DoublePair &p, points)
        VERTEX(p.first, p.second);
    glEnd();
    glEndList();

    return _delDisplayList;
}

void Airport::addApproach(Controller* client) {
    approaches.insert(client);
    active = true;
}

void Airport::addTower(Controller* client) {
    towers.insert(client);
    active = true;
}

void Airport::addGround(Controller* client) {
    grounds.insert(client);
    active = true;
}

void Airport::addDelivery(Controller* client) {
    deliveries.insert(client);
    active = true;
}


void Airport::showDetailsDialog() {
    AirportDetails *infoDialog = AirportDetails::instance(true);
    infoDialog->refresh(this);
    infoDialog->show();
    infoDialog->raise();
    infoDialog->activateWindow();
    infoDialog->setFocus();
}

QSet<Controller*> Airport::allControllers() const {
    return approaches + towers + grounds + deliveries;
}

bool Airport::matches(const QRegExp& regex) const {
    if (name.contains(regex)) return true;
    if (city.contains(regex)) return true;
    if (countryCode.contains(regex)) return true;
    return MapObject::matches(regex);
}

QString Airport::mapLabel() const {
    QString result = label;
    if(!this->active || numFilteredArrivals + numFilteredDepartures == 0)
        return label;

    if (Settings::filterTraffic())
        result += QString(" %1/%2").arg(numFilteredArrivals? QString::number(numFilteredArrivals): "-")
                  .arg(numFilteredDepartures? QString::number(numFilteredDepartures): "-");
    else
        result += QString(" %1/%2").arg(arrivals.isEmpty()? "-": QString::number(arrivals.size()))
                  .arg(departures.isEmpty()? "-": QString::number(departures.size()));
    return result;
}

QString Airport::toolTip() const {
    return QString("%1 %2 (%3, %4)").arg(label).arg(name).arg(city).arg(countryCode);
}
