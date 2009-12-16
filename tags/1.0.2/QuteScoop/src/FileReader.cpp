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
