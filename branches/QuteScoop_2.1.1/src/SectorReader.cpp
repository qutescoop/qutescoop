/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "SectorReader.h"

#include "FileReader.h"
#include "Settings.h"

void SectorReader::loadSectors(QHash<QString, Sector*>& sectors) {
    sectors.clear();
    idIcaoMapping.clear();

    loadSectorlist(sectors);
    loadSectordisplay(sectors, Settings::applicationDataDirectory("data/firdisplay.dat"));
    if(Settings::useSupFile())
        loadSectordisplay(sectors, Settings::applicationDataDirectory("data/firdisplay.sup"));
}

void SectorReader::loadSectorlist(QHash<QString, Sector*>& sectors) {
    FileReader *fileReader = new FileReader(Settings::applicationDataDirectory("data/firlist.dat"));

    QString line = fileReader->nextLine();
    while(!line.isNull()) {
        Sector *sector = new Sector(line.split(':'));
        if(sector->isNull()) {
            delete sector;
            continue;
        }
        sectors[sector->icao] = sector;
        idIcaoMapping.insert(sector->id, sector->icao);
        line = fileReader->nextLine();
    }
    delete fileReader;
}

void SectorReader::loadSectordisplay(QHash<QString, Sector*>& sectors, const QString& filename) {
    FileReader *fileReader = new FileReader(filename);

    QString workingSectorId;
    QList<QPair<double, double> > pointList;

    while(!fileReader->atEnd()) {
        QString line = fileReader->nextLine();
        // DISPLAY_LIST_100
        // 51.08:2.55
        // ...

        if(line.startsWith("DISPLAY_LIST_")) {
            if(!workingSectorId.isEmpty()) {
                QList<QString> sectorIcaos = idIcaoMapping.values(workingSectorId);
                for(int i = 0; i < sectorIcaos.size(); i++) {
                    if(sectors.value(sectorIcaos[i]) != 0) // be conservative as a segfault was reported on Mac OS
                        sectors[sectorIcaos[i]]->points = pointList;
                }
            }
            workingSectorId = line.split('_').last();
            pointList.clear();
        } else if(!workingSectorId.isEmpty() && !line.isEmpty()) {
            QStringList points = line.split(':');
            double lat = points[0].toDouble();
            double lon = points[1].toDouble();
            if(lat != 0. || lon != 0.)
                pointList.append(QPair<double, double>(lat, lon));
        }
    }
    delete fileReader;
}
