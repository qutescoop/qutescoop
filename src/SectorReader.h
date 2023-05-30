#ifndef SECTORREADER_H_
#define SECTORREADER_H_

#include "Sector.h"

class SectorReader {
    public:
        SectorReader() {}
        ~SectorReader() {}

        void loadSectors(QMultiMap<QString, Sector*>& sectors);
    private:
        void loadSectorlist(QMultiMap<QString, Sector*>& sectors);
        void loadSectordisplay(QMultiMap<QString, Sector*>& sectors);
};

#endif /*SECTORREADER_H_*/
