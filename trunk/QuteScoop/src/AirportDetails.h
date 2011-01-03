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

#ifndef AIRPORTDETAILS_H_
#define AIRPORTDETAILS_H_

#include <QSortFilterProxyModel>

#include "ui_AirportDetails.h"
#include "ClientDetails.h"
#include "Airport.h"
#include "MetarModel.h"
#include "Whazzup.h"


#include "AirportDetailsAtcModel.h"
#include "AirportDetailsArrivalsModel.h"
#include "AirportDetailsDeparturesModel.h"

class AirportDetails : public ClientDetails, private Ui::AirportDetails
{
    Q_OBJECT

public:
    static AirportDetails *getInstance(bool createIfNoInstance = true);
    void refresh(Airport* airport = 0);

private slots:
    void on_pbMetar_clicked();
    void on_cbAtis_toggled(bool checked);
    void on_cbObservers_toggled(bool checked);
    void on_cbPlotRoutes_toggled(bool checked);
    void atcSelected(const QModelIndex& index);
    void arrivalSelected(const QModelIndex& index);
    void departureSelected(const QModelIndex& index);

private:
    AirportDetails();

    AirportDetailsAtcModel atcModel;
    AirportDetailsArrivalsModel arrivalsModel;
    AirportDetailsDeparturesModel departuresModel;
    Airport* airport;
    QSortFilterProxyModel *atcSortModel;
    QSortFilterProxyModel *arrivalsSortModel;
    QSortFilterProxyModel *departuresSortModel;

    //test for sectorcheck
    QList<Controller*> checkSectors();

    MetarModel* metarModel;
};
#endif /*AIRPORTDETAILS_H_*/
