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
        static Launcher* instance(bool createIfNoInstance = false);
        void fireUp();

    public slots:
        void loadWindow();

        // Wind
        void startWindDownload();

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
        void windProgress(int prog, int total);
        void loadNavdata();

        //Wind
        void startWindDecoding(bool error);

    private:
        Launcher(QWidget *parent = 0);
        ~Launcher();

        //Startupimage
        QPixmap map;
        QLabel *_image, *_text;
        QProgressBar *_progress;
        //required to move the widget
        QPoint _dragPosition;

        //For NavDataUpdate
        void checkForDataUpdates();
        QHttp *_dataVersionsAndFilesDownloader;
        QBuffer *_dataVersionsBuffer;
        QList<QFile*> _dataFilesToDownload;

        // Wind
        QHttp *_windDataDownloader;
        QBuffer *_windDataBuffer;


        bool _navReady, _windowReady, _windReady;
        QTimer _finalTimer, _windowTimer;

};

#endif // LAUNCHER_H
