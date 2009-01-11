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

#ifndef METAR_H_
#define METAR_H_

#include <QString>
#include <QDateTime>

class Metar
{
public:
	Metar(const QString& encodedString = QString());
	
	bool isNull() const;
	bool isValid() const;
	bool doesNotExist() const;
	bool needsRefresh() const;
	
	QString encoded;
	QDateTime downloaded;
	QString decodedHtml() const;
	
private:
	QString decodeDate(QStringList& tokens) const;
	QString decodeWind(QStringList& tokens) const;
	QString decodeVisibility(QStringList& tokens) const;
	QString decodeSigWX(QStringList& tokens) const;
	QString decodeClouds(QStringList& tokens) const;
	QString decodeTemp(QStringList& tokens) const;
	QString decodeQNH(QStringList& tokens) const;
	QString decodePrediction(QStringList& tokens) const;
};

#endif /*METAR_H_*/
