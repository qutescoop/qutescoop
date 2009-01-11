/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
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

#ifndef CLIENT_H_
#define CLIENT_H_

#include <QStringList>
#include <QString>
#include <QDialog>
#include <QDateTime>

#include "MapObject.h"
#include "WhazzupData.h"
#include "ClientDetails.h"

class ClientDetails;
class WhazzupData;

class Client: public MapObject
{
public:
	enum Network { IVAO, VATSIM, OTHER };
	
	Client(const QStringList& stringList, const WhazzupData *whazzup);
	
	QString toolTip() const;
	
	virtual QString rank() const { return QString(); }
	virtual bool matches(const QRegExp& regex) const;
	bool isFriend() const;
	
	// convenience functions for detail displays
	QString onlineTime() const;
	virtual QString displayName(bool withLink = false) const;
	virtual QString detailInformation() const;
	QString clientInformation() const;
	
	QString userId;
	QString realName, homeBase;
	QString server;
	int protrevision;
	QDateTime timeConnected;
	
	int adminRating; // IVAO only
	int rating; // IVAO only
	QString clientSoftware; // IVAO only
	QString clientVersion; // IVAO only
	
	Network network;
	
protected:
	QString getField(const QStringList& list, int index);
};

#endif /*CLIENT_H_*/
