/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef MAPOBJECT_H_
#define MAPOBJECT_H_

#include "_pch.h"

#include "ClientDetails.h"

class ClientDetails;

class MapObject: public QObject
{
	Q_OBJECT

public:
	MapObject();
	MapObject(const MapObject& obj);
	MapObject& operator=(const MapObject& obj);
	virtual ~MapObject();

	virtual bool matches(const QRegExp& regex) const;
	virtual QString mapLabel() const { return label; }

	bool isNull() const { return label.isNull(); }

	virtual QString toolTip() const { return label; }
	virtual void showDetailsDialog() {}

	double lat, lon;
	QString label;
};

#endif /*MAPOBJECT_H_*/
