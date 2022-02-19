/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CLIENT_H_
#define CLIENT_H_

#include "_pch.h"

#include "MapObject.h"
#include "WhazzupData.h"
#include "ClientDetails.h"

#include <QJsonDocument>

class Client: public MapObject {
    public:
        Client(const QJsonObject& json, const WhazzupData *whazzup);

        virtual QString toolTip() const;

        virtual QString rank() const { return QString(); }
        virtual bool matches(const QRegExp& regex) const;
        bool isFriend() const;

        // convenience functions for detail displays
        QString onlineTime() const;
        virtual QString displayName(bool withLink = false) const;
        virtual QString detailInformation() const;
        QString clientInformation() const;

        QString userId, realName, homeBase, server;
        QDateTime timeConnected;

        int rating;
};

#endif /*CLIENT_H_*/
