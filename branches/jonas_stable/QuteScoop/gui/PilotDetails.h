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

#ifndef PILOTDETAILS_H_
#define PILOTDETAILS_H_

#include "ui_PilotDetails.h" // file generated by UIC from PilotDetails.ui

#include "ClientDetails.h"
#include "Pilot.h"

class PilotDetails : public ClientDetails, private Ui::PilotDetails
{
	Q_OBJECT
	
public:
	static PilotDetails* getInstance();
	
	virtual void refresh(Pilot *pilot = 0);

private slots:
	void on_buttonDest_clicked();
	void on_buttonFrom_clicked();
	void on_buttonAddFriend_clicked();
	
private:
	PilotDetails();

	Pilot *pilot;
};

#endif /*PILOTDETAILS_H_*/
