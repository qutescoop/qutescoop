/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Airport.h"

#include "Tessellator.h"
#include "helpers.h"
#include "AirportDetails.h"
#include "Settings.h"

Airport::Airport() :
        showFlightLines(false),
        _appDisplayList(0),
        _twrDisplayList(0), _gndDisplayList(0), _delDisplayList(0) {
    resetWhazzupStatus();
}
Airport::Airport(const QStringList& list) :
        showFlightLines(false),
        _appDisplayList(0),
        _twrDisplayList(0), _gndDisplayList(0), _delDisplayList(0) {
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
    atises.clear();
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

const GLuint& Airport::appDisplayList() {
    if(_appDisplayList != 0)
        return _appDisplayList;

    QColor middleColor = Settings::appCenterColor();
    QColor marginColor = Settings::appMarginColor();
    QColor borderColor = Settings::appBorderLineColor();
    GLfloat borderLineWidth = Settings::appBorderLineWidth();

    _appDisplayList = glGenLists(1);
    glNewList(_appDisplayList, GL_COMPILE);

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(middleColor.redF(), middleColor.greenF(), middleColor.blueF(), middleColor.alphaF());
    VERTEX(lat, lon);
    glColor4f(marginColor.redF(), marginColor.greenF(), marginColor.blueF(), marginColor.alphaF());
    GLdouble circle_distort = qCos(lat * Pi180);
    GLfloat deltaLon = Nm2Deg(40);
    GLfloat deltaLat = circle_distort * deltaLon;

    for(int i = 0; i <= 360; i += 10) {
        VERTEX(
            lat + deltaLat * qCos(i * Pi180),
            lon + deltaLon * qSin(i * Pi180)
        );
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    glLineWidth(borderLineWidth);
    glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
    for(int i = 0; i <= 360; i += 10) {
        VERTEX(
            lat + deltaLat * qCos(i * Pi180),
            lon + deltaLon * qSin(i * Pi180)
        );
    }
    glEnd();

    glEndList();

    return _appDisplayList;
}

const GLuint& Airport::twrDisplayList() {
    if(_twrDisplayList != 0)
        return _twrDisplayList;

    QColor middleColor = Settings::twrCenterColor();
    QColor marginColor = Settings::twrMarginColor();
    // @todo: using APP border currently
    QColor borderColor = Settings::appBorderLineColor();
    GLfloat borderWidth = Settings::appBorderLineWidth();

    _twrDisplayList = glGenLists(1);
    glNewList(_twrDisplayList, GL_COMPILE);

    GLdouble circle_distort = qCos(lat * Pi180);
    GLfloat deltaLon = Nm2Deg(22);
    GLfloat deltaLat = circle_distort * deltaLon;

    QList<QPointF> points;
    for(int i = 0; i <= 360; i += 10) {
        points.append(
            QPointF(
                lon + deltaLon * qSin(i * Pi180),
                lat + deltaLat * qCos(i * Pi180)
            )
        );
    }

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(middleColor.redF(), middleColor.greenF(), middleColor.blueF(), middleColor.alphaF());
    VERTEX(lat, lon);
    glColor4f(marginColor.redF(), marginColor.greenF(), marginColor.blueF(), marginColor.alphaF());
    for(int i = 0; i < points.size(); i++) {
        VERTEX(points[i].y(), points[i].x());
    }
    glEnd();

    if (Settings::appBorderLineWidth() > 0.) {
        glBegin(GL_LINE_LOOP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        glLineWidth(borderWidth);
        for(int i = 0; i < points.size(); i++) {
            VERTEX(points[i].y(), points[i].x());
        }
        glEnd();
    }

    glEndList();

    return _twrDisplayList;
}

const GLuint& Airport::gndDisplayList() {
    if(_gndDisplayList != 0)
        return _gndDisplayList;

    QColor fillColor = Settings::gndFillColor();
    QColor borderColor = Settings::gndBorderLineColor();

    _gndDisplayList = glGenLists(1);
    glNewList(_gndDisplayList, GL_COMPILE);

    GLfloat circle_distort = qCos(lat * Pi180);
    GLfloat innerDeltaLon = Nm2Deg(4);
    GLfloat outerDeltaLon = Nm2Deg(18);
    GLfloat innerDeltaLat = circle_distort * innerDeltaLon;
    GLfloat outerDeltaLat = circle_distort * outerDeltaLon;

    glBegin(GL_POLYGON);
    glColor4f(fillColor.redF(), fillColor.greenF(), fillColor.blueF(), fillColor.alphaF());
    // first point is in center to avoid problems with the concave shape
    VERTEX(lat, lon);

    // draw a star shape
    VERTEX(lat + outerDeltaLat, lon);
    VERTEX(lat + innerDeltaLat, lon + innerDeltaLon);
    VERTEX(lat, lon + outerDeltaLon);
    VERTEX(lat - innerDeltaLat, lon + innerDeltaLon);
    VERTEX(lat - outerDeltaLat, lon);
    VERTEX(lat - innerDeltaLat, lon - innerDeltaLon);
    VERTEX(lat, lon - outerDeltaLon);
    VERTEX(lat + innerDeltaLat, lon - innerDeltaLon);
    VERTEX(lat + outerDeltaLat, lon);
    glEnd();

    if (Settings::gndBorderLineWidth() > 0.) {
        glBegin(GL_LINE_STRIP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        glLineWidth(Settings::gndBorderLineWidth());
        VERTEX(lat + outerDeltaLat, lon);
        VERTEX(lat + innerDeltaLat, lon + innerDeltaLon);
        VERTEX(lat, lon + outerDeltaLon);
        VERTEX(lat - innerDeltaLat, lon + innerDeltaLon);
        VERTEX(lat - outerDeltaLat, lon);
        VERTEX(lat - innerDeltaLat, lon - innerDeltaLon);
        VERTEX(lat, lon - outerDeltaLon);
        VERTEX(lat + innerDeltaLat, lon - innerDeltaLon);
        VERTEX(lat + outerDeltaLat, lon);
        glEnd();
    }
    glEndList();
    return _gndDisplayList;
}

const GLuint& Airport::delDisplayList() {
    if(_delDisplayList != 0)
        return _delDisplayList;

    // @todo: using GND colors currently
    QColor fillColor = Settings::gndFillColor();
    QColor borderColor = Settings::gndBorderLineColor();
    GLfloat borderLineWidth = Settings::gndBorderLineWidth();
    _delDisplayList = glGenLists(1);
    glNewList(_delDisplayList, GL_COMPILE);

    GLfloat circle_distort = qCos(lat * Pi180);
    GLfloat deltaLon = Nm2Deg(14);
    GLfloat deltaLat = circle_distort * deltaLon;

    QList<QPointF> points;
    for(int i = 0; i <= 360; i += 10) {
        points.append(
            QPointF(
                lon + deltaLon * qSin(i * Pi180),
                lat + deltaLat * qCos(i * Pi180)
            )
        );
    }

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(fillColor.redF(), fillColor.greenF(), fillColor.blueF(), fillColor.alphaF());
    VERTEX(lat, lon);
    for(int i = 0; i < points.size(); i++) {
        VERTEX(points[i].y(), points[i].x());
    }
    glEnd();

    if (Settings::gndBorderLineWidth() > 0.) {
        glBegin(GL_LINE_LOOP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        glLineWidth(borderLineWidth);
        for(int i = 0; i < points.size(); i++) {
            VERTEX(points[i].y(), points[i].x());
        }
        glEnd();
    }

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

void Airport::addAtis(Controller* client) {
    atises.insert(client);
    active = true;
}

void Airport::showDetailsDialog() {
    AirportDetails *infoDialog = AirportDetails::instance();
    infoDialog->refresh(this);
    infoDialog->show();
    infoDialog->raise();
    infoDialog->activateWindow();
    infoDialog->setFocus();
}

QSet<Controller*> Airport::allControllers() const {
    return approaches + towers + grounds + deliveries + atises;
}

bool Airport::matches(const QRegExp& regex) const {
    return name.contains(regex)
            || city.contains(regex)
            || countryCode.contains(regex)
            || MapObject::matches(regex);
}

QString Airport::prettyName() const
{
    if (name.contains(city)) {
        return name;
    }
    return city + " " + name;
}

QString Airport::mapLabel() const {
    QString result = label;
    if(!this->active || numFilteredArrivals + numFilteredDepartures == 0)
        return label;

    if (Settings::filterTraffic())
        result += QString(" %1/%2").arg(numFilteredArrivals? QString::number(numFilteredArrivals): "-")
                  .arg(numFilteredDepartures? QString::number(numFilteredDepartures): "-");
    else
        result += QString(" %1/%2")
                .arg(
                    arrivals.isEmpty()? "-": QString::number(arrivals.size()),
                    departures.isEmpty()? "-": QString::number(departures.size())
                );
    return result;
}

QString Airport::toolTip() const {
    return QString("%1 (%2, %3)").arg(label, prettyName(), countryCode);
}
