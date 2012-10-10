/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Client.h"

#include "Whazzup.h"
#include "Settings.h"


Client::Client(const QStringList& stringList, const WhazzupData* whazzup)
{
    label = getField(stringList, 0);
    userId = getField(stringList, 1);
    realName = getField(stringList, 2);
    lat = getField(stringList, 5).toDouble();
    lon = getField(stringList, 6).toDouble();
    server = getField(stringList, 14);
    protrevision = getField(stringList, 15).toInt();
    rating = getField(stringList, 16).toInt();
    timeConnected = QDateTime::fromString(getField(stringList, 37), "yyyyMMddHHmmss");
    timeConnected.setTimeSpec(Qt::UTC);

    if(whazzup->isIvao()) {
        clientSoftware = getField(stringList, 38); // IVAO only
        clientVersion = getField(stringList, 39); // IVAO only
        adminRating = getField(stringList, 40).toInt(); // IVAO only
        rating = getField(stringList, 41).toInt(); // IVAO only
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

QString Client::getField(const QStringList& list, int index) const {
    if(index < 0 || index >= list.size())
        return QString();

    return list[index];
}

QString Client::onlineTime() const {
    if (!timeConnected.isValid())
        return QString("not connected");
    return QDateTime::fromTime_t(               // this will get wrapped by 24h but that should not be a problem...
            Whazzup::getInstance()->whazzupData().whazzupTime.toTime_t()
            - timeConnected.toTime_t()
            ).toUTC().toString("HH:mm");
}

QString Client::displayName(bool withLink) const {
    QString result = realName;
    if(!rank().isEmpty())
        result += " (" + rank() + ")";

    if(withLink && !userId.isEmpty()) {
        QString link = Whazzup::getInstance()->getUserLink(userId);
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
