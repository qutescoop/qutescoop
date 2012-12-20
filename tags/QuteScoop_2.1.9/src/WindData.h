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
    static WindData *getInstance();
    explicit WindData(QObject *parent = 0);

    int getStatus() { return status;}
    GLuint getWindArrows( int alt);

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

    QString rawData;
    QStringList stationRawData;
    QHash< int , Station> stationList;
    QList<GLuint> windList;
    GLuint result;

    int status;
    int mode; // 0 = TTAA; 1 = TTBB;2 =  PPBB; 3 = UNKNOWN
};


#endif // WINDDATA_H
