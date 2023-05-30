#ifndef WAYPOINT_H_
#define WAYPOINT_H_

#include "MapObject.h"

#include <QStringList>

class Waypoint: public MapObject {
    public:
        Waypoint() {}
        Waypoint(const QStringList& stringList);
        Waypoint(const QString& id, const double lat, const double lon);

        virtual QString mapLabel() const {
            return label;
        }
        virtual QString toolTip() const;
        virtual void showDetailsDialog() {} // not applicable
        virtual int type() {
            return 0;
        }
        QString regionCode;
};

#endif /*WAYPOINT_H_*/
