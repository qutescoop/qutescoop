#include "SectorReader.h"

#include "FileReader.h"
#include "helpers.h"
#include "Settings.h"

SectorReader::SectorReader() {}

SectorReader::~SectorReader() {}

void SectorReader::loadSectors(QMultiMap<QString, Sector*> &sectors) {
    sectors.clear();

    loadSectorlist(sectors);
    loadSectordisplay(sectors);
}

void SectorReader::loadSectorlist(QMultiMap<QString, Sector*>& sectors) {
    auto filePath = "data/firlist.dat";
    FileReader* fileReader = new FileReader(Settings::dataDirectory(filePath));

    auto count = 0;
    while (!fileReader->atEnd()) {
        ++count;
        QString line = fileReader->nextLine();
        if (line.isEmpty() || line.startsWith(";")) {
            continue;
        }

        Sector* sector = new Sector(line.split(':'), count);
        if (sector->isNull()) {
            delete sector;
            continue;
        }

        sectors.insert(sector->icao, sector);
    }
    delete fileReader;
}

void SectorReader::loadSectordisplay(QMultiMap<QString, Sector*>& sectors) {
    auto filePath = "data/firdisplay.dat";
    FileReader* fileReader = new FileReader(Settings::dataDirectory(filePath));

    QString workingSectorId;
    QList<QPair<double, double> > pointList;

    unsigned int count = 0;
    unsigned int debugLineWorkingSectorStart = 0;
    while (!fileReader->atEnd()) {
        ++count;
        QString line = fileReader->nextLine();

        if (line.isEmpty() || line.startsWith(";")) {
            continue;
        }

        // DISPLAY_LIST_100 or DISPLAY_LIST_EDFM-APP
        // 51.08:2.55
        // ...
        // DISPLAY_LIST_    // last line is always DISPLAY_LIST_

        if (line.startsWith("DISPLAY_LIST_")) {
            if (!workingSectorId.isEmpty()) { // we are at the end of a section
                if (pointList.size() < 3) {
                    auto msg = QString("While processing lines #%1-%2 from %3: Sector %4 doesn't contain enough points (%5, expected 3+)")
                        .arg(debugLineWorkingSectorStart)
                        .arg(count)
                        .arg(filePath, workingSectorId)
                        .arg(pointList.size());
                    qCritical() << "SectorReader::loadSectordisplay()" << msg;
                    QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
                    exit(EXIT_FAILURE);
                }

                QList<Sector*> sectorsWithMatchingId;
                foreach (const auto sector, sectors) {
                    if (sector->id == workingSectorId) {
                        sectorsWithMatchingId.append(sector);
                    }
                }

                if (sectorsWithMatchingId.size() == 0) {
                    auto msg = QString("While processing line #%1 '%2' from %3: Sector ID %4 is not used in firlist.dat.")
                        .arg(count)
                        .arg(line, filePath)
                        .arg(workingSectorId);
                    qInfo() << "SectorReader::loadSectordisplay()" << msg;

                    // add this pseudo sector to be able to show it in StaticSectorsDialog
                    auto* s = new Sector(
                        {
                            "ZZZZ " + workingSectorId,
                            "not used by any controller",
                            NULL,
                            NULL,
                            NULL,
                            workingSectorId,
                        },
                        -1,
                        debugLineWorkingSectorStart
                    );
                    s->setPoints(pointList);

                    sectors.insert(NULL, s);
                }

                foreach (const auto sector, sectorsWithMatchingId) {
                    if (sector != 0) {
                        sector->setDebugSectorLineNumber(debugLineWorkingSectorStart);
                        sector->setPoints(pointList);
                    }
                }
            }

            // new section starts here
            workingSectorId = line.split('_').last();
            debugLineWorkingSectorStart = count;
            pointList.clear();
        } else if (!workingSectorId.isEmpty()) {
            QStringList latLng = line.split(':');
            if (latLng.size() < 2) {
                continue;
            }
            double lat = latLng[0].toDouble();
            double lon = Helpers::modPositive(latLng[1].toDouble() + 180., 360.) - 180.;
            if (lat > 90. || lat < -90. || lon > 180. || lon < -180. || (qFuzzyIsNull(lat) && qFuzzyIsNull(lon))) {
                auto msg = QString("While processing line #%1 '%2' from %3: Sector id=%4 has invalid point %5:%6")
                    .arg(count)
                    .arg(line, filePath)
                    .arg(workingSectorId).arg(lat).arg(lon);
                qCritical() << "SectorReader::loadSectordisplay()" << msg;
                QTextStream(stdout) << "CRITICAL: " << msg << Qt::endl;
                exit(EXIT_FAILURE);
            }

            pointList.append(QPair<double, double>(lat, lon));
        }
    }
    delete fileReader;
}
