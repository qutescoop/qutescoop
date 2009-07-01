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

#include "AirportDetails.h"
#include <QLocale>
#include <QHeaderView>

#include "helpers.h"
#include "Whazzup.h"
#include "NavData.h"
#include "Window.h"

AirportDetails *airportDetailsInstance = 0;

AirportDetails *AirportDetails::getInstance() {
	if(airportDetailsInstance == 0)
		airportDetailsInstance = new AirportDetails();
	return airportDetailsInstance;
}

AirportDetails::AirportDetails():
	ClientDetails(),
	airport(0)
{
	setupUi(this);
	
	connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
	connect(this, SIGNAL(showOnMap(double, double)), Window::getInstance(), SLOT(showOnMap(double, double)));

	// ATC list
	atcSortModel = new QSortFilterProxyModel;
	atcSortModel->setDynamicSortFilter(true);
	atcSortModel->setSourceModel(&atcModel);
	treeAtc->setModel(atcSortModel);
    treeAtc->sortByColumn(1, Qt::AscendingOrder);
    treeAtc->header()->setResizeMode(QHeaderView::Interactive);

    connect(treeAtc->header(), SIGNAL(sectionClicked(int)), treeAtc, SLOT(sortByColumn(int)));
    connect(treeAtc, SIGNAL(clicked(const QModelIndex&)), this, SLOT(atcSelected(const QModelIndex&)));

	// arrivals
	arrivalsSortModel = new QSortFilterProxyModel;
	arrivalsSortModel->setDynamicSortFilter(true);
	arrivalsSortModel->setSourceModel(&arrivalsModel);
	treeArrivals->setModel(arrivalsSortModel);
    treeArrivals->sortByColumn(8, Qt::AscendingOrder);
    treeArrivals->header()->setResizeMode(QHeaderView::Interactive);

    connect(treeArrivals->header(), SIGNAL(sectionClicked(int)), treeArrivals, SLOT(sortByColumn(int)));
    connect(treeArrivals, SIGNAL(clicked(const QModelIndex&)), this, SLOT(arrivalSelected(const QModelIndex&)));

	// departures
	departuresSortModel = new QSortFilterProxyModel;
	departuresSortModel->setDynamicSortFilter(true);
	departuresSortModel->setSourceModel(&departuresModel);
    departuresSortModel->sort(7, Qt::AscendingOrder);
	treeDepartures->setModel(departuresSortModel);
    treeDepartures->sortByColumn(7, Qt::AscendingOrder);
    treeDepartures->header()->setResizeMode(QHeaderView::Interactive);
	
    connect(treeDepartures->header(), SIGNAL(sectionClicked(int)), treeDepartures, SLOT(sortByColumn(int)));
    connect(treeDepartures, SIGNAL(clicked(const QModelIndex&)), this, SLOT(departureSelected(const QModelIndex&)));

    refresh();
}

void AirportDetails::refresh(Airport* newAirport) {
	if(newAirport != 0) airport = newAirport;
	if(airport == 0) return;
	setMapObject(airport);
	
	setWindowTitle(airport->toolTip());

    lblName->setText(QString("%1\n%2").arg(airport->city).arg(airport->name));
	
	QLocale locale(airport->countryCode.toLower());
    int utcDev = (int) (airport->lon/180*12 + 0.5); // lets estimate the deviation from UTC and round that
    QString lt = Whazzup::getInstance()->whazzupData().timestamp().addSecs(utcDev*3600).time().toString("HH:mm");
	lblCountry->setText(QString("%1 (%2)")
                        .arg(airport->countryCode)
                        .arg(NavData::getInstance()->countryName(airport->countryCode)));
    lblLocation->setText(QString("%1\n%2").arg(lat2str(airport->lat)).arg(lon2str(airport->lon)));
    lblTime->setText(QString("local time %1, UTC %2%3")
                        .arg(lt)
                        .arg(utcDev < 0 ? "": "+") // just a plus sign
                        .arg(utcDev));

	
    // arrivals
	arrivalsModel.setClients(airport->getArrivals());
    arrivalsSortModel->invalidate();
    treeArrivals->header()->resizeSections(QHeaderView::ResizeToContents);
	
    // departures
    departuresModel.setClients(airport->getDepartures());
    departuresSortModel->invalidate();
    treeDepartures->header()->resizeSections(QHeaderView::ResizeToContents);
	
    // set titles
	groupBoxArrivals->setTitle(QString("Arrivals (%1)").arg(airport->getArrivals().size()));
	groupBoxDepartures->setTitle(QString("Departures (%1)").arg(airport->getDepartures().size()));

    QList<Controller*> atcContent = airport->getAllControllers();
    groupBoxAtc->setTitle(QString("ATC (%1)").arg(atcContent.size()));

    // ATIS
    if(cbAtis->isChecked()) {
        Controller* atis = Whazzup::getInstance()->whazzupData().getController(airport->label + "_ATIS");
        if (atis != 0)
            atcContent.append(atis);
    }
	
    // observers
    if(cbObservers->isChecked()) {
        QList<Controller*> controllers = Whazzup::getInstance()->whazzupData().getControllers();
        for(int i = 0; i < controllers.size(); i++) {
            Controller* c = controllers[i];
            if(c->isObserver()) {
                double distance = NavData::distance(airport->lat, airport->lon, c->lat, c->lon);
                if(c->visualRange > distance && distance < 20)
                    atcContent.append(c);
            }
        }
    }
    /*
    // booked ATC
    QList<BookedController*> bookedcontrollers = airport->getBookedControllers();
    for (int i = 0; i < bookedcontrollers.size(); i++) {
        controllers.append(dynamic_cast <Controller*> (bookedcontrollers[i]));
    }
    */
	
	atcModel.setClients(atcContent);
    atcSortModel->invalidate();
    treeAtc->header()->resizeSections(QHeaderView::ResizeToContents);    

    cbPlotRoutes->setChecked(airport->showFlightLines);
}

void AirportDetails::atcSelected(const QModelIndex& index) {
	atcModel.modelSelected(atcSortModel->mapToSource(index));
}

void AirportDetails::arrivalSelected(const QModelIndex& index) {
	arrivalsModel.modelSelected(arrivalsSortModel->mapToSource(index));
}

void AirportDetails::departureSelected(const QModelIndex& index) {
	departuresModel.modelSelected(departuresSortModel->mapToSource(index));
}

void AirportDetails::on_cbPlotRoutes_toggled(bool checked)
{
    if(airport->showFlightLines != checked) {
        airport->setDisplayFlightLines(checked);
        Window::getInstance()->updateGLPilots();
    }
}

void AirportDetails::on_cbObservers_toggled(bool checked)
{
    refresh();
}

void AirportDetails::on_cbAtis_toggled(bool checked)
{
    refresh();
}
