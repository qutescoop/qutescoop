/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef PLANFLIGHTDIALOG_H
#define PLANFLIGHTDIALOG_H

#include <QStringList>
#include <QList>
#include <QHttp>
#include <QBuffer>
#include <QSortFilterProxyModel>
#include <QItemDelegate>

#include <QtCore>

#include <QtGui/QDialog>

#include "ui_PlanFlightDialog.h"
#include "PlanFlightRoutesModel.h"

class PlanFlightDialog : public QDialog, private Ui::PlanFlightDialog {
    Q_OBJECT

public:
    static PlanFlightDialog *getInstance(bool createIfNoInstance = true);
    void plotPlannedRoute() const;

signals:
    void networkMessage(QString message);
    void downloadError(QString message);
    void fpDownloaded();

private slots:
    //void on_textRoute_textChanged();
    void on_cbPlot_toggled(bool checked);
    void on_edDest_textChanged(QString str);
    void on_edDep_textChanged(QString str);
    void on_buttonRequest_clicked();
    void fpDownloaded(bool error);
    void fpDownloading(int prog, int tot);
    void routeSelected(const QModelIndex& index);

private:
    PlanFlightDialog();

    //void requestGenerated();
    void requestVroute();

    QHttp *fpDownloader;
    QBuffer *fpBuffer;
    QList<Route*> routes;

    Route* selectedRoute;

    PlanFlightRoutesModel routesModel;
    QSortFilterProxyModel *routesSortModel;
};

#endif // PLANFLIGHTDIALOG_H
