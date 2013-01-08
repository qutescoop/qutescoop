/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef AIRAC_H_
#define AIRAC_H_

#include "_pch.h"

#include "Waypoint.h"
#include "NavAid.h"
#include "Airway.h"


class Airac : public QObject {
        Q_OBJECT
    public:
        static Airac *instance(bool createIfNoInstance = true);
        virtual ~Airac();

        Waypoint* waypoint(const QString &id, double lat, double lon,
                           double maxDist = 2000.0) const;
        QList<Waypoint*> resolveFlightplan(QStringList plan, double lat, double lon) const;
        Airway* airway(const QString& name, double lat, double lon) const;

        QSet<Waypoint*> allPoints;
        QHash<QString, QSet<Waypoint*> > waypoints;
        QHash<QString, QSet<NavAid*> > navaids;
        QHash<QString, QList<Airway*> > airways;
    public slots:
        void load();
    signals:
        void loaded();
    private:
        Airac();
        void readWaypoints(const QString &directory);
        void readNavaids(const QString &directory);
        void readAirways(const QString &directory);
        void addAirwaySegment(Waypoint* from, Waypoint* to, Airway::Type type,
                              int base, int top, const QString &name);

        Airway* airway(const QString& name, Airway::Type type, int base, int top);
};

#endif /* AIRAC_H_ */
