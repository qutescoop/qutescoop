/**************************************************************************
 *  This file is part of QuteScoop. See README for license
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
