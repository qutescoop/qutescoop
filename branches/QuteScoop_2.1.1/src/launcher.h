#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "_pch.h"

#include "Window.h"
#include "NavData.h"
#include "FileReader.h"
#include "Whazzup.h"
#include "Settings.h"
#include "GuiMessage.h"

class Launcher : public QWidget
{
    Q_OBJECT

public:
    Launcher(QWidget *parent = 0);
    ~Launcher();
    void fireUp();

public slots:
    void loadWindow();

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void keyReleaseEvent(QKeyEvent *event);

signals:
    void navdataUpdated();

private slots:
    // NavdataUpdate update
    void dataVersionsDownloaded(bool error);
    void dataFilesRequestFinished(int id, bool error);
    void dataFilesDownloaded(bool error);
    void loadNavdata();

    //Wind
    void startWindDecoding(bool error);

private:

    //Startupimage
    QPixmap map;
    QLabel *image;
    QLabel *text;
    //required to move the widget
    QPoint dragPosition;

    //For NavDataUpdate
    void checkForDataUpdates();
    QHttp *dataVersionsAndFilesDownloader;
    QBuffer *dataVersionsBuffer;
    QList<QFile*> dataFilesToDownload;

    // Wind
    void startWindDownload();
    QHttp *windDataDownloader;
    QBuffer *windDataBuffer;


    bool navReady, windowReady, windReady;
    QTimer finalTimer,windowTimer;

};

#endif // LAUNCHER_H
