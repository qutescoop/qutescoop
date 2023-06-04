#ifndef WHAZZUPDATA_H_
#define WHAZZUPDATA_H_

#include "MapObjectVisitor.h"

class Pilot;
class Controller;
class BookedController;
class Client;

class WhazzupData {
    public:
        enum WhazzupType { NONE, WHAZZUP, ATCBOOKINGS, UNIFIED };

        WhazzupData();
        WhazzupData(QByteArray* bytes, WhazzupType type);
        WhazzupData(const QDateTime predictTime, const WhazzupData &data); // predict whazzup data
        WhazzupData(const WhazzupData &data);
        ~WhazzupData();

        WhazzupData &operator=(const WhazzupData &data);

        bool isNull() const;
        void updateFrom(const WhazzupData &data);

        QSet<Controller*> controllersWithSectors() const;
        QHash<QString, Pilot*> pilots, bookedPilots;
        QHash<QString, Controller*> controllers;
        QList<Pilot*> allPilots() const;
        QList<BookedController*> bookedControllers;

        QList<QPair<double, double> > friendsLatLon() const;

        Pilot* findPilot(const QString& callsign) const;

        QList<QStringList> servers;
        QHash<int, QString> ratings;
        QHash<int, QString> pilotRatings;
        QHash<int, QString> militaryRatings;

        QDateTime updateEarliest, whazzupTime, bookingsTime, predictionBasedOnTime, predictionBasedOnBookingsTime;

        void accept(MapObjectVisitor* visitor) const;
    private:
        void assignFrom(const WhazzupData &data);
        void updatePilotsFrom(const WhazzupData &data);
        void updateControllersFrom(const WhazzupData &data);
        void updateBookedControllersFrom(const WhazzupData &data);
        int _whazzupVersion;
        WhazzupType _dataType;
};

#endif /*WHAZZUPDATA_H_*/
