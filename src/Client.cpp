#include "Client.h"

#include "Settings.h"
#include "Whazzup.h"

#include <QInputDialog>

const QRegularExpression Client::livestreamRegExp = QRegularExpression(
    "("
    "(twitch)(\\.tv)?"
    "|(youtu\\.?be)(\\.com)?"
    "|(owncast)(\\.online)?"
    ")"
    "\\W([^ |\\n]+)",
    QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption
);

QString Client::livestreamString(const QString& str) {
    auto matchIterator = livestreamRegExp.globalMatch(str);

    // take last match. Helps with "live on twitch now: twitch/user"
    while (matchIterator.hasNext()) {
        auto match = matchIterator.next();
        if (!matchIterator.hasNext()) {
            QString network(match.captured(2) + match.captured(4) + match.captured(6));
            return network.toLower() + "/" + match.capturedRef(8);
        }
    }

    return "";
}

Client::Client(const QJsonObject& json, const WhazzupData*)
    : callsign(""), userId(""), homeBase(""), server(""), rating(-99) {
    callsign = json["callsign"].toString();
    if (callsign.isNull()) {
        callsign = "";
    }

    userId = QString::number(json["cid"].toInt());
    server = json["server"].toString();
    rating = json["rating"].toInt();
    timeConnected = QDateTime::fromString(json["logon_time"].toString(), Qt::ISODate);

    m_nameOrCid = json["name"].toString();
    if (m_nameOrCid.isNull()) {
        m_nameOrCid = "";
    }
    if (m_nameOrCid.contains(QRegExp("\\b[A-Z]{4}$"))) {
        homeBase = m_nameOrCid.right(4);
        m_nameOrCid = m_nameOrCid.chopped(4).trimmed();
    }
}

QString Client::onlineTime() const {
    if (!timeConnected.isValid()) {
        return QString("not connected");
    }

    return QDateTime::fromTime_t(
        // this will get wrapped by 24h but that should not be a problem...
        Whazzup::instance()->whazzupData().whazzupTime.toTime_t()
        - timeConnected.toTime_t()
    ).toUTC().toString("HH:mm") + " hrs";
}

QString Client::displayName(bool withLink) const {
    QString result = realName();

    if (!rank().isEmpty()) {
        result += " (" + rank() + ")";
    }

    if (withLink && hasValidID()) {
        QString link = Whazzup::instance()->userUrl(userId);
        if (link.isEmpty()) {
            return result;
        }
        result = QString("<a href='%1'>%2</a>").arg(link, result.toHtmlEscaped());
    }

    return result;
}

QString Client::detailInformation() const {
    if (!homeBase.isEmpty()) {
        return "(" + homeBase + ")";
    }
    return "";
}

bool Client::showAliasDialog(QWidget* parent) const {
    bool ok;
    QString alias = QInputDialog::getText(
        parent,
        QString("Edit alias"),
        QString("Set the alias for %1 [empty to unset]:").arg(nameOrCid()),
        QLineEdit::Normal,
        Settings::clientAlias(userId),
        &ok
    );
    if (ok) {
        Settings::setClientAlias(userId, alias);
    }
    return ok;
}

bool Client::isValidID(const QString id) {
    return !id.isEmpty() && id.toInt() >= 800000;
}

bool Client::matches(const QRegExp& regex) const {
    return m_nameOrCid.contains(regex)
        || userId.contains(regex);
}

QString Client::rank() const {
    return "";
}

QString Client::livestreamString() const {
    return "";
}

bool Client::isFriend() const {
    return Settings::friends().contains(userId);
}

const QString Client::realName() const {
    auto& _alias = Settings::clientAlias(userId);
    if (!_alias.isEmpty() && _alias != m_nameOrCid) {
        return _alias + " | " + m_nameOrCid;
    }
    return m_nameOrCid;
}

const QString Client::nameOrCid() const {
    return m_nameOrCid;
}

const QString Client::aliasOrName() const {
    auto _alisOrNameOrCid = aliasOrNameOrCid();

    if (isValidID(_alisOrNameOrCid)) {
        return "";
    }

    return _alisOrNameOrCid;
}

const QString Client::aliasOrNameOrCid() const {
    auto& _alias = Settings::clientAlias(userId);
    if (!_alias.isEmpty()) {
        return _alias;
    }
    return m_nameOrCid;
}

bool Client::hasValidID() const {
    return Client::isValidID(userId);
}
