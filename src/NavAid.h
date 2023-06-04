#ifndef NAVAID_H_
#define NAVAID_H_

#include "Waypoint.h"

class NavAid
    : public Waypoint {
    public:
        enum Type {
            NDB = 2,
            VOR = 3,
            ILS_LOC = 4,
            LOC = 5,
            GS = 6,
            OM = 7,
            MM = 8,
            IM = 9,
            DME_NO_FREQ = 12,
            DME = 13,
            FAP_GBAS = 14,
            GBAS_GND = 15,
            GBAS_THR = 16
        };
        static QString typeStr(Type _type);

        NavAid(const QStringList& stringList);

        virtual QString toolTip() const override;
        virtual QString mapLabelHovered() const override;
        virtual QStringList mapLabelSecondaryLinesHovered() const override;
        virtual int type() override;

        QString freqString() const;
    private:
        Type _type;
        int _freq;
        float _hdg;
        QString _name;
};

#endif /* NAVAID_H_ */
