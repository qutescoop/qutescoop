#include "Controller.h"

#include "Airport.h"
#include "Client.h"
#include "NavData.h"
#include "Settings.h"
#include "Whazzup.h"
#include "dialogs/ControllerDetails.h"
#include "helpers.h"

#include <QJsonObject>

const QRegularExpression Controller::cpdlcRegExp = QRegularExpression(
    "CPDLC.{0,25}\\W([A-Z]{3}[A-Z0-9])(\\W|$)", QRegularExpression::MultilineOption | QRegularExpression::InvertedGreedinessOption
);

const QHash<QString, std::function<QString(Controller*)> > Controller::placeholders {
    {
        "{sectorOrLogin}", [](Controller* o)->QString {
            if (o->sector != 0) {
                return o->controllerSectorName();
            }

            return o->callsign;
        }
    },
    {
        "{sector}", [](Controller* o)->QString {
            if (o->sector != 0) {
                return o->sector->name;
            }

            return "";
        }
    },
    {
        "{name}", [](Controller* o)->QString {
            return o->aliasOrName();
        }
    },
    {
        "{nameIfFriend}", [](Controller* o)->QString {
            return o->isFriend()? o->aliasOrName(): "";
        }
    },
    {
        "{rating}", [](Controller* o)->QString {
            return o->rank();
        }
    },
    {
        "{frequency}", [](Controller* o)->QString {
            return o->frequency.length() > 1? o->frequency: "";
        }
    },
    {
        "{cpdlc}", [](Controller* o)->QString {
            return o->cpdlcString();
        }
    },
    {
        "{livestream}", [](Controller* o)->QString {
            return o->livestreamString();
        }
    },
};

Controller::Controller(const QJsonObject& json, const WhazzupData* whazzup)
    : MapObject(), Client(json, whazzup),
      sector(0) {
    frequency = json["frequency"].toString();
    Q_ASSERT(!frequency.isNull());
    facilityType = json["facility"].toInt();
    if (callsign.right(4) == "_FSS") {
        facilityType = 7; // workaround as VATSIM reports 1 for _FSS
    }
    visualRange = json["visual_range"].toInt();

    atisMessage = "";
    if (json.contains("text_atis") && json["text_atis"].isArray()) {
        QJsonArray atis = json["text_atis"].toArray();
        for (int i = 0; i < atis.size(); ++i) {
            atisMessage += atis[i].toString() + "\n";
        }
    }

    atisCode = "";
    if (json.contains("atis_code")) {
        atisCode = json["atis_code"].isNull()? "": json["atis_code"].toString();
    }

    // do some magic for Controller Info like "online until"...
    QRegExp rxOnlineUntil = QRegExp(
        "(open|close|online|offline|till|until)(\\W*\\w*\\W*){0,4}\\b(\\d{1,2}):?(\\d{2})\\W?(z|utc)?",
        Qt::CaseInsensitive
    );
    if (rxOnlineUntil.indexIn(atisMessage) > 0) {
        QTime found = QTime::fromString(rxOnlineUntil.cap(3) + rxOnlineUntil.cap(4), "HHmm");
        if (found.isValid()) {
            if (qAbs(found.secsTo(whazzup->whazzupTime.time())) > 60 * 60 * 12) {
                // e.g. now its 2200z, and he says "online until 0030z", allow for up to 12 hours
                assumeOnlineUntil = QDateTime(whazzup->whazzupTime.date().addDays(1), found, Qt::UTC);
            } else {
                assumeOnlineUntil = QDateTime(whazzup->whazzupTime.date(), found, Qt::UTC);
            }
        }
    }

    QString icao = controllerSectorName();
    // Look for a sector name prefix matching the login
    if (!icao.isEmpty()) {
        do {
            foreach (const auto _sector, NavData::instance()->sectors.values(icao)) {
                if (_sector->controllerSuffixes().isEmpty()) {
                    sector = _sector;
                    break;
                }

                foreach (const auto suffix, _sector->controllerSuffixes()) {
                    if (callsign.endsWith(suffix)) {
                        sector = _sector;
                        break;
                    }
                }
            }

            if (sector != 0) {
                // We determine lat/lon from the sector
                QPair<double, double> center = this->sector->getCenter();
                if (center.first > -180.) {
                    lat = center.first;
                    lon = center.second;
                }

                break;
            }
            icao.chop(1);
        } while (icao.length() >= 2);

        if (sector == 0) {
            QString msg("Unknown sector/FIR " + controllerSectorName() + " - Please provide sector information if you can.");
            qInfo() << msg;
            QTextStream(stdout) << "INFO: " << msg << Qt::endl;
        }
    } else {
        // We try to get lat/lng from covered airports
        auto _airports = airports();
        if (_airports.size() > 0) {
            auto _a = *_airports.constBegin();
            lat = _a->lat;
            lon = _a->lon;
        }
    }
}

Controller::~Controller() {}

QString Controller::facilityString() const {
    switch (facilityType) {
        case 0: return "OBS";
        case 1: return "Staff";
        case 2: return "DEL";
        case 3: return "GND";
        case 4: return "TWR";
        case 5: return "APP";
        case 6: return "CTR";
        case 7: return "FSS";
    }
    return QString();
}

QString Controller::typeString() const {
    const auto labelTokens = atcLabelTokens();
    if (labelTokens.isEmpty()) {
        return "";
    }
    return labelTokens.constLast();
}

QStringList Controller::atcLabelTokens() const {
    if (!isATC()) {
        return QStringList();
    }

    return callsign.split('_', Qt::SkipEmptyParts);
}

QString Controller::controllerSectorName() const {
    auto _atcLabelTokens = atcLabelTokens();

    if (
        !_atcLabelTokens.empty()
        && (_atcLabelTokens.last().startsWith("CTR") || _atcLabelTokens.last().startsWith("FSS"))
    ) {
        _atcLabelTokens.removeLast();
        return _atcLabelTokens.join("_");
    }
    return QString();
}

QString Controller::livestreamString() const {
    return Client::livestreamString(atisMessage);
}

bool Controller::isCtrFss() const {
    return callsign.endsWith("_CTR") || callsign.endsWith("_FSS");
}

bool Controller::isAppDep() const {
    return callsign.endsWith("_APP") || callsign.endsWith("_DEP");
}

bool Controller::isTwr() const {
    return callsign.endsWith("_TWR");
}

bool Controller::isGnd() const {
    return callsign.endsWith("_GND");
}

bool Controller::isDel() const {
    return callsign.endsWith("_DEL");
}

bool Controller::isAtis() const {
    return callsign.endsWith("_ATIS");
}

QSet <Airport*> Controller::airports(bool withAdditionalMatches) const {
    auto airports = QSet<Airport*>();
    auto _atcLabelTokens = atcLabelTokens();
    if (_atcLabelTokens.empty()) {
        return airports;
    }
    auto prefix = _atcLabelTokens.constFirst();

    // ordinary / normal match EDDS_STG_APP -> EDDS
    auto a = NavData::instance()->airports.value(prefix, 0);
    if (a != 0) {
        airports.insert(a);
    }

    if (withAdditionalMatches) {
        auto suffix = _atcLabelTokens.constLast();
        // use matches from controllerAirportsMapping.dat
        auto _airports = NavData::instance()->additionalMatchedAirportsForController(prefix, suffix);
        foreach (const auto& _a, _airports) {
            airports.insert(_a);
        }
    }

//    if(sector != 0) {
//        foreach(auto* a, NavData::instance()->airports) {
//            if(sector->containsPoint(QPointF(a->lat, a->lon))) {
//                airports.insert(a);
//            }
//        }
//    }

    // if we have not had any match yet and since some
    // VATSIMmers still don't think ICAO codes are cool
    // IAH_TWR -> IAH
    if (airports.isEmpty() && prefix.length() == 3) {
        auto a = NavData::instance()->airports.value("K" + prefix, 0);
        if (a != 0) {
            airports.insert(a);
        }
    }

    return airports;
}

QList <Airport*> Controller::airportsSorted() const {
    auto _airports = airports().values();
    // sort by congestion
    std::sort(
        _airports.begin(),
        _airports.end(),
        [](const Airport* a, const Airport* b)->bool {
            return a->congestion() > b->congestion();
        }
    );

    return _airports;
}

const QString Controller::cpdlcString() const {
    auto match = cpdlcRegExp.match(atisMessage);
    if (match.hasMatch()) {
        return "CPDLC/" + match.capturedRef(1);
    }

    return "";
}

void Controller::showDetailsDialog() {
    ControllerDetails* infoDialog = ControllerDetails::instance();
    infoDialog->refresh(this);
    infoDialog->show();
    infoDialog->raise();
    infoDialog->activateWindow();
    infoDialog->setFocus();
}

QString Controller::rank() const {
    return Whazzup::instance()->realWhazzupData().ratings.value(rating, QString());
}

bool Controller::isFriend() const {
    if (isAtis()) {
        return false;
    }
    return Client::isFriend();
}

QString Controller::toolTip() const { // LOVV_CTR [Vienna] (134.350, Alias | Name, C1)
    QString result = callsign;
    if (sector != 0) {
        result += " [" + sector->name + "]";
    }
    result += " (";
    if (!isObserver() && !frequency.isEmpty()) {
        result += frequency + ", ";
    }
    result += realName();
    if (!rank().isEmpty()) {
        result += ", " + rank();
    }
    result += ")";
    return result;
}

QString Controller::toolTipShort() const // LOVV_CTR [Vienna]
{
    QString result = callsign;
    if (sector != 0) {
        result += " [" + sector->name + "]";
    }
    return result;
}

QString Controller::mapLabel() const {
    auto str = Settings::firPrimaryContent();

    for (auto i = placeholders.cbegin(), end = placeholders.cend(); i != end; ++i) {
        if (str.contains(i.key())) {
            str.replace(i.key(), i.value()((Controller*) this));
        }
    }

    return str.trimmed();
}

QString Controller::mapLabelHovered() const {
    auto str = Settings::firPrimaryContentHovered();

    for (auto i = placeholders.cbegin(), end = placeholders.cend(); i != end; ++i) {
        if (str.contains(i.key())) {
            str.replace(i.key(), i.value()((Controller*) this));
        }
    }

    return str.trimmed();
}

QStringList Controller::mapLabelSecondaryLines() const {
    auto str = Settings::firSecondaryContent();

    for (auto i = placeholders.cbegin(), end = placeholders.cend(); i != end; ++i) {
        if (str.contains(i.key())) {
            str.replace(i.key(), i.value()((Controller*) this));
        }
    }

    return Helpers::linesFilteredTrimmed(str);
}

QStringList Controller::mapLabelSecondaryLinesHovered() const {
    auto str = Settings::firSecondaryContentHovered();

    for (auto i = placeholders.cbegin(), end = placeholders.cend(); i != end; ++i) {
        if (str.contains(i.key())) {
            str.replace(i.key(), i.value()((Controller*) this));
        }
    }

    return Helpers::linesFilteredTrimmed(str);
}

bool Controller::matches(const QRegExp& regex) const {
    return frequency.contains(regex)
        || atisMessage.contains(regex)
        || realName().contains(regex)
        || (sector != 0 && sector->name.contains(regex))
        || MapObject::matches(regex);
}

bool Controller::hasPrimaryAction() const {
    return true;
}

void Controller::primaryAction() {
    showDetailsDialog();
}

bool Controller::isObserver() const {
    return facilityType == 0;
}

bool Controller::isATC() const {
    // 199.998 gets transmitted on VATSIM for a controller without prim freq
    Q_ASSERT(!frequency.isNull());
    return facilityType > 0 && !frequency.isEmpty() && frequency != "199.998";
}
