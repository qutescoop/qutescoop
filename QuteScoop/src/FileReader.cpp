/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "FileReader.h"

FileReader::FileReader(const QString& filename)
{
	stream = 0;
	file = new QFile(filename);
	if(!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		delete file;
		file = 0;
		return;
	}
	
	stream = new QTextStream(file);
}

FileReader::~FileReader()
{
	if(file != 0) {
		delete file;
	}
	if(stream != 0) {
		delete stream;
	}
}

QString FileReader::nextLine() {
	if(stream == 0)
		return QString();
	
	if(stream->atEnd())
		return QString();

	return stream->readLine();
}

bool FileReader::atEnd() const {
	if(stream == 0)
		return true;
	
	return stream->atEnd();
}
