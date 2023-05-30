#include "FileReader.h"

FileReader::FileReader(const QString& filename) {
    _stream = 0;
    _file = new QFile(filename);
    if(!_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        delete _file;
        _file = 0;
        return;
    }
    _stream = new QTextStream(_file);
}

FileReader::~FileReader() {
    if(_file != 0) {
        delete _file;
    }
    if(_stream != 0) {
        delete _stream;
    }
}

QString FileReader::nextLine() const {
    if(_stream == 0 || _stream->atEnd()) {
        return QString();
    }
    return _stream->readLine();
}

bool FileReader::atEnd() const {
    if(_stream == 0) {
        return true;
    }

    return _stream->atEnd();
}
