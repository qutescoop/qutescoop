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
    static NavData *getInstance(bool createIfNoInstance = true);

    const QHash<QString, Airport*>& airports() const { return airportHash; }
    const QMultiMap<int, Airport*>& activeAirports() const { return activeAirportsCongestionMap; }
    const QHash<QString, Sector*>& sectors() const { return sectorHash; }
    QList<Airport*> airportsAt(double lat, double lon, double maxDist);

    static const QPair<double, double> *fromArinc(const QString &str);
    static const QString toArinc(const short lat, const short lon);

    static double distance(double lat1, double lon1, double lat2, double lon2);
    static QPair<double, double> pointDistanceBearing(double lat, double lon, double dist, double heading);
    static double courseTo(double lat1, double lon1, double lat2, double lon2);
    static QPair<double, double> greatCircleFraction(double lat1, double lon1,
                                    double lat2, double lon2, double fraction);
    static QList<QPair<double, double> > greatCirclePoints(double lat1, double lon1, double lat2, double lon2, double pointEachNm = 30.);
    static void plotPointsOnEarth(const QList<QPair<double, double> > &points);

    QString countryName(const QString& countryCode) const { return countryCodes[countryCode]; }

    void updateData(const WhazzupData& whazzupData);
    void accept(MapObjectVisitor* visitor);

private:
    NavData();

    void loadAirports(const QString& filename);
    void loadSectors();
    void loadCountryCodes(const QString& filename);

    QHash<QString, Airport*> airportHash;
    QMultiMap<int, Airport*> activeAirportsCongestionMap; // holds activeAirports sorted by congestion ascending
    QHash<QString, Sector*> sectorHash;
    QHash<QString, QString> countryCodes;
};

#endif /*NAVDATA_H_*/
