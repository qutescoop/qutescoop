/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Airport.h"

#include "helpers.h"
#include "AirportDetails.h"
#include "Settings.h"
#include "NavData.h"

Airport::Airport(const QStringList& list, unsigned int debugLineNumber) :
        showRoutes(false),
        _appDisplayList(0),
        _twrDisplayList(0), _gndDisplayList(0), _delDisplayList(0) {
    resetWhazzupStatus();

    if(list.size() != 6) {
        auto msg = QString("While processing line #%1 '%2' from data/airports.dat: Found %3 fields, expected exactly 6.")
                       .arg(debugLineNumber).arg(list.join(':')).arg(list.size());
        qCritical() << "Airport::Airport()" << msg;
        QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
        exit(EXIT_FAILURE);
    }

    label = list[0];
    name = list[1];
    city = list[2];
    countryCode = list[3];

    if (countryCode != "" && NavData::instance()->countryCodes.value(countryCode, "") == "") {
        auto msg = QString("While processing line #%1 from data/airports.dat: Could not find country '%2' for airport %3 (%4, %5) in data/countryCodes.dat.")
                       .arg(debugLineNumber)
                       .arg(countryCode, label, name, city);
        qCritical() << "Airport::Airport()" << msg;
        QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
        exit(EXIT_FAILURE);
    }

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

    _appDisplayList = glGenLists(1);
    glNewList(_appDisplayList, GL_COMPILE);
    appGl(
        Settings::appCenterColor(),
        Settings::appMarginColor(),
        Settings::appBorderLineColor(),
        Settings::appBorderLineWidth()
    );
    glEndList();

    return _appDisplayList;
}

void Airport::appGl(const QColor &middleColor, const QColor &marginColor, const QColor &borderColor, const GLfloat &borderLineWidth) const {
    auto otherAirportsOfAppControllers = QSet<Airport*>();
    foreach(auto *approach, approaches) {
        foreach(auto *airport, approach->airports()) {
            if (airport != this) {
                otherAirportsOfAppControllers.insert(airport);
            }
        }
    }

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(middleColor.redF(), middleColor.greenF(), middleColor.blueF(), middleColor.alphaF());
    VERTEX(lat, lon);
    for(short int i = 0; i <= 360; i += 10) {
        auto _p = NavData::pointDistanceBearing(lat, lon, Airport::symbologyAppRadius_nm, i);

        short int airportsClose = 0;
        foreach(auto *a, otherAirportsOfAppControllers) {
            auto _dist = NavData::distance(_p.first, _p.second, a->lat, a->lon);

            airportsClose += _dist < Airport::symbologyAppRadius_nm;
        }

        if (airportsClose > 0) {
            // reduce opacity in overlap areas - https://github.com/qutescoop/qutescoop/issues/211
            // (this is still a TRIANGLE_FAN, so it has the potential to be a bit meh...)
            glColor4f(marginColor.redF(), marginColor.greenF(), marginColor.blueF(), marginColor.alphaF() / (airportsClose + 1));
        } else {
            glColor4f(marginColor.redF(), marginColor.greenF(), marginColor.blueF(), marginColor.alphaF());
        }

        VERTEX(_p.first,_p.second);
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    glLineWidth(borderLineWidth);
    glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
    for(short int i = 0; i <= 360; i += 1) {
        auto _p = NavData::pointDistanceBearing(lat, lon, Airport::symbologyAppRadius_nm, i);

        short int airportsClose = 0;
        foreach(auto *a, otherAirportsOfAppControllers) {
            auto _dist = NavData::distance(_p.first, _p.second, a->lat, a->lon);

            airportsClose += _dist < Airport::symbologyAppRadius_nm;
        }

        if (airportsClose > 0) {
            // hide border line on overlap - https://github.com/qutescoop/qutescoop/issues/211
            glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), 0.);
        } else {
            glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        }

        VERTEX(_p.first,_p.second);
    }
    glEnd();
}


const GLuint& Airport::twrDisplayList() {
    if(_twrDisplayList != 0)
        return _twrDisplayList;

    _twrDisplayList = glGenLists(1);
    glNewList(_twrDisplayList, GL_COMPILE);

    twrGl(
        Settings::twrCenterColor(),
        Settings::twrMarginColor(),
        // @todo: using APP border currently
        Settings::appBorderLineColor(),
        Settings::appBorderLineWidth()
    );

    glEndList();

    return _twrDisplayList;
}


void Airport::twrGl(const QColor &middleColor, const QColor &marginColor, const QColor &borderColor, const GLfloat &borderLineWidth) const {
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(middleColor.redF(), middleColor.greenF(), middleColor.blueF(), middleColor.alphaF());
    VERTEX(lat, lon);
    glColor4f(marginColor.redF(), marginColor.greenF(), marginColor.blueF(), marginColor.alphaF());
    for(int i = 0; i <= 360; i += 10) {
        auto _p = NavData::pointDistanceBearing(lat, lon, Airport::symbologyTwrRadius_nm, i);
        VERTEX(_p.first,_p.second);
    }
    glEnd();

    if (borderLineWidth > 0.) {
        glLineWidth(borderLineWidth);
        glBegin(GL_LINE_LOOP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        for(int i = 0; i <= 360; i += 10) {
            auto _p = NavData::pointDistanceBearing(lat, lon, Airport::symbologyTwrRadius_nm, i);
            VERTEX(_p.first,_p.second);
        }
        glEnd();
    }
}

const GLuint& Airport::gndDisplayList() {
    if(_gndDisplayList != 0)
        return _gndDisplayList;

    QColor fillColor = Settings::gndFillColor();
    QColor borderColor = Settings::gndBorderLineColor();

    _gndDisplayList = glGenLists(1);
    glNewList(_gndDisplayList, GL_COMPILE);

    GLfloat circle_distort = qCos(lat * Pi180);
    GLfloat innerDeltaLon = Nm2Deg(Airport::symbologyGndRadius_nm / 2.);
    GLfloat outerDeltaLon = Nm2Deg(Airport::symbologyGndRadius_nm / .7);
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
        glLineWidth(Settings::gndBorderLineWidth());
        glBegin(GL_LINE_STRIP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
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
    GLfloat deltaLon = Nm2Deg(Airport::symbologyDelRadius_nm / .7);
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
        glLineWidth(borderLineWidth);
        glBegin(GL_LINE_LOOP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
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
    return name + (city.isEmpty()? "": ", " + city);
}

QString Airport::mapLabel() const {
    QString result = label;
    if(!this->active || numFilteredArrivals + numFilteredDepartures == 0)
        return label;

    if (Settings::filterTraffic())
        result += QString(" %1/%2")
                .arg(
                    numFilteredArrivals? QString::number(numFilteredArrivals): "-",
                    numFilteredDepartures? QString::number(numFilteredDepartures): "-"
                );
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
