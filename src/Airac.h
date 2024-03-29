#ifndef AIRAC_H_
#define AIRAC_H_

#include "Airway.h"
#include "NavAid.h"
#include "Waypoint.h"

class Airac
    : public QObject {
    Q_OBJECT
    public:
        static Airac* instance(bool createIfNoInstance = true);
        static QString effectiveCycle(const QDate& date = QDate::currentDate());

        // this is the longest legitimate route part that we can't resolve (yet) (PACOT entry-exit)
        constexpr static const double ifrMaxWaypointInterval = 5500.;
        constexpr static const double nonIfrMaxWaypointInterval = 300.;

        virtual ~Airac();

        Waypoint* waypoint(const QString &id, const QString &regionCode, const int &type) const;
        Waypoint* waypointNearby(const QString &id, double lat, double lon, double maxDist);

        Airway* airway(const QString& name);
        Airway* airwayNearby(const QString& name, double lat, double lon) const;

        QList<Waypoint*> resolveFlightplan(QStringList plan, double lat, double lon, double maxDist);

        QSet<Waypoint*> allPoints;
        QHash<QString, QSet<Waypoint*> > fixes;
        QHash<QString, QSet<NavAid*> > navaids;
        QHash<QString, QList<Airway*> > airways;
    public slots:
        void load();
    signals:
        void loaded();
    private:
        Airac();
        void readFixes(const QString &directory);
        void readNavaids(const QString &directory);
        void readAirways(const QString &directory);
        void addAirwaySegment(Waypoint* from, Waypoint* to, const QString &name);

        QString fpTokenToWaypoint(QString token) const;
};

#endif /* AIRAC_H_ */
