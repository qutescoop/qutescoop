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

#include "FirReader.h"
#include "FileReader.h"
#include "Settings.h"

FirReader::FirReader() {
}

FirReader::~FirReader() {
}

void FirReader::loadFirs(QHash<QString, Fir*>& firs) {
	firs.clear();
	idIcaoMapping.clear();

	loadFirlist(firs);
	loadFirdisplay(firs, Settings::dataDirectory() + "firdisplay.dat");
	if(Settings::useSupFile())
		loadFirdisplay(firs, Settings::dataDirectory() + "firdisplay.sup");
}

void FirReader::loadFirlist(QHash<QString, Fir*>& firs) {
	FileReader *fileReader = new FileReader(Settings::dataDirectory() + "firlist.dat");

	QString line = fileReader->nextLine();
	while(!line.isNull()) {
		Fir *fir = new Fir(line.split(':'));
		if(fir->isNull()) {
			delete fir;
			continue;
		}

		firs[fir->icao()] = fir;
		idIcaoMapping.insert(fir->id(), fir->icao());
		line = fileReader->nextLine();
	}
	
	delete fileReader;
}

void FirReader::loadFirdisplay(QHash<QString, Fir*>& firs, const QString& filename) {
	FileReader *fileReader = new FileReader(filename);

	QString workingFirId;
	QList<QPair<double, double> > pointList;
	
	QString line = fileReader->nextLine();
	while(!line.isNull() && !fileReader->atEnd()) {
		// DISPLAY_LIST_100
		// 51.08:2.55
		// ...

		if(line.startsWith("DISPLAY_LIST_")) {
			
			if(!workingFirId.isEmpty()) {
				QList<QString> firIcaos = idIcaoMapping.values(workingFirId);
				for(int i = 0; i < firIcaos.size(); i++) {
                    if(firs.contains(firIcaos[i])) { // be conservative as a segfault was reported on Mac OS
                        firs[firIcaos[i]]->setPointList(pointList);
                    }
				}
			}
			
			workingFirId = line.right(line.length() - QString("DISPLAY_LIST_").length());
			pointList.clear();
			
		} else if(!workingFirId.isEmpty()) {
			QStringList points = line.split(':');
			double lat = points[0].toDouble();
			double lon = points[1].toDouble();
			if(lat != 0.0 && lon != 0.0)
				pointList.append(QPair<double, double>(lat, lon));
		}
		
		line = fileReader->nextLine();
	}
	
	delete fileReader;
}
