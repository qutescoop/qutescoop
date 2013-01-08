/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WINDOW_H_
#define WINDOW_H_

#include "ui_Window.h"

#include "_pch.h"

#include "MapScreen.h"
#include "SearchResultModel.h"
#include "MetarModel.h"
#include "FileReader.h"

class Window : public QMainWindow, public Ui::Window {
        Q_OBJECT
    public:
        static Window* instance(bool createIfNoInstance = true);
        void setEnableBookedAtc(bool enable);
        void shootScreenshot();
        MapScreen *mapScreen;
        //GLWidget *glWidget;

    public slots:
        void updateMetarDecoder(const QString& airport, const QString& decodedText);
        void refreshFriends();
        void on_actionShowRoutes_triggered(bool checked);
        void startCloudDownload();
        void whazzupDownloaded(bool isNew = true);
        void restore();

    private slots:
        void on_actionShowWaypoints_triggered(bool checked);
        void on_actionDebugLog_triggered();
        void on_actionHighlight_Friends_triggered(bool checked);
        void on_pb_highlightFriends_toggled(bool checked);

        void on_actionZoomReset_triggered();
        void on_tbZoomOut_clicked();
        void on_tbZoomIn_clicked();
        void on_tbZoomOut_customContextMenuRequested(QPoint pos);
        void on_tbZoomIn_customContextMenuRequested(QPoint pos);

        void on_actionMoveDown_triggered();
        void on_actionMoveUp_triggered();
        void on_actionMoveRight_triggered();
        void on_actionMoveLeft_triggered();

        void on_actionRememberMapPosition9_triggered();
        void on_actionRememberMapPosition1_triggered();
        void on_actionRememberMapPosition2_triggered();
        void on_actionRememberMapPosition3_triggered();
        void on_actionRememberMapPosition4_triggered();
        void on_actionRememberMapPosition5_triggered();
        void on_actionRememberMapPosition6_triggered();
        void on_actionRememberMapPosition7_triggered();
        void on_actionRecallMapPosition9_triggered();
        void on_actionRecallMapPosition1_triggered();
        void on_actionRecallMapPosition2_triggered();
        void on_actionRecallMapPosition3_triggered();
        void on_actionRecallMapPosition4_triggered();
        void on_actionRecallMapPosition5_triggered();
        void on_actionRecallMapPosition6_triggered();
        void on_actionRecallMapPosition7_triggered();

        void on_actionHideAllWindows_triggered();

        void on_actionPredict_toggled(bool );
        void on_dateTimePredict_dateTimeChanged(QDateTime date);
        void on_tbDisablePredict_clicked();
        void on_tbRunPredict_toggled(bool checked);
        void on_cbUseDownloaded_toggled(bool checked);
        void on_cbOnlyUseDownloaded_toggled(bool checked);
        void performWarp(bool forceUseDownloaded = false);
        void runPredict();

        void about();

        void toggleFullscreen();
        void openPreferences();
        void openPlanFlight();
        void openBookedAtc();
        void openListClients();
        void openSectorView();

        void on_searchEdit_textChanged(const QString& text);

        void on_metarEdit_textChanged(const QString& text);
        void on_btnRefreshMetar_clicked();

        void performSearch();
        void updateMetars();
        void metarDoubleClicked(const QModelIndex& index);
        void metarDockMoved(Qt::DockWidgetArea area);
        void searchDockMoved(Qt::DockWidgetArea area);
        void metarDecoderDockMoved(Qt::DockWidgetArea area);
        void friendsDockMoved(Qt::DockWidgetArea area);

        void friendClicked(const QModelIndex& index);
        void friendDoubleClicked(const QModelIndex& index);

        //void versionDownloaded(bool error);
        void downloadWatchdogTriggered();

        void allSectorsChanged(bool);

        void cloudDownloadFinished(bool error);

        void on_actionChangelog_triggered();
    protected:
        virtual void closeEvent(QCloseEvent *event);
    private:
        // singleton
        Window(QWidget *parent = 0);
        ~Window();
        void updateTitlebarAfterMove(Qt::DockWidgetArea, QDockWidget *dock);
        //void checkForUpdates();

        SearchResultModel _modelSearchResult, _modelFriends;
        QTimer _timerSearch, _timerMetar, _timerEditPredict, _timerRunPredict,
        _timerWhazzup, _timerCloud;
        QSortFilterProxyModel *_sortmodelMetar, *_sortmodelFriends;
        MetarModel _metarModel;
        QHttp *_cloudDownloader;
        QBuffer *_cloudBuffer;
        QDateTime _dateTimePredict_old;
        QLabel *_lblStatus;
        QProgressBar *_progressBar;
};

#endif /*WINDOW_H_*/
