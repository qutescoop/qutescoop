#include "BookedController.h"
#include "Settings.h"

#include <QJsonObject>

BookedController::BookedController(const QJsonObject& json) {
    callsign = json["callsign"].toString();
    Q_ASSERT(!callsign.isNull());
    userId = QString::number(json["cid"].toInt());

    bookingType = json["type"].toString();

    timeConnected = m_starts = QDateTime::fromString(json["start"].toString() + "Z", "yyyy-MM-dd HH:mm:sst");
    m_ends = QDateTime::fromString(json["end"].toString() + "Z", "yyyy-MM-dd HH:mm:sst");

    if (bookingType == "booking") {
        bookingInfoStr = "";
    } else if (bookingType == "event") {
        bookingInfoStr = "Event";
    } else if (bookingType == "training") {
        bookingInfoStr = "Training";
    } else {
        bookingInfoStr = bookingType;
    }

    if (callsign.right(5) == "_ATIS") {
        facilityType = 2;
    } else if (callsign.right(4) == "_DEL") {
        facilityType = 2;
    } else if (callsign.right(4) == "_GND") {
        facilityType = 3;
    } else if (callsign.right(4) == "_TWR") {
        facilityType = 4;
    } else if (callsign.right(4) == "_APP" || callsign.right(4) == "_DEP") {
        facilityType = 5;
    } else if (callsign.right(4) == "_CTR") {
        facilityType = 6;
    } else if (callsign.right(4) == "_FSS") {
        facilityType = 7;
    }
}

QString BookedController::facilityString() const {
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

const QString BookedController::realName() const {
    auto& _alias = Settings::clientAlias(userId);
    if (!_alias.isEmpty()) {
        return _alias;
    }
    return userId;
}

bool BookedController::isFriend() const {
    return Settings::friends().contains(userId);
}

QDateTime BookedController::starts() const {
    return m_starts;
}

QDateTime BookedController::ends() const {
    return m_ends;
}
