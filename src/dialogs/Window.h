#ifndef WINDOW_H_
#define WINDOW_H_

#include "ui_Window.h"

#include "../MapScreen.h"
#include "../models/SearchResultModel.h"
#include "../models/MetarModel.h"

class Window
    : public QMainWindow, public Ui::Window {
    Q_OBJECT
    public:
        static Window* instance(bool createIfNoInstance = true);
        void setEnableBookedAtc(bool enable);
        MapScreen* mapScreen;
    public slots:
        void refreshFriends();
        void processWhazzup(bool isNew = true);
        void restore();
    signals:
        void restored();
    private slots:
        void actionShowRoutes_triggered(bool checked, bool showStatus = true);
        void actionShowImmediateRoutes_triggered(bool checked, bool showStatus = true);
        void actionHideLabels_triggered(bool checked, bool showStatus = true);
        void showInactiveAirports(bool value);
        void on_actionShowWaypoints_triggered(bool checked);
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
        void on_actionPredict_toggled(bool);
        void on_tbDisablePredict_clicked();
        void on_tbRunPredict_toggled(bool checked);
        void on_cbUseDownloaded_toggled(bool checked);
        void on_cbOnlyUseDownloaded_toggled(bool checked);
        void dateTimePredict_dateTimeChanged(QDateTime date);
        void performWarp();
        void runPredict();
        void about();
        void toggleFullscreen();
        void openPreferences();
        void openPlanFlight();
        void openBookedAtc();
        void openListClients();
        void openStaticSectorsDialog();
        void on_searchEdit_textChanged(const QString& text);
        void on_metarEdit_textChanged(const QString& text);
        void on_btnRefreshMetar_clicked();
        void performSearch();
        void updateMetars();
        void metarClicked(const QModelIndex& index);
        void metarDockMoved(Qt::DockWidgetArea area);
        void searchDockMoved(Qt::DockWidgetArea area);
        void friendsDockMoved(Qt::DockWidgetArea area);
        void friendClicked(const QModelIndex& index);
        void downloadWatchdogTriggered();
    protected:
        virtual void closeEvent(QCloseEvent* event) override;
    private:
        // singleton
        Window(QWidget* parent = 0);
        ~Window();
        void updateTitlebarAfterMove(Qt::DockWidgetArea, QDockWidget* dock);

        SearchResultModel _modelSearchResult, _modelFriends;
        QTimer _timerSearch, _timerMetar, _timerEditPredict, _timerRunPredict, _timerWatchdog;
        QSortFilterProxyModel* _sortmodelMetar, * _sortmodelFriends;
        MetarModel _metarModel;
        QDateTime _dateTimePredict_old;
        QLabel* _lblStatus;
        QProgressBar* _progressBar;
};

#endif /*WINDOW_H_*/
