/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#ifndef MAPOBJECT_H_
#define MAPOBJECT_H_

#include <QObject>
#include <QDialog>

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
	virtual void showDetailsDialog() = 0;

	double lat, lon;
	QString label;
};

#endif /*MAPOBJECT_H_*/
