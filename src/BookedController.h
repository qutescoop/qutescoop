#ifndef BOOKEDCONTROLLER_H_
#define BOOKEDCONTROLLER_H_

#include <QJsonObject>

/*
 * This is not a "Client" in the sense of a VATSIM client. This is rather a DTO to initialize
 * actual "Controller"s from.
 */
class BookedController {
    public:
        BookedController(const QJsonObject& json);

        QString facilityString() const;

        const QString realName() const;
        bool isFriend() const;
        QDateTime starts() const;
        QDateTime ends() const;

        int facilityType;

        QString bookingInfoStr, bookingType;
        QString callsign, userId;
        QDateTime timeConnected;
    protected:
        QDateTime m_starts, m_ends;
};

#endif /*BOOKEDCONTROLLER_H_*/
