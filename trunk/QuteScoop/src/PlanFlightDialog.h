/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef PLANFLIGHTDIALOG_H
#define PLANFLIGHTDIALOG_H

#include "ui_PlanFlightDialog.h"

#include "_pch.h"

#include "PlanFlightRoutesModel.h"

class PlanFlightDialog : public QDialog, public Ui::PlanFlightDialog {
    Q_OBJECT

public:
    static PlanFlightDialog *getInstance(bool createIfNoInstance = true, QWidget *parent = 0);
    void plotPlannedRoute() const;
    Route* selectedRoute;

signals:
    void fpDownloaded();

private slots:
    void on_bDestDetails_clicked();
    void on_bDepDetails_clicked();
    void on_pbVatsimPrefile_clicked();
    void on_pbCopyToClipboard_clicked();
    void on_cbPlot_toggled(bool checked);
    void on_edDest_textChanged(QString str);
    void on_edDep_textChanged(QString str);
    void on_buttonRequest_clicked();
    void vrouteDownloaded(bool error);
    void vatrouteDownloaded(bool error);
    void routeSelected(const QModelIndex& index);

private:
    PlanFlightDialog(QWidget *parent);

    void requestGenerated();
    void requestVroute();
    void requestVatroute();

    QHttp *vrouteDownloader, *vatrouteDownloader;
    QBuffer *vrouteBuffer, *vatrouteBuffer;
    QList<Route*> routes;

    PlanFlightRoutesModel routesModel;
    QSortFilterProxyModel *routesSortModel;
};

#endif // PLANFLIGHTDIALOG_H
