/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef NAVAID_H_
#define NAVAID_H_

#include "Waypoint.h"

class NavAid: public Waypoint {
    public:
        NavAid(const QStringList& stringList);

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
        virtual QString toolTip() const;
        int type() { return _type; }
        QString regionCode;

    private:
        Type _type;
        int _alt, _freq, _range;
        float _hdg;
        QString _name;
};

#endif /* NAVAID_H_ */
