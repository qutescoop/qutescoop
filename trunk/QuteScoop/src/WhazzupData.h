/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WHAZZUPDATA_H_
#define WHAZZUPDATA_H_

#include <QBuffer>
#include <QDateTime>
#include <QHash>

#include "MapObjectVisitor.h"

class Pilot;
class Controller;
class BookedController;
class Client;

class WhazzupData
{
public:
    enum WhazzupType { NONE, WHAZZUP, ATCBOOKINGS, UNIFIED };

    WhazzupData();
    WhazzupData(QBuffer* buffer, WhazzupType type);
    WhazzupData(const QDateTime predictTime, const WhazzupData& data); // predict whazzup data
    ~WhazzupData();

    // copy constructor and assignment operator
    WhazzupData(const WhazzupData& data);
    WhazzupData& operator=(const WhazzupData& data);

    void updateFrom(const WhazzupData& data);

    QList<Controller*> activeSectors() const;
    QList<Pilot*> getPilots() const;
    QList<Pilot*> getBookedPilots() const;
    QList<Pilot*> getAllPilots() const;
    QList<Controller*> getControllers() const { return controllers.values(); }
    QList<BookedController*> getBookedControllers() const { return bookedcontrollers; }

    Pilot* getPilot(const QString& callsign) const { if(pilots[callsign] != 0) return pilots[callsign]; else return bookedpilots[callsign]; }
    Controller* getController(const QString& callsign) const { return controllers[callsign]; }

    int clients() const { return controllers.size() + pilots.size(); }
    QList<QStringList> serverList() const { return connectedServerList; }
    QList<QStringList> voiceServerList() const { return connectedVoiceServerList; }
    int version() const { return whazzupVersion; }

    QDateTime updateEarliest;
    //const QDateTime& updateEarliest() const { return updateEarliest; }
    const QDateTime& timestamp() const { return whazzupTime; }
    const QDateTime& bookingsTimestamp() const { return bookingsTime; }
    const QDateTime& predictionBasedOnTimestamp() const { return predictionBasedOnTime; }
    const QDateTime& predictionBasedOnBookingsTimestamp() const { return predictionBasedOnBookingsTime; }

    bool isIvao() const { return whazzupVersion == 4; }
    bool isVatsim() const { return whazzupVersion == 8; }
    int network() const { switch(whazzupVersion) { case 4: return 0; case 8: return 1; default: return 2;} }

    void accept(MapObjectVisitor *visitor) const;

    bool isNull() const { return (whazzupTime.isNull() && bookingsTime.isNull()); }

private:
    void assignFrom(const WhazzupData& data);

    void updatePilotsFrom(const WhazzupData& data);
    void updateControllersFrom(const WhazzupData& data);
    void updateBookedControllersFrom(const WhazzupData& data);

    QHash<QString, Pilot*> pilots;
    QHash<QString, Pilot*> bookedpilots; // honestly, there could be same callsigns in the bookings, but it is easier like that (searching...)
    QHash<QString, Controller*> controllers;
    QList<BookedController*> bookedcontrollers;
    QList<QStringList> connectedServerList;
    QList<QStringList> connectedVoiceServerList;
    int whazzupVersion;

    QDateTime whazzupTime, bookingsTime;
    QDateTime predictionBasedOnTime;
    QDateTime predictionBasedOnBookingsTime;
    WhazzupType dataType;
};

#endif /*WHAZZUPDATA_H_*/
