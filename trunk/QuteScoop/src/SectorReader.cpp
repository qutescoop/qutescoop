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

#include "SectorReader.h"
#include "FileReader.h"
#include "Settings.h"

SectorReader::SectorReader() {
}

SectorReader::~SectorReader() {
}

void SectorReader::loadSectors(QHash<QString, Sector*>& sectors) {
        sectors.clear();
    idIcaoMapping.clear();

        loadSectorlist(sectors);
        loadSectordisplay(sectors, Settings::dataDirectory() + "firdisplay.dat");
    if(Settings::useSupFile())
                loadSectordisplay(sectors, Settings::dataDirectory() + "firdisplay.sup");
}

void SectorReader::loadSectorlist(QHash<QString, Sector*>& sectors) {
    FileReader *fileReader = new FileReader(Settings::dataDirectory() + "firlist.dat");

    QString line = fileReader->nextLine();
    while(!line.isNull()) {
                Sector *sector = new Sector(line.split(':'));
                if(sector->isNull()) {
                        delete sector;
            continue;
        }

                sectors[sector->icao()] = sector;
                idIcaoMapping.insert(sector->id(), sector->icao());
        line = fileReader->nextLine();
    }

    delete fileReader;
}

void SectorReader::loadSectordisplay(QHash<QString, Sector*>& sectors, const QString& filename)
{
    FileReader *fileReader = new FileReader(filename);

        QString workingSectorId;
    QList<QPair<double, double> > pointList;

    QString line = fileReader->nextLine();
    while(!line.isNull() && !fileReader->atEnd()) {
        // DISPLAY_LIST_100
        // 51.08:2.55
        // ...

        if(line.startsWith("DISPLAY_LIST_"))
        {
            if(!workingSectorId.isEmpty())
            {
                QList<QString> SectorIcaos = idIcaoMapping.values(workingSectorId);
                for(int i = 0; i < SectorIcaos.size(); i++)
                {
                if(sectors.contains(SectorIcaos[i])) // be conservative as a segfault was reported on Mac OS
                {
                    sectors[SectorIcaos[i]]->setPointList(pointList);
                }
                }
            }
            workingSectorId = line.right(line.length() - QString("DISPLAY_LIST_").length());
            pointList.clear();

        } else if(!workingSectorId.isEmpty() && !line.isEmpty())
        {
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
