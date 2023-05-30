#ifndef AIRWAY_H_
#define AIRWAY_H_

#include "Waypoint.h"

class Airway {
    public:
        Airway(const QString& name);
        virtual ~Airway() {}

        QList<Waypoint*> waypoints() const {
            return _waypoints;
        };
        QList<Waypoint*> expand(const QString& startId, const QString& endId) const;
        Waypoint* closestPointTo(double lat, double lon) const;
        void addSegment(Waypoint* from, Waypoint* to);
        QList<Airway*> sort();

        QString name;

    private:
        int index(const QString& id) const;

        class Segment {
            public:
                Segment(Waypoint* from, Waypoint* to);
                bool operator==(const Segment& other) const;
                Waypoint* from;
                Waypoint* to;
        };

        QList<Segment> _segments;
        QList<Waypoint*> _waypoints;

        Airway* createFromSegments();
};

#endif /* AIRWAY_H_ */
