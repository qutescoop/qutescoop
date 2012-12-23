/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/
#ifndef COASTLINEREADER_H_
#define COASTLINEREADER_H_

#include "FileReader.h"

class LineReader: public FileReader
{
    public:
        LineReader(const QString& filename): FileReader(filename) {}
        const QList<QPair<double, double> >& readLine();

    private:
        QList<QPair<double, double> > _currentLine;
};

#endif /*COASTLINEREADER_H_*/
