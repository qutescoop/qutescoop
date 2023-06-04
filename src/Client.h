#ifndef CLIENT_H_
#define CLIENT_H_

#include "WhazzupData.h"

#include <QJsonDocument>

class Client {
    public:
        static const QRegularExpression livestreamRegExp;
        static QString livestreamString(const QString& str);

        Client(const QJsonObject& json, const WhazzupData* whazzup);

        virtual bool matches(const QRegExp& regex) const;
        virtual QString rank() const;
        virtual QString livestreamString() const;
        virtual bool isFriend() const;
        virtual const QString realName() const;
        virtual const QString nameOrCid() const;
        virtual const QString aliasOrName() const;
        virtual const QString aliasOrNameOrCid() const;

        QString onlineTime() const;
        virtual QString displayName(bool withLink = false) const;
        virtual QString detailInformation() const;

        QString callsign, userId, homeBase, server;
        QDateTime timeConnected;

        bool showAliasDialog(QWidget* parent) const;

        int rating = -99;

        static bool isValidID(const QString id);
        bool hasValidID() const;

    protected:
        QString m_nameOrCid;
};

#endif /*CLIENT_H_*/
