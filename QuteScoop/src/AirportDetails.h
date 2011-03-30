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


#include "AirportDetailsAtcModel.h"
#include "AirportDetailsArrivalsModel.h"
#include "AirportDetailsDeparturesModel.h"

class AirportDetails : public ClientDetails, private Ui::AirportDetails {
    Q_OBJECT
public:
    static AirportDetails *getInstance(bool createIfNoInstance = true, QWidget *parent = 0);
    void destroyInstance();
    void refresh(Airport* airport = 0);

private slots:
    void on_pbMetar_clicked();
    void on_cbAtis_toggled(bool checked);
    void on_cbObservers_toggled(bool checked);
    void on_cbPlotRoutes_toggled(bool checked);
    void atcSelected(const QModelIndex &index);
    void arrivalSelected(const QModelIndex &index);
    void departureSelected(const QModelIndex &index);

private:
    AirportDetails(QWidget *parent);

    AirportDetailsAtcModel atcModel;
    AirportDetailsArrivalsModel arrivalsModel;
    AirportDetailsDeparturesModel departuresModel;
    Airport* airport;
    QSortFilterProxyModel *atcSortModel, *arrivalsSortModel, *departuresSortModel;

    QSet<Controller*> checkSectors() const;

    MetarModel* metarModel;
};
#endif /*AIRPORTDETAILS_H_*/
