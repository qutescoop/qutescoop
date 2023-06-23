#include "Airport.h"

#include "helpers.h"
#include "NavData.h"
#include "Settings.h"
#include "dialogs/AirportDetails.h"
#include "src/mustache/Renderer.h"

const QRegularExpression Airport::pdcRegExp = QRegularExpression(
    "PDC.{0,20}\\W([A-Z]{4})(\\W|$)", QRegularExpression::MultilineOption | QRegularExpression::InvertedGreedinessOption
);

Airport::Airport(const QStringList& list, unsigned int debugLineNumber)
    : MapObject(),
      _appDisplayList(0), _twrDisplayList(0), _gndDisplayList(0), _delDisplayList(0) {
    resetWhazzupStatus();

    if (list.size() != 6) {
        QMessageLogger("airports.dat", debugLineNumber, QT_MESSAGELOG_FUNC).critical()
            << "While processing line" << list.join(':') << ": Found" << list.size() << "fields, expected exactly 6.";
        exit(EXIT_FAILURE);
    }

    id = list[0];
    name = list[1];
    city = list[2];
    countryCode = list[3];

    if (countryCode != "" && NavData::instance()->countryCodes.value(countryCode, "") == "") {
        QMessageLogger("airports.dat", debugLineNumber, QT_MESSAGELOG_FUNC).critical()
            << "While processing line" << list.join(':') << ": Could not find country" << countryCode;
        exit(EXIT_FAILURE);
    }

    lat = list[4].toDouble();
    lon = list[5].toDouble();
}

Airport::~Airport() {
    MustacheQs::Renderer::teardownContext(this);

    if (glIsList(_appDisplayList) == GL_TRUE) {
        glDeleteLists(_appDisplayList, 1);
    }
    if (glIsList(_twrDisplayList) == GL_TRUE) {
        glDeleteLists(_twrDisplayList, 1);
    }
    if (glIsList(_gndDisplayList) == GL_TRUE) {
        glDeleteLists(_gndDisplayList, 1);
    }
    if (glIsList(_delDisplayList) == GL_TRUE) {
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

    nMaybeFilteredArrivals = 0;
    nMaybeFilteredDepartures = 0;
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
    return nMaybeFilteredArrivals + nMaybeFilteredDepartures;
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
    if (glIsList(_appDisplayList) == GL_TRUE) {
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
    if (glIsList(_twrDisplayList) == GL_TRUE) {
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
    auto tmpl = "{#allArrs}{allArrs}{/allArrs}{^allArrs}-{/allArrs}/{#allDeps}{allDeps}{/allDeps}{^allDeps}-{/allDeps}";

    if (Settings::filterTraffic()) {
        tmpl = "{#arrs}{arrs}{/arrs}{^arrs}-{/arrs}/{#deps}{deps}{/deps}{^deps}-{/deps}";
    }
    return MustacheQs::Renderer::render(tmpl, (QObject*) this);
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
            controllers << ((c->frequency.length() > 1? c->frequency: "") + (suffix.isEmpty()? "": "_" + suffix));
            frequencies.insert(c->frequency);
        }
        if (!controllers.empty()) {
            ret << i->first + (controllers.join("").isEmpty()? "": ": " + controllers.join(", "));
        }
    }

    return ret.join("\n");
}

const QString Airport::pdcString(const QString &prepend, bool alwaysWithIdentifier) const {
    QStringList matches;

    foreach (const auto* c, allControllers()) {
        auto match = pdcRegExp.match(c->atisMessage);
        if (match.hasMatch()) {
            auto logon = match.captured(1);
            if (id == logon) {
                if (!alwaysWithIdentifier) {
                    // found perfect match
                    return prepend;
                }
            }
            if (!matches.contains(logon)) {
                matches << logon;
            }
        }
    }

    return matches.isEmpty()? "": prepend + matches.join(",");
}

const GLuint& Airport::gndDisplayList() {
    if (glIsList(_gndDisplayList) == GL_TRUE) {
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
    if (glIsList(_delDisplayList) == GL_TRUE) {
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
    auto tmpl = Settings::airportPrimaryContent();
    return MustacheQs::Renderer::render(tmpl, (QObject*) this);
}

QString Airport::mapLabelHovered() const {
    auto tmpl = Settings::airportPrimaryContentHovered();
    return MustacheQs::Renderer::render(tmpl, (QObject*) this);
}

QStringList Airport::mapLabelSecondaryLines() const {
    auto tmpl = Settings::airportSecondaryContent();
    return Helpers::linesFilteredTrimmed(
        MustacheQs::Renderer::render(tmpl, (QObject*) this)
    );
}

QStringList Airport::mapLabelSecondaryLinesHovered() const {
    auto tmpl = Settings::airportSecondaryContentHovered();
    return Helpers::linesFilteredTrimmed(
        MustacheQs::Renderer::render(tmpl, (QObject*) this)
    );
}

QString Airport::livestreamString() const {
    QStringList ret;
    foreach (const auto c, allControllers()) {
        const auto str = c->livestreamString();
        if (!str.isEmpty()) {
            ret << str;
        }
    }

    return ret.join(" ");
}

const QString Airport::shortLabel() const {
    auto _trafficString = trafficString();
    return QString("%1%2").arg(
        id,
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
