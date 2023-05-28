/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef NAVDATA_H_
#define NAVDATA_H_

#include "Sector.h"
#include "SearchVisitor.h"
#include "Airline.h"

struct ControllerAirportsMapping {
    QString prefix;
    QStringList suffixes;
    QList<Airport*> airports;
};

enum LatLngPrecission {
    Secs = 3, Mins = 2, Degrees = 1
};

class NavData: public QObject {
        Q_OBJECT
    public:
        static NavData *instance(bool createIfNoInstance = true);
        static QPair<double, double> *fromArinc(const QString &str);
        static QString toArinc(const short lat, const short lon);
        static QString toEurocontrol(const double lat, const double lon, const LatLngPrecission maxPrecision = LatLngPrecission::Secs);

        static double distance(double lat1, double lon1, double lat2, double lon2);
        static QPair<double, double> pointDistanceBearing(double lat, double lon,
                                                          double dist, double heading);
        static double courseTo(double lat1, double lon1, double lat2, double lon2);
        static QPair<double, double> greatCircleFraction(double lat1, double lon1,
                                                         double lat2, double lon2, double fraction);
        static QList<QPair<double, double> > greatCirclePoints(double lat1, double lon1, double lat2,
                                                              double lon2, double intervalNm = 30.);
        static void plotGreatCirclePoints(const QList<QPair<double, double> > &points);

        virtual ~NavData();

        QHash<QString, Airport*> airports;
        QMultiMap<int, Airport*> activeAirports; // holds activeAirports sorted by congestion ascending
        QHash<QString, Sector*> sectors;
        QHash<QString, QString> countryCodes;
        QString airline(const QString &airlineCode);
        QHash<QString, Airline*> airlines;

        Airport* airportAt(double lat, double lon, double maxDist) const;

        QList<Airport*> additionalMatchedAirportsForController(QString prefix, QString suffix) const;

        void updateData(const WhazzupData& whazzupData);
        void accept(SearchVisitor* visitor);
    public slots:
        void load();
    signals:
        void loaded();
    private:
        NavData();
        void loadAirports(const QString& filename);
        void loadControllerAirportsMapping(const QString& filename);
        QList<ControllerAirportsMapping> m_controllerAirportsMapping;
        void loadSectors();
        void loadCountryCodes(const QString& filename);
        void loadAirlineCodes(const QString& filename);
};

#endif /*NAVDATA_H_*/
