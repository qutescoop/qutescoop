#ifndef WAYPOINT_H_
#define WAYPOINT_H_

#include "MapObject.h"

#include <QStringList>

class Waypoint
    : public MapObject {
    public:
        Waypoint() {}
        Waypoint(const QStringList& stringList);
        Waypoint(const QString& id, const double lat, const double lon);
        virtual ~Waypoint();

        virtual QString mapLabel() const override;
        virtual QStringList mapLabelSecondaryLinesHovered() const override;
        virtual QString toolTip() const override;

        virtual int type();

        const QString airwaysString() const;

        QString id, regionCode;
};

#endif /*WAYPOINT_H_*/
