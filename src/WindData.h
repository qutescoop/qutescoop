#ifndef WINDDATA_H
#define WINDDATA_H


//#include <QThread>
//#include <QMutex>
#include "_pch.h"
#include "FileReader.h"
#include "Settings.h"
#include "Station.h"

class WindData : public QObject
{
        Q_OBJECT
    public:
        static WindData *instance();
        explicit WindData(QObject *parent = 0);

        int status() { return _status;}
        GLuint windArrows( int alt);

        void setRawData(QString);
        void decodeData();

        void refreshLists();


    signals:

    public slots:

    protected:


    private slots:

    private:
        int round (int a, double b);
        GLuint createWindArrowList(int alt);

        QString _rawData;
        QStringList _stationRawData;
        QHash< int , Station> _stationList;
        QList<GLuint> _windList;
        GLuint _result;

        int _status;
        int _mode; // 0 = TTAA; 1 = TTBB;2 =  PPBB; 3 = UNKNOWN
};


#endif // WINDDATA_H
