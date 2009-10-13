/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
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
    static PlanFlightDialog *getInstance();
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

    void requestGenerated();
    void requestVroute();

    QHttp *fpDownloader;
    QBuffer *fpBuffer;
    QList<Route*> routes;

    Route* selectedRoute;

    PlanFlightRoutesModel routesModel;
    QSortFilterProxyModel *routesSortModel;
};

#endif // PLANFLIGHTDIALOG_H
