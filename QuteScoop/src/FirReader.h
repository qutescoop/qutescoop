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
#ifndef FIRREADER_H_
#define FIRREADER_H_

#include <QString>
#include <QMultiMap>

#include "Fir.h"

class FirReader
{
public:
	FirReader();
	~FirReader();
	
	void loadFirs(QHash<QString, Fir*>& firs);
	
private:
	void loadFirlist(QHash<QString, Fir*>& firs);
	void loadFirdisplay(QHash<QString, Fir*>& firs, const QString& filename);
	
	QMultiMap<QString, QString> idIcaoMapping;
	
};

#endif /*FIRREADER_H_*/
