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
#ifndef SECTORREADER_H_
#define SECTORREADER_H_

#include <QHash>
#include <QString>
#include <QMultiMap>

#include "Sector.h"

class SectorReader
{
public:
        SectorReader();
        ~SectorReader();
	
        void loadSectors(QHash<QString, Sector*>& sectors);
	
private:
        void loadSectorlist(QHash<QString, Sector*>& sectors);
        void loadSectordisplay(QHash<QString, Sector*>& sectors, const QString& filename);
	
	QMultiMap<QString, QString> idIcaoMapping;
	
};

#endif /*SECTORREADER_H_*/
