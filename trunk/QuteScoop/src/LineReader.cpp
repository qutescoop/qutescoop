/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/
#include "LineReader.h"

#include <QStringList>

const QList<QPair<double, double> >& LineReader::readLine() {
	currentLine.clear();

	if(atEnd()) return currentLine;

	do {
		QString line = nextLine();
		if(line == "end" || line.isNull()) {
			break;
		}
		
		QStringList list = line.split(':');
		if(list.size() != 2) continue;
		
		bool ok = true;
		double lat = list[0].toDouble(&ok);
		if(!ok) continue;
		double lon = list[1].toDouble(&ok);
		if(!ok) continue;
	
		currentLine.append(QPair<double, double>(lat, lon));
	} while(!atEnd());
	
	return currentLine;
}
