/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef AIRPORTDETAILS_H_
#define AIRPORTDETAILS_H_

#include "ui_AirportDetails.h"

#include "_pch.h"

#include "ClientDetails.h"
#include "Airport.h"
#include "MetarModel.h"
#include "Whazzup.h"
#include "Settings.h"

#include "AirportDetailsAtcModel.h"
#include "AirportDetailsArrivalsModel.h"
#include "AirportDetailsDeparturesModel.h"

class AirportDetails : public ClientDetails, private Ui::AirportDetails {
        Q_OBJECT
    public:
        static AirportDetails *instance(bool createIfNoInstance = true, QWidget *parent = 0);
        void destroyInstance();
        void refresh(Airport* _airport = 0);

    protected:
        void closeEvent(QCloseEvent *event);

    private slots:
        void onGotMetar(const QString &airportLabel, const QString &encoded, const QString &humanHtml);
        void on_pbMetar_clicked();
        void on_cbAtis_toggled(bool checked);
        void on_cbObservers_toggled(bool checked);
        void on_cbPlotRoutes_toggled(bool checked);
        void atcSelected(const QModelIndex &index);
        void arrivalSelected(const QModelIndex &index);
        void departureSelected(const QModelIndex &index);

    private:
        AirportDetails(QWidget *parent);

        AirportDetailsAtcModel _atcModel;
        AirportDetailsArrivalsModel _arrivalsModel;
        AirportDetailsDeparturesModel _departuresModel;
        Airport* _airport;
        QSortFilterProxyModel *_atcSortModel, *_arrivalsSortModel, *_departuresSortModel;

        QSet<Controller*> checkSectors() const;

        MetarModel* _metarModel;
};
#endif /*AIRPORTDETAILS_H_*/
