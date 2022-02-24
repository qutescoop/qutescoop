/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef NAVDATA_H_
#define NAVDATA_H_

#include "_pch.h"

#include "Airport.h"
#include "Sector.h"
#include "Airac.h"
#include "GuiMessage.h"
#include "SearchVisitor.h"

class NavData: public QObject {
        Q_OBJECT
    public:
        static NavData *instance(bool createIfNoInstance = true);
        virtual ~NavData();

        QHash<QString, Airport*> airports;
        QMultiMap<int, Airport*> activeAirports; // holds activeAirports sorted by congestion ascending
        QHash<QString, Sector*> sectors;
        QHash<QString, QString> countryCodes;
        QString airlineStr(QString airlineCode);

        Airport* airportAt(double lat, double lon, double maxDist) const;

        static QPair<double, double> *fromArinc(const QString &str);
        static QString toArinc(const short lat, const short lon);

        static double distance(double lat1, double lon1, double lat2, double lon2);
        static QPair<double, double> pointDistanceBearing(double lat, double lon,
                                                          double dist, double heading);
        static double courseTo(double lat1, double lon1, double lat2, double lon2);
        static QPair<double, double> greatCircleFraction(double lat1, double lon1,
                                                         double lat2, double lon2, double fraction);
        static QList<QPair<double, double> > greatCirclePoints(double lat1, double lon1, double lat2,
                                                               double lon2, double intervalNm = 30.);
        static void plotPointsOnEarth(const QList<QPair<double, double> > &points);

        void updateData(const WhazzupData& whazzupData);
        void accept(SearchVisitor* visitor);
    public slots:
        void load();
    signals:
        void loaded();
    private:
        NavData();
        void loadAirports(const QString& filename);
        void loadSectors();
        void loadCountryCodes(const QString& filename);
        void loadAirlineCodes(const QString& filename);
        QHash<QString, QString> _airlineCodes;
};

#endif /*NAVDATA_H_*/
