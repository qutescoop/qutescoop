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

#include <QDebug>

#include "Whazzup.h"
#include "Client.h"
#include "Settings.h"

Client::Client(const QStringList& stringList, const WhazzupData* whazzup)
{
	label = getField(stringList, 0);
	userId = getField(stringList, 1);
	realName = getField(stringList, 2);
	lat = getField(stringList, 5).toDouble();
	lon = getField(stringList, 6).toDouble();
	server = getField(stringList, 14);
	protrevision = getField(stringList, 15).toInt();
	rating = getField(stringList, 16).toInt();
	timeConnected = QDateTime::fromString(getField(stringList, 37), "yyyyMMddhhmmss");
	timeConnected.setTimeSpec(Qt::UTC);
	
	if(whazzup->isIvao()) {
		clientSoftware = getField(stringList, 38); // IVAO only
		clientVersion = getField(stringList, 39); // IVAO only
		adminRating = getField(stringList, 40).toInt(); // IVAO only
		rating = getField(stringList, 41).toInt(); // IVAO only
	}
	
	if(whazzup->isVatsim()) {
		network = VATSIM;
	} else if(whazzup->isIvao()){
		network = IVAO;
	} else {
		network = OTHER;
	}
	
	// un-fuck user names. people enter all kind of stuff here
	realName.remove(QRegExp("[_\\-\\d\\.\\,\\;\\:\\#\\+\\(\\)]"));
	realName = realName.trimmed();
	
	if(realName.contains(QRegExp("\\b[A-Z]{4}$"))) {
		homeBase = realName.right(4);
		realName = realName.left(realName.length() - 4).trimmed();
	}
}

QString Client::getField(const QStringList& list, int index) {
	if(index < 0 || index >= list.size())
		return QString();
	
	return list[index];
}

QString Client::onlineTime() const {
	if (timeConnected.isNull()) 
		return QString("not connected");
	int secondsOnline = timeConnected.secsTo(Whazzup::getInstance()->whazzupData().timestamp());
	int minutesOnline = secondsOnline / 60;
	int hoursOnline = secondsOnline / 3600;
	QTime result = QTime(hoursOnline % 24, minutesOnline % 60, secondsOnline % 60);
	return result.toString("HH:mm");
}

QString Client::displayName(bool withLink) const {
	QString result = realName;
	QString clientRank = rank();
	if(!clientRank.isEmpty())
		result += " (" + clientRank + ")";
	
	if(withLink && !userId.isEmpty()) {
		QString link = Whazzup::getInstance()->getUserLink(userId);
		if(link.isEmpty())
			return result;
		result = QString("<a href='%1'>%2</a>").arg(link).arg(result);
	}
	
	return result;
}

QString Client::clientInformation() const {
	if(!clientSoftware.isEmpty())
		return clientSoftware + " " + clientVersion;
	return QString();
}

QString Client::detailInformation() const {
	if(!homeBase.isEmpty()) {
		return "Home: " + homeBase;
	}
	return QString();
}

bool Client::matches(const QRegExp& regex) const {
	if(realName.contains(regex)) return true;
	if(userId.contains(regex)) return true;
	return MapObject::matches(regex);
}

QString Client::toolTip() const {
	QString r = rank();
	QString result = label + " (" + realName;
	if(!r.isEmpty()) result += ", " + r;
	result += ")";
	return result;
}

bool Client::isFriend() const {
	return Settings::friends().contains(userId);
}
