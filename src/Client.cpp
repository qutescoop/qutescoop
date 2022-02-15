/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Client.h"

#include "Whazzup.h"
#include "Settings.h"

Client::Client(const QJsonObject& json, const WhazzupData*) {
    label = json["callsign"].toString();
    userId = QString::number(json["cid"].toInt());
    m_name = json["name"].toString();
    lat = json["latitude"].toDouble();
    lon = json["longitude"].toDouble();
    server = json["server"].toString();
    rating = json["rating"].toInt();
    timeConnected = QDateTime::fromString(json["logon_time"].toString(), Qt::ISODate);

    if(m_name.contains(QRegExp("\\b[A-Z]{4}$"))) {
        homeBase = m_name.right(4);
        m_name = m_name.left(m_name.length() - 4).trimmed();
    }
}

QString Client::onlineTime() const {
    if (!timeConnected.isValid())
        return QString("not connected");

    return QDateTime::fromTime_t( // this will get wrapped by 24h but that should not be a problem...
            Whazzup::instance()->whazzupData().whazzupTime.toTime_t()
            - timeConnected.toTime_t()
            ).toUTC().toString("HH:mm");
}

QString Client::displayName(bool withLink) const {
    QString result = realName();

    if(!rank().isEmpty())
        result += " (" + rank() + ")";

    if(withLink && !userId.isEmpty()) {
        QString link = Whazzup::instance()->userUrl(userId);
        if(link.isEmpty())
            return result;
        result = QString("<a href='%1'>%2</a>").arg(link, result);
    }

    return result;
}

QString Client::detailInformation() const {
    if(!homeBase.isEmpty()) {
        return "(" + homeBase + ")";
    }
    return QString();
}

bool Client::showAliasDialog(QWidget *parent) const
{
    bool ok;
    QString alias = QInputDialog::getText(
        parent,
        QString("Add alias"),
        QString("Set an alias for %1 [empty to unset]:").arg(name()),
        QLineEdit::Normal,
        Settings::clientAlias(userId),
        &ok
    );
    if (ok) {
        Settings::setClientAlias(userId, alias);
    }
    return ok;
}

bool Client::matches(const QRegExp& regex) const {
    if(m_name.contains(regex)) return true;
    if(userId.contains(regex)) return true;
    return MapObject::matches(regex);
}

QString Client::toolTip() const {
    QString result = label + " (" + realName();

    if(!rank().isEmpty()) {
        result += ", " + rank();
    }

    result += ")";
    return result;
}

bool Client::isFriend() const {
    return Settings::friends().contains(userId);
}

const QString Client::realName() const {
     auto& _alias = Settings::clientAlias(userId);
     if (!_alias.isEmpty()) {
         return _alias + " | " + m_name;
     }
     return m_name;
}

const QString Client::name() const
{
    return m_name;
}
