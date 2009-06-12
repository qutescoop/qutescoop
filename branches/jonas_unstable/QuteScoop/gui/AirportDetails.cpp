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
	treeAtc->setModel(&atcModel);
	treeAtc->header()->setResizeMode(QHeaderView::ResizeToContents);
	connect(treeAtc->header(), SIGNAL(sectionClicked(int)), treeAtc, SLOT(sortByColumn(int)));
	connect(treeAtc, SIGNAL(doubleClicked(const QModelIndex&)), &atcModel, SLOT(modelSelected(const QModelIndex&)));

	// arrivals
	arrivalsSortModel = new QSortFilterProxyModel;
	arrivalsSortModel->setDynamicSortFilter(true);
	arrivalsSortModel->setSourceModel(&arrivalsModel);
	treeArrivals->setModel(arrivalsSortModel);
	
	treeArrivals->header()->setResizeMode(QHeaderView::ResizeToContents);
	connect(treeArrivals->header(), SIGNAL(sectionClicked(int)), treeArrivals, SLOT(sortByColumn(int)));
	connect(treeArrivals, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(arrivalSelected(const QModelIndex&)));

	// departures
	departuresSortModel = new QSortFilterProxyModel;
	departuresSortModel->setDynamicSortFilter(true);
	departuresSortModel->setSourceModel(&departuresModel);
	treeDepartures->setModel(departuresSortModel);

	treeDepartures->header()->setResizeMode(QHeaderView::ResizeToContents);
	connect(treeDepartures->header(), SIGNAL(sectionClicked(int)), treeDepartures, SLOT(sortByColumn(int)));
	connect(treeDepartures, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(departureSelected(const QModelIndex&)));
	
	treeArrivals->sortByColumn(8, Qt::AscendingOrder);
	treeDepartures->sortByColumn(7, Qt::AscendingOrder);
	
	refresh();
}

void AirportDetails::refresh(Airport* newAirport) {
	if(newAirport != 0) airport = newAirport;
	if(airport == 0) return;
	setMapObject(airport);
	
	setWindowTitle(airport->toolTip());

	lblIcao->setText(airport->label);
	lblCity->setText(airport->city);
	lblName->setText(airport->name);
	
	QLocale locale(airport->countryCode.toLower());
	lblCountry->setText(QString("%1 (%2)").arg(airport->countryCode).arg(NavData::getInstance()->countryName(airport->countryCode)));
	lblLocation->setText(QString("%1 %2").arg(lat2str(airport->lat)).arg(lon2str(airport->lon)));
	
	arrivalsModel.setClients(airport->getArrivals());
	treeArrivals->sortByColumn(8, Qt::AscendingOrder);
	departuresModel.setClients(airport->getDepartures());
	treeDepartures->sortByColumn(7, Qt::AscendingOrder);
	
	groupBoxArrivals->setTitle(QString("Arrivals (%1)").arg(airport->getArrivals().size()));
	groupBoxDepartures->setTitle(QString("Departures (%1)").arg(airport->getDepartures().size()));

	QList<Controller*> atcContent = airport->getAllControllers();
	groupBoxAtc->setTitle(QString("ATC (%1)").arg(atcContent.size()));
	
	// observers
	QList<Controller*> controllers = Whazzup::getInstance()->whazzupData().getControllers();
	for(int i = 0; i < controllers.size(); i++) {
		Controller* c = controllers[i];
		if(c->isObserver()) {
			double distance = NavData::distance(airport->lat, airport->lon, c->lat, c->lon);
			if(c->visualRange > distance && distance < 20)
				atcContent.append(c);			
		}
	}
	
	atcModel.setClients(atcContent);
}

void AirportDetails::arrivalSelected(const QModelIndex& index) {
	arrivalsModel.modelSelected(arrivalsSortModel->mapToSource(index));
}

void AirportDetails::departureSelected(const QModelIndex& index) {
	departuresModel.modelSelected(departuresSortModel->mapToSource(index));
}

