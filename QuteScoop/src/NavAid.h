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
		DME = 13
	};
	static QString typeStr(Type type);
	virtual QString toolTip() const;

private:
	Type type;
	int altitude, frequency, range;
	float heading;
	QString name;
};

#endif /* NAVAID_H_ */
