/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "FriendsVisitor.h"
#include "Settings.h"
#include "Client.h"

FriendsVisitor::FriendsVisitor() {
	_friendList = Settings::friends();
}

void FriendsVisitor::visit(MapObject* object) {
	Client *c = dynamic_cast<Client*>(object);
	if(c != 0) {
		if(_friendList.contains(c->userId))
			_friends.append(c);
	}
}
