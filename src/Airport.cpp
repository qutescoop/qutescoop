#include "Airport.h"

#include "helpers.h"
#include "NavData.h"
#include "Settings.h"
#include "dialogs/AirportDetails.h"

const QRegularExpression Airport::pdcRegExp = QRegularExpression(
    "PDC.{0,20}\\W([A-Z]{4})(\\W|$)", QRegularExpression::MultilineOption | QRegularExpression::InvertedGreedinessOption
);

const QHash<QString, std::function<QString(Airport*)> > Airport::placeholders {
    {
        "{code}", [](Airport* o)->QString {
            return o->id;
        }
    },
    {
        "{traffic}", [](Airport* o)->QString {
            return o->trafficString();
        }
    },
    {
        "{trafficUnfiltered}", [](Airport* o)->QString {
            const auto ret = o->trafficUnfilteredString();
            return ret.isEmpty()? "": "#" + o->trafficUnfilteredString();
        }
    },
    {
        "{controllers}", [](Airport* o)->QString {
            return o->controllersString();
        }
    },
    {
        "{atis}", [](Airport* o)->QString {
            return o->atisCodeString();
        }
    },
    {
        "{country}", [](Airport* o)->QString {
            return o->countryCode;
        }
    },
    {
        "{prettyName}", [](Airport* o)->QString {
            return o->prettyName();
        }
    },
    {
        "{name}", [](Airport* o)->QString {
            return o->name;
        }
    },
    {
        "{city}", [](Airport* o)->QString {
            return o->city;
        }
    },
    {
        "{frequencies}", [](Airport* o)->QString {
            return o->frequencyString();
        }
    },
    {
        "{pdc}", [](Airport* o)->QString {
            return o->pdcString();
        }
    },
    {
        "{livestream}", [](Airport* o)->QString {
            QStringList ret;
            foreach (const auto c, o->allControllers()) {
                const auto str = c->livestreamString();
                if (!str.isEmpty()) {
                    ret << str;
                }
            }

            return ret.join(" ");
        }
    },
};

Airport::Airport(const QStringList& list, unsigned int debugLineNumber)
    : MapObject(),
      _appDisplayList(0),
      _twrDisplayList(0), _gndDisplayList(0), _delDisplayList(0) {
    resetWhazzupStatus();

    if (list.size() != 6) {
        auto msg = QString("While processing line #%1 '%2' from data/airports.dat: Found %3 fields, expected exactly 6.")
            .arg(debugLineNumber).arg(list.join(':')).arg(list.size());
        qCritical() << "Airport::Airport()" << msg;
        QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
        exit(EXIT_FAILURE);
    }

    id = list[0];
    name = list[1];
    city = list[2];
    countryCode = list[3];

    if (countryCode != "" && NavData::instance()->countryCodes.value(countryCode, "") == "") {
        auto msg = QString("While processing line #%1 from data/airports.dat: Could not find country '%2' for airport %3 (%4, %5) in data/countryCodes.dat.")
            .arg(debugLineNumber)
            .arg(countryCode, id, name, city);
        qCritical() << "Airport::Airport()" << msg;
        QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
        exit(EXIT_FAILURE);
    }

    lat = list[4].toDouble();
    lon = list[5].toDouble();

    showRoutes = Settings::showRoutes();
}

Airport::~Airport() {
    if (_appDisplayList != 0) {
        glDeleteLists(_appDisplayList, 1);
    }
    if (_twrDisplayList != 0) {
        glDeleteLists(_twrDisplayList, 1);
    }
    if (_gndDisplayList != 0) {
        glDeleteLists(_gndDisplayList, 1);
    }
    if (_delDisplayList != 0) {
        glDeleteLists(_delDisplayList, 1);
    }
}

void Airport::resetWhazzupStatus() {
    active = false;

    dels.clear();
    atiss.clear();
    gnds.clear();
    twrs.clear();
    appDeps.clear();

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

uint Airport::congestion() const {
    return numFilteredArrivals + numFilteredDepartures;
}

void Airport::addController(Controller* c) {
    if (c->isAppDep()) {
        appDeps.insert(c);
    }
    if (c->isTwr()) {
        twrs.insert(c);
    }
    if (c->isGnd()) {
        gnds.insert(c);
    }
    if (c->isDel()) {
        dels.insert(c);
    }
    if (c->isAtis()) {
        atiss.insert(c);
    }
    active = true;
}

const GLuint& Airport::appDisplayList() {
    if (_appDisplayList != 0) {
        return _appDisplayList;
    }

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
    foreach (const auto* approach, appDeps) {
        foreach (const auto& airport, approach->airports()) {
            if (airport != this) {
                otherAirportsOfAppControllers.insert(airport);
            }
        }
    }

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(middleColor.redF(), middleColor.greenF(), middleColor.blueF(), middleColor.alphaF());
    VERTEX(lat, lon);
    for (short int i = 0; i <= 360; i += 10) {
        auto _p = NavData::pointDistanceBearing(lat, lon, Airport::symbologyAppRadius_nm, i);

        short int airportsClose = 0;
        foreach (const auto* a, otherAirportsOfAppControllers) {
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

        VERTEX(_p.first, _p.second);
    }
    glEnd();

    if (borderLineWidth > 0.) {
        glLineWidth(borderLineWidth);
        glBegin(GL_LINE_STRIP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        for (short int i = 0; i <= 360; i += 1) {
            auto _p = NavData::pointDistanceBearing(lat, lon, Airport::symbologyAppRadius_nm, i);

            auto isAirportsClose = false;
            foreach (const auto* a, otherAirportsOfAppControllers) {
                auto _dist = NavData::distance(_p.first, _p.second, a->lat, a->lon);

                if (_dist < Airport::symbologyAppRadius_nm) {
                    isAirportsClose = true;
                }
            }

            if (isAirportsClose) {
                // hide border line on overlap - https://github.com/qutescoop/qutescoop/issues/211
                glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), 0.);
            } else {
                glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
            }

            VERTEX(_p.first, _p.second);
        }
        glEnd();
    }
}


const GLuint& Airport::twrDisplayList() {
    if (_twrDisplayList != 0) {
        return _twrDisplayList;
    }

    _twrDisplayList = glGenLists(1);
    glNewList(_twrDisplayList, GL_COMPILE);

    twrGl(
        Settings::twrCenterColor(),
        Settings::twrMarginColor(),
        Settings::twrBorderLineColor(),
        Settings::twrBorderLineWidth()
    );

    glEndList();

    return _twrDisplayList;
}


void Airport::twrGl(const QColor &middleColor, const QColor &marginColor, const QColor &borderColor, const GLfloat &borderLineWidth) const {
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(middleColor.redF(), middleColor.greenF(), middleColor.blueF(), middleColor.alphaF());
    VERTEX(lat, lon);
    glColor4f(marginColor.redF(), marginColor.greenF(), marginColor.blueF(), marginColor.alphaF());
    for (int i = 0; i <= 360; i += 10) {
        auto _p = NavData::pointDistanceBearing(lat, lon, Airport::symbologyTwrRadius_nm, i);
        VERTEX(_p.first, _p.second);
    }
    glEnd();

    if (borderLineWidth > 0.) {
        glLineWidth(borderLineWidth);
        glBegin(GL_LINE_LOOP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        for (int i = 0; i <= 360; i += 10) {
            auto _p = NavData::pointDistanceBearing(lat, lon, Airport::symbologyTwrRadius_nm, i);
            VERTEX(_p.first, _p.second);
        }
        glEnd();
    }
}

const QString Airport::trafficString() const {
    if (Settings::filterTraffic()) {
        return trafficFilteredString();
    } else {
        return trafficUnfilteredString();
    }
}

const QString Airport::trafficFilteredString() const {
    if (!active || congestion() == 0) {
        return "";
    }

    return QString("%1/%2").arg(
        numFilteredArrivals? QString::number(numFilteredArrivals): "-",
        numFilteredDepartures? QString::number(numFilteredDepartures): "-"
    );
}

const QString Airport::trafficUnfilteredString() const {
    if (!active || congestion() == 0) {
        return "";
    }

    return QString("%1/%2").arg(
        arrivals.isEmpty()? "-": QString::number(arrivals.size()),
        departures.isEmpty()? "-": QString::number(departures.size())
    );
}

const QString Airport::controllersString() const {
    QStringList controllers;
    if (!dels.empty()) {
        controllers << "D";
    }
    if (!gnds.empty()) {
        controllers << "G";
    }
    if (!twrs.empty()) {
        controllers << "T";
    }
    if (!appDeps.empty()) {
        controllers << "A";
    }
    return controllers.join(",");
}

const QString Airport::atisCodeString() const {
    QStringList ret;
    foreach (const auto atis, atiss) {
        const QString code = atis->atisCode.isEmpty()? "?": atis->atisCode;
        QString suffix;
        const auto atcLabelTokens = atis->atcLabelTokens();
        if (atcLabelTokens.size() > 2) {
            suffix = atcLabelTokens[1];
        }
        ret << (suffix.isEmpty()
                    ? ": " + code
                    : "_" + suffix + ": " + code);
    }

    if (ret.isEmpty()) {
        return "";
    }

    return "ATIS" + ret.join(", ");
}

const QString Airport::frequencyString() const {
    QStringList ret;
    QStringList controllers;
    foreach (const auto c, atiss) {
        const auto atcLabelTokens = c->atcLabelTokens();
        const QString atisCode = c->atisCode.isEmpty()? "": ": " + c->atisCode;
        QString suffix;
        if (atiss.size() > 1 && atcLabelTokens.size() > 2) {
            suffix = atcLabelTokens[1];
        }
        controllers << ((c->frequency.length() > 1? c->frequency: "") + (suffix.isEmpty()? "": "_" + suffix) + atisCode);
    }
    if (!controllers.empty()) {
        ret << "ATIS: " + controllers.join(", ");
    }

    const QList<QPair < QString, QSet<Controller*> > > controllerSections = {
        { "DEL", dels },
        { "GND", gnds },
        { "TWR", twrs },
        { "APP", appDeps },
    };

    for (auto i = controllerSections.constBegin(); i != controllerSections.constEnd(); ++i) {
        QStringList controllers;
        QSet<QString> frequencies;
        foreach (const auto c, i->second) {
            const auto atcLabelTokens = c->atcLabelTokens();
            QString suffix;
            if (i->second.size() > 1 && atcLabelTokens.size() > 2) {
                suffix = atcLabelTokens[1];
            }
            if (frequencies.contains(c->frequency)) {
                continue;
            }
            controllers << ((c->frequency.length() > 1? c->frequency: c->aliasOrName()) + (suffix.isEmpty()? "": "_" + suffix));
            frequencies.insert(c->frequency);
        }
        if (!controllers.empty()) {
            ret << i->first + (controllers.join("").isEmpty()? "": ": " + controllers.join(", "));
        }
    }

    return ret.join("\n");
}

const QString Airport::pdcString() const {
    foreach (const auto* c, allControllers()) {
        auto match = pdcRegExp.match(c->atisMessage);
        if (match.hasMatch()) {
            return "PDC/" + match.capturedRef(1);
        }
    }

    return "";
}

const GLuint& Airport::gndDisplayList() {
    if (_gndDisplayList != 0) {
        return _gndDisplayList;
    }

    QColor fillColor = Settings::gndFillColor();
    QColor borderColor = Settings::gndBorderLineColor();
    GLfloat borderLineWidth = Settings::gndBorderLineWidth();

    _gndDisplayList = glGenLists(1);
    glNewList(_gndDisplayList, GL_COMPILE);

    GLfloat circle_distort = qCos(lat * Pi180);
    GLfloat innerDeltaLon = (GLfloat) Nm2Deg(Airport::symbologyGndRadius_nm / 2.);
    GLfloat outerDeltaLon = (GLfloat) Nm2Deg(Airport::symbologyGndRadius_nm / .7);
    GLfloat innerDeltaLat = circle_distort * innerDeltaLon;
    GLfloat outerDeltaLat = circle_distort * outerDeltaLon;

    // draw a star shape
    const QList<QPointF> points{
        { lat + outerDeltaLat, lon },
        { lat + innerDeltaLat, lon + innerDeltaLon },
        { lat, lon + outerDeltaLon },
        { lat - innerDeltaLat, lon + innerDeltaLon },
        { lat - outerDeltaLat, lon },
        { lat - innerDeltaLat, lon - innerDeltaLon },
        { lat, lon - outerDeltaLon },
        { lat + innerDeltaLat, lon - innerDeltaLon },
        { lat + outerDeltaLat, lon },
    };

    glColor4f(fillColor.redF(), fillColor.greenF(), fillColor.blueF(), fillColor.alphaF());
    glBegin(GL_TRIANGLE_FAN);
    VERTEX(lat, lon);
    for (int i = 0; i < points.size(); i++) {
        VERTEX(points[i].x(), points[i].y());
    }
    glEnd();

    if (borderLineWidth > 0.) {
        glLineWidth(borderLineWidth);
        glBegin(GL_LINE_LOOP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        for (int i = 0; i < points.size(); i++) {
            VERTEX(points[i].x(), points[i].y());
        }
        glEnd();
    }
    glEndList();

    return _gndDisplayList;
}

const GLuint& Airport::delDisplayList() {
    if (_delDisplayList != 0) {
        return _delDisplayList;
    }

    QColor fillColor = Settings::delFillColor();
    QColor borderColor = Settings::delBorderLineColor();
    GLfloat borderLineWidth = Settings::delBorderLineWidth();
    _delDisplayList = glGenLists(1);
    glNewList(_delDisplayList, GL_COMPILE);

    GLfloat circle_distort = qCos(lat * Pi180);
    GLfloat deltaLon = (GLfloat) Nm2Deg(Airport::symbologyDelRadius_nm / .7);
    GLfloat deltaLat = circle_distort * deltaLon;

    QList<QPointF> points;
    for (int i = 0; i <= 360; i += 10) {
        points.append(
            QPointF(
                lon + deltaLon * qSin(i * Pi180),
                lat + deltaLat * qCos(i * Pi180)
            )
        );
    }

    glColor4f(fillColor.redF(), fillColor.greenF(), fillColor.blueF(), fillColor.alphaF());
    glBegin(GL_TRIANGLE_FAN);
    VERTEX(lat, lon);
    for (int i = 0; i < points.size(); i++) {
        VERTEX(points[i].y(), points[i].x());
    }
    glEnd();

    if (borderLineWidth > 0.) {
        glLineWidth(borderLineWidth);
        glBegin(GL_LINE_LOOP);
        glColor4f(borderColor.redF(), borderColor.greenF(), borderColor.blueF(), borderColor.alphaF());
        for (int i = 0; i < points.size(); i++) {
            VERTEX(points[i].y(), points[i].x());
        }
        glEnd();
    }

    glEndList();

    return _delDisplayList;
}

void Airport::showDetailsDialog() {
    AirportDetails* infoDialog = AirportDetails::instance();
    infoDialog->refresh(this);
    infoDialog->show();
    infoDialog->raise();
    infoDialog->activateWindow();
    infoDialog->setFocus();
}

QSet<Controller*> Airport::allControllers() const {
    return appDeps + twrs + gnds + dels + atiss;
}

bool Airport::matches(const QRegExp& regex) const {
    return id.contains(regex)
        || name.contains(regex)
        || city.contains(regex)
        || countryCode.contains(regex)
        || MapObject::matches(regex);
}

const QString Airport::prettyName() const {
    if (name.contains(city)) {
        return name;
    }
    foreach (const auto part, city.split(QRegExp("\\W"), Qt::SkipEmptyParts)) {
        if (name.contains(part)) {
            return name;
        }
    }

    return name + (city.isEmpty()? "": ", " + city);
}

QString Airport::mapLabel() const {
    auto str = Settings::airportPrimaryContent();

    for (auto i = placeholders.cbegin(), end = placeholders.cend(); i != end; ++i) {
        if (str.contains(i.key())) {
            str.replace(i.key(), i.value()((Airport*) this));
        }
    }

    return str.trimmed();
}

QString Airport::mapLabelHovered() const {
    auto str = Settings::airportPrimaryContentHovered();

    for (auto i = placeholders.cbegin(), end = placeholders.cend(); i != end; ++i) {
        if (str.contains(i.key())) {
            str.replace(i.key(), i.value()((Airport*) this));
        }
    }

    return str.trimmed();
}

QStringList Airport::mapLabelSecondaryLines() const {
    auto str = Settings::airportSecondaryContent();

    for (auto i = placeholders.cbegin(), end = placeholders.cend(); i != end; ++i) {
        if (str.contains(i.key())) {
            str.replace(i.key(), i.value()((Airport*) this));
        }
    }

    return Helpers::linesFilteredTrimmed(str);
}

QStringList Airport::mapLabelSecondaryLinesHovered() const {
    auto str = Settings::airportSecondaryContentHovered();

    for (auto i = placeholders.cbegin(), end = placeholders.cend(); i != end; ++i) {
        if (str.contains(i.key())) {
            str.replace(i.key(), i.value()((Airport*) this));
        }
    }

    return Helpers::linesFilteredTrimmed(str);
}

const QString Airport::shortLabel() const {
    auto _trafficString = trafficString();
    return QString("%1%2").arg(
        id,
        _trafficString.isEmpty()? "": " " + trafficString()
    );
}

const QString Airport::longLabel() const {
    auto _trafficString = trafficString();
    return QString("%1 (%2)%3").arg(
        id,
        prettyName(),
        _trafficString.isEmpty()? "": " " + trafficString()
    );
}

QString Airport::toolTip() const {
    return QString("%1 (%2, %3)").arg(
        id,
        prettyName(),
        countryCode
    );
}

bool Airport::hasPrimaryAction() const {
    return true;
}

void Airport::primaryAction() {
    showDetailsDialog();
}
