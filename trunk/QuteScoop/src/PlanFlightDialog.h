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
    static PlanFlightDialog *getInstance(bool createIfNoInstance = true, QWidget *parent = 0);
    void plotPlannedRoute() const;

signals:
    void fpDownloaded();

private slots:
//    void on_textRoute_textChanged();
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

    Route* selectedRoute;

    PlanFlightRoutesModel routesModel;
    QSortFilterProxyModel *routesSortModel;
};

#endif // PLANFLIGHTDIALOG_H