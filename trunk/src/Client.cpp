/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Client.h"

#include "Whazzup.h"
#include "Settings.h"


Client::Client(const QStringList& stringList, const WhazzupData* whazzup)
{
    label = field(stringList, 0);
    userId = field(stringList, 1);
    realName = field(stringList, 2);
    lat = field(stringList, 5).toDouble();
    lon = field(stringList, 6).toDouble();
    server = field(stringList, 14);
    protrevision = field(stringList, 15).toInt();
    rating = field(stringList, 16).toInt();
    timeConnected = QDateTime::fromString(field(stringList, 37), "yyyyMMddHHmmss");
    timeConnected.setTimeSpec(Qt::UTC);

    if(whazzup->isIvao()) {
        clientSoftware = field(stringList, 38); // IVAO only
        clientVersion = field(stringList, 39); // IVAO only
        adminRating = field(stringList, 40).toInt(); // IVAO only
        rating = field(stringList, 41).toInt(); // IVAO only
    }

    if(whazzup->isVatsim())
        network = VATSIM;
    else if(whazzup->isIvao())
        network = IVAO;
    else
        network = OTHER;

    // un-fuck user names. people enter all kind of stuff here
    realName.remove(QRegExp("[_\\-\\d\\.\\,\\;\\:\\#\\+\\(\\)]"));
    realName = realName.trimmed();

    if(realName.contains(QRegExp("\\b[A-Z]{4}$"))) {
        homeBase = realName.right(4);
        realName = realName.left(realName.length() - 4).trimmed();
    }
}

QString Client::field(const QStringList& list, int index) const {
    if(index < 0 || index >= list.size())
        return QString();

    return list[index];
}

QString Client::onlineTime() const {
    if (!timeConnected.isValid())
        return QString("not connected");
    return QDateTime::fromTime_t(               // this will get wrapped by 24h but that should not be a problem...
            Whazzup::instance()->whazzupData().whazzupTime.toTime_t()
            - timeConnected.toTime_t()
            ).toUTC().toString("HH:mm");
}

QString Client::displayName(bool withLink) const {
    QString result = realName;
    if(!rank().isEmpty())
        result += " (" + rank() + ")";

    if(withLink && !userId.isEmpty()) {
        QString link = Whazzup::instance()->userLink(userId);
        if(link.isEmpty())
            return result;
        result = QString("<a href='%1'>%2</a>").arg(link).arg(result);
    }

    return result;
}

QString Client::clientInformation() const {
    if(!clientSoftware.isEmpty())
        return clientSoftware + " " + clientVersion;
    return QString();
}

QString Client::detailInformation() const {
    if(!homeBase.isEmpty()) {
        return "(" + homeBase + ")";
    }
    return QString();
}

bool Client::matches(const QRegExp& regex) const {
    if(realName.contains(regex)) return true;
    if(userId.contains(regex)) return true;
    return MapObject::matches(regex);
}

QString Client::toolTip() const {
    QString result = label + " (" + realName;
    if(!rank().isEmpty()) result += ", " + rank();
    result += ")";
    return result;
}

bool Client::isFriend() const {
    return Settings::friends().contains(userId);
}
