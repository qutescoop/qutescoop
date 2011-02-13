/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef NAVDATA_H_
#define NAVDATA_H_

#include "_pch.h"

#include "Airport.h"
#include "Sector.h"
#include "Airac.h"

class NavData
{
public:
    static NavData* getInstance();

    const QHash<QString, Airport*>& airports() const { return airportHash; }
    const QMultiMap<int, Airport*>& activeAirports() const { return activeAirportsCongestionMap; }
    const QHash<QString, Sector*>& sectors() const { return sectorHash; }
    QList<Airport*> airportsAt(double lat, double lon, double maxDist);

    const Airac& getAirac() const { return airac; }

    static double distance(double lat1, double lon1, double lat2, double lon2);
    static void distanceTo(double lat, double lon, double dist, double heading, double *latTo, double *lonTo);
    static double courseTo(double lat1, double lon1, double lat2, double lon2);
    static void greatCirclePlotTo(double lat1, double lon1,
                                    double lat2, double lon2,
                                    double fraction,
                                    double *lat, double *lon);
    static void plotPath(double lat1, double lon1, double lat2, double lon2);

    QString countryName(const QString& countryCode) const { return countryCodes[countryCode]; }

    void updateData(const WhazzupData& whazzupData);
    void accept(MapObjectVisitor* visitor);

private:
    NavData();

    void loadDatabase(const QString& directory);
    void loadAirports(const QString& filename);
    void loadSectors();
    void loadCountryCodes(const QString& filename);

    QHash<QString, Airport*> airportHash;
    QMultiMap<int, Airport*> activeAirportsCongestionMap; // holds activeAirports sorted by congestion ascending
    QHash<QString, Sector*> sectorHash;
    QHash<QString, QString> countryCodes;

    Airac airac;
};

#endif /*NAVDATA_H_*/
