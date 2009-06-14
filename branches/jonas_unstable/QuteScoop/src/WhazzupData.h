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

#ifndef WHAZZUPDATA_H_
#define WHAZZUPDATA_H_

#include <QBuffer>
#include <QDateTime>
#include <QHash>

#include "MapObjectVisitor.h"

class Pilot;
class Controller;
class BookedController;
class Client;

class WhazzupData
{
public:
    enum WhazzupType { GENERAL, ATCBOOKINGS };
        
	WhazzupData();
	WhazzupData(QBuffer* buffer, WhazzupType type);
	~WhazzupData();

	// copy constructor and assignment operator
	WhazzupData(const WhazzupData& data);
	WhazzupData& operator=(const WhazzupData& data);
	
	void updateFrom(const WhazzupData& data);
	
	QList<Controller*> activeSectors() const;
	QList<Pilot*> getPilots() const;
	QList<Pilot*> getActivePilots() const;
	QList<Controller*> getControllers() const { return controllers.values(); }
	QList<BookedController*> getBookedControllers() const { return bookedcontrollers; }
	
	Pilot* getPilot(const QString& callsign) const { return pilots[callsign]; }
	Controller* getController(const QString& callsign) const { return controllers[callsign]; }
	
	int clients() const { return connectedClients; }
	int servers() const { return connectedServers; }
	int version() const { return whazzupVersion; }
	const QDateTime& timestamp() const { return whazzupTime; }
	
	bool isIvao() const { return whazzupVersion == 4; }
	bool isVatsim() const { return whazzupVersion == 8; }
	
	void accept(MapObjectVisitor *visitor) const;
	
	bool isNull() const { return whazzupTime.isNull(); }
	
private:
	void assignFrom(const WhazzupData& data);
	
	void updatePilotsFrom(const WhazzupData& data);
	void updateControllersFrom(const WhazzupData& data);
	void updateBookedControllersFrom(const WhazzupData& data);
	
	QHash<QString, Pilot*> pilots;
	QHash<QString, Controller*> controllers;
	QList<BookedController*> bookedcontrollers;
	int connectedClients;
	int connectedServers;
	int whazzupVersion;
	QDateTime whazzupTime;
    WhazzupType datatype;
};

#endif /*WHAZZUPDATA_H_*/
