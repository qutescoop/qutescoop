/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef PLANFLIGHTDIALOG_H
#define PLANFLIGHTDIALOG_H

#include "ui_PlanFlightDialog.h"

#include "PlanFlightRoutesModel.h"

#include <QtNetwork>

class PlanFlightDialog : public QDialog, public Ui::PlanFlightDialog {
        Q_OBJECT

    public:
        static PlanFlightDialog *instance(bool createIfNoInstance = true, QWidget *parent = 0);
        void plotPlannedRoute() const;
        Route* selectedRoute;

    signals:
        void fpDownloaded();

    protected:
        void closeEvent(QCloseEvent *event);

    private slots:
        void on_bDestDetails_clicked();
        void on_bDepDetails_clicked();
        void on_pbVatsimPrefile_clicked();
        void on_pbCopyToClipboard_clicked();
        void on_cbPlot_toggled(bool checked);
        void on_edDest_textChanged(QString str);
        void on_edDep_textChanged(QString str);
        void on_buttonRequest_clicked();
        void vrouteDownloaded();
        void routeSelected(const QModelIndex& index);

    private:
        PlanFlightDialog(QWidget *parent);

        void requestGenerated();
        void requestVroute();

        QNetworkReply *_replyVroute;
        QList<Route*> _routes;

        PlanFlightRoutesModel _routesModel;
        QSortFilterProxyModel *_routesSortModel;

        constexpr static char m_preferencesName[] = "planFlightDialog";
};

#endif // PLANFLIGHTDIALOG_H
