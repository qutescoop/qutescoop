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

#include "FriendsVisitor.h"
#include "Settings.h"
#include "Client.h"

FriendsVisitor::FriendsVisitor()
{
	friendList = Settings::friends();
}

void FriendsVisitor::visit(MapObject* object) {
	Client *c = dynamic_cast<Client*>(object);
	if(c != 0) {
		if(friendList.contains(c->userId))
			friends.append(c);
	}
}
