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

#ifndef WINDOW_H_
#define WINDOW_H_

#include <QTimer>
#include <QSortFilterProxyModel>

#include "ui_MainWindow.h" // file generated by UIC from MainWindow.ui
#include "GLWidget.h"
#include "Whazzup.h"
#include "ClientSelectionWidget.h"
#include "PreferencesDialog.h"
#include "PlanFlightDialog.h"
#include "BookedAtcDialog.h"
#include "ListClientsDialog.h"
#include "SearchResultModel.h"
#include "MetarModel.h"

class Window : public QMainWindow, private Ui::MainWindow {

Q_OBJECT

public:
    static Window* getInstance();
    void setStatusText(QString text);
    void setProgressBar(int prog, int tot);
    void setProgressBar(bool isVisible);
    void setEnableBookedAtc(bool enable);
    void setPlotFlightPlannedRoute(bool value);
    void shootScreenie();
    GLWidget *glWidget;

public slots:
    void showOnMap(double lat, double lon);
    void updateMetarDecoder(const QString& airport, const QString& decodedText);
    void refreshFriends();
    void updateGLPilots();

private slots:
    void on_tbZoomOut_clicked();
    void on_tbZoomIn_clicked();

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
    void on_timePredictTime_timeChanged(QTime date);
    void on_datePredictTime_dateChanged(QDate date);
    void on_tbDisablePredict_clicked();
    void on_tbRunPredict_toggled(bool checked);
    void performWarp();
    void runPredict();

    void about();

    void networkMessage(QString message);
    void downloadError(QString message);
    void toggleFullscreen();
    void whazzupDownloaded(bool isNew = true);
    void mapClicked(int x, int y, QPoint absolutePos);
    void openPreferences();
    void openPlanFlight();
    void openBookedAtc();
    void openListClients();

    void on_searchEdit_textChanged(const QString& text);
    void on_actionClearAllFlightPaths_triggered();
    void on_actionDisplayAllFlightPaths_triggered();

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
protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void checkForUpdates();

    void updateTitlebarAfterMove(Qt::DockWidgetArea, QDockWidget *dock);

    // singleton
    Window(QWidget *parent = 0);
    void createActions();

    //QToolBar *toolBar;
    ClientSelectionWidget *clientSelection;

    SearchResultModel searchResultModel, friendsModel;
    QTimer searchTimer, metarTimer, warpTimer, runPredictTimer;
    QTimer downloadWatchdog;
    QSortFilterProxyModel *metarSortModel, *friendsSortModel;
    MetarModel metarModel;

    QHttp *versionChecker;
    QBuffer *versionBuffer;

    QTime timePredictTime_old;
    QDate datePredictTime_old;
};

#endif /*WINDOW_H_*/
