/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/
#ifndef FILEREADER_H_
#define FILEREADER_H_

#include <QFile>
#include <QString>
#include <QList>
#include <QPair>
#include <QTextStream>

class FileReader
{
public:
	FileReader(const QString& filename);
	virtual ~FileReader();
	
	bool atEnd() const;
	QString nextLine();
	
private:
	QFile *file;
	QTextStream *stream;
};

#endif /*FILEREADER_H_*/
