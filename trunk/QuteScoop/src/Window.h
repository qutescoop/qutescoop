/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef WINDOW_H_
#define WINDOW_H_

#include "ui_MainWindow.h"

#include "_pch.h"

#include "GLWidget.h"
#include "SearchResultModel.h"
#include "MetarModel.h"
#include "GuiMessage.h"



class Window : public QMainWindow, private Ui::MainWindow {

Q_OBJECT

public:
    static Window* getInstance(bool createIfNoInstance = false);
    //~Window();
    void setEnableBookedAtc(bool enable);
    void shootScreenshot();
    GLWidget *glWidget;

public slots:
    void updateMetarDecoder(const QString& airport, const QString& decodedText);
    void refreshFriends();

    // the message system
    void showGuiMessage(QString msg, GuiMessage::GuiMessageType msgType = GuiMessage::Temporary, QString id = QString(), int progress = 0, int total = 0);

private slots:
    void on_actionShowRoutes_triggered(bool checked);
    void on_actionDebugLog_triggered();

    void on_actionZoomReset_triggered();
    void on_tbZoomOut_clicked();
    void on_tbZoomIn_clicked();
    void on_tbZoomOut_customContextMenuRequested(QPoint pos);
    void on_tbZoomIn_customContextMenuRequested(QPoint pos);

    void on_actionMoveDown_triggered();
    void on_actionMoveUp_triggered();
    void on_actionMoveRight_triggered();
    void on_actionMoveLeft_triggered();

    void on_actionRememberPosition_triggered();
    void on_actionRememberMapPosition2_triggered();
    void on_actionRememberMapPosition3_triggered();
    void on_actionRememberMapPosition4_triggered();
    void on_actionRememberMapPosition5_triggered();
    void on_actionRememberMapPosition6_triggered();
    void on_actionRememberMapPosition7_triggered();

    void on_actionRecallMapPosition_triggered();
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
    void performWarp(bool forceUseDownloaded = false);
    void runPredict();

    void about();

    void toggleFullscreen();
    void whazzupDownloaded(bool isNew = true);
    void openPreferences();
    void openPlanFlight();
    void openBookedAtc();
    void openListClients();

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

    void versionDownloaded(bool error);
    void downloadWatchdogTriggered();

    // datafiles update
    void dataVersionsDownloaded(bool error);
    void dataFilesRequestFinished(int id, bool error);
    void dataFilesDownloaded(bool error);

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void checkForUpdates();
    void checkForDataUpdates();

    void updateTitlebarAfterMove(Qt::DockWidgetArea, QDockWidget *dock);

    // singleton
    Window(QWidget *parent = 0);
    void createActions();

    SearchResultModel searchResultModel, friendsModel;
    QTimer searchTimer, metarTimer, editPredictTimer, runPredictTimer;
    QTimer downloadWatchdog;
    QSortFilterProxyModel *metarSortModel, *friendsSortModel;
    MetarModel metarModel;

    QHttp *versionChecker;
    QBuffer *versionBuffer;

    QHttp *dataVersionsAndFilesDownloader;
    QBuffer *dataVersionsBuffer;
    QList<QFile*> dataFilesToDownload;

    QDateTime dateTimePredict_old;

    QLabel *lblStatus;
    QProgressBar *progressBar;

    QMultiMap< QDateTime, GuiMessage > guiMessages;
};

#endif /*WINDOW_H_*/
