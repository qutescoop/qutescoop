/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WHAZZUPDATA_H_
#define WHAZZUPDATA_H_

#include "_pch.h"

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
    WhazzupData(const QDateTime predictTime, const WhazzupData &data); // predict whazzup data
    ~WhazzupData();
    // copy constructor and assignment operator
    WhazzupData(const WhazzupData &data);
    WhazzupData &operator=(const WhazzupData &data);

    bool isNull() const { return (whazzupTime.isNull() && bookingsTime.isNull()); }
    void updateFrom(const WhazzupData &data);

    QSet<Controller*> activeSectors() const;
    QHash<QString, Pilot*> pilots, bookedPilots;
    QHash<QString, Controller*> controllers;
    QList<Pilot*> allPilots() const { return bookedPilots.values() + pilots.values(); }
    QList<BookedController*> bookedControllers;

    Pilot* findPilot(const QString& callsign) const { if(pilots[callsign] != 0) return pilots[callsign];
        else return bookedPilots[callsign]; }

    QList<QStringList> servers, voiceServers;

    QDateTime updateEarliest, whazzupTime, bookingsTime, predictionBasedOnTime, predictionBasedOnBookingsTime;

    bool isIvao() const { return whazzupVersion == 5; }
    bool isVatsim() const { return whazzupVersion == 8; }
    int network() const { switch(whazzupVersion) { case 5: return 0; case 8: return 1; default: return 2; } }

    void accept(MapObjectVisitor *visitor) const;

private:
    void assignFrom(const WhazzupData &data);
    void updatePilotsFrom(const WhazzupData &data);
    void updateControllersFrom(const WhazzupData &data);
    void updateBookedControllersFrom(const WhazzupData &data);

    int whazzupVersion;

    WhazzupType dataType;
};

#endif /*WHAZZUPDATA_H_*/
