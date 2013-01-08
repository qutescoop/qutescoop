/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/
#ifndef SECTORREADER_H_
#define SECTORREADER_H_

#include "_pch.h"

#include "Sector.h"

class SectorReader {
    public:
        SectorReader() {}
        ~SectorReader() {}

        void loadSectors(QHash<QString, Sector*>& sectors);
    private:
        void loadSectorlist(QHash<QString, Sector*>& sectors);
        void loadSectordisplay(QHash<QString, Sector*>& sectors, const QString& filename);
        QMultiMap<QString, QString> _idIcaoMapping;
};

#endif /*SECTORREADER_H_*/
