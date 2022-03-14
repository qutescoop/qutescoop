/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "SectorReader.h"

#include "FileReader.h"
#include "Settings.h"
#include "helpers.h"

void SectorReader::loadSectors(QHash<QString, Sector*>& sectors) {
    sectors.clear();
    _idIcaoMapping.clear();

    loadSectorlist(sectors);

    loadSectordisplay(sectors, Settings::dataDirectory("data/firdisplay.dat"));
}

void SectorReader::loadSectorlist(QHash<QString, Sector*>& sectors) {
    FileReader *fileReader = new FileReader(Settings::dataDirectory("data/firlist.dat"));

    QString line = fileReader->nextLine();
    while(!line.isNull()) {
        Sector *sector = new Sector(line.split(':'));
        if(sector->isNull()) {
            delete sector;
            continue;
        }
        sectors[sector->icao] = sector;
        _idIcaoMapping.insert(sector->id, sector->icao);
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
                QList<QString> sectorIcaos = _idIcaoMapping.values(workingSectorId);
                for(int i = 0; i < sectorIcaos.size(); i++) {
                    auto sector = sectors[sectorIcaos[i]];
                    if(sector != 0) {
                        if(pointList.size() < 3) {
                            auto msg = QString("Sector %1 (%2) doesn't contain enough points (%3)")
                                .arg(sector->name, sector->icao).arg(pointList.size());
                            qCritical() << "Sector::getCenter()" << msg;
                            QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
                            exit(EXIT_FAILURE);
                        }

                        sector->setPoints(pointList);
                    }
                }
            }
            workingSectorId = line.split('_').last();
            pointList.clear();
        } else if(!workingSectorId.isEmpty() && !line.isEmpty()) {
            QStringList points = line.split(':');
            if(points.size() < 2) continue;
            double lat = points[0].toDouble();
            double lon = modPositive(points[1].toDouble() + 180., 360.) - 180.;
            if (lat > 90. || lat < -90. || lon > 180. || lon < -180. || (qFuzzyIsNull(lat) && qFuzzyIsNull(lon))) {
                auto msg = QString("Sector id=%1 has invalid point %2:%3")
                    .arg(workingSectorId).arg(lat).arg(lon);
                qCritical() << "Sector::getCenter()" << msg;
                QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
                exit(EXIT_FAILURE);
            }

            if(!qFuzzyIsNull(lat) || !qFuzzyIsNull(lon))
                pointList.append(QPair<double, double>(lat, lon));
        }
    }
    delete fileReader;
}
