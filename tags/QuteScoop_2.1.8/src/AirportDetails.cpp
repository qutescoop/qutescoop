/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "AirportDetails.h"

#include "helpers.h"
#include "NavData.h"
#include "Window.h"
#include "PilotDetails.h"
#include "Settings.h"

// singleton instance
AirportDetails *airportDetails = 0;
AirportDetails *AirportDetails::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(airportDetails == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::getInstance(true);
            airportDetails = new AirportDetails(parent);
        }
    return airportDetails;
}

// destroys a singleton instance
void AirportDetails::destroyInstance() {
    delete airportDetails;
    airportDetails = 0;
}

AirportDetails::AirportDetails(QWidget *parent):
    ClientDetails(parent),
    airport(0)
{
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);

    connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));

    // ATC list
    atcSortModel = new QSortFilterProxyModel;
    atcSortModel->setDynamicSortFilter(true);
    atcSortModel->setSourceModel(&atcModel);
    treeAtc->setModel(atcSortModel);
    treeAtc->sortByColumn(0, Qt::AscendingOrder);
    treeAtc->header()->setResizeMode(QHeaderView::Interactive);

    connect(treeAtc->header(), SIGNAL(sectionClicked(int)),
            treeAtc, SLOT(sortByColumn(int)));
    connect(treeAtc, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(atcSelected(const QModelIndex&)));

    // arrivals
    arrivalsSortModel = new QSortFilterProxyModel;
    arrivalsSortModel->setDynamicSortFilter(true);
    arrivalsSortModel->setSourceModel(&arrivalsModel);
    treeArrivals->setModel(arrivalsSortModel);
    treeArrivals->sortByColumn(9, Qt::AscendingOrder);
    treeArrivals->header()->setResizeMode(QHeaderView::Interactive);

    connect(treeArrivals->header(), SIGNAL(sectionClicked(int)),
            treeArrivals, SLOT(sortByColumn(int)));
    connect(treeArrivals, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(arrivalSelected(const QModelIndex&)));

    // departures
    departuresSortModel = new QSortFilterProxyModel;
    departuresSortModel->setDynamicSortFilter(true);
    departuresSortModel->setSourceModel(&departuresModel);
    treeDepartures->setModel(departuresSortModel);
    treeDepartures->sortByColumn(8, Qt::AscendingOrder);
    treeDepartures->header()->setResizeMode(QHeaderView::Interactive);

    connect(treeDepartures->header(), SIGNAL(sectionClicked(int)),
            treeDepartures, SLOT(sortByColumn(int)));
    connect(treeDepartures, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(departureSelected(const QModelIndex&)));

    metarModel = new MetarModel(qobject_cast<Window *>(this->parent()));

    refresh();
}

void AirportDetails::refresh(Airport* newAirport) {
    if (!Settings::airportDetailsSize().isNull()) resize(Settings::airportDetailsSize());
    if (!Settings::airportDetailsPos().isNull()) move(Settings::airportDetailsPos());
    if (!Settings::airportDetailsGeometry().isNull()) restoreGeometry(Settings::airportDetailsGeometry());

    if(newAirport != 0) {
        if(newAirport != airport) {
            // scroll Boxes to top on new Data
            treeAtc->scrollTo(atcSortModel->index(0, 0));
            treeArrivals->scrollTo(arrivalsSortModel->index(0, 0));
            treeDepartures->scrollTo(departuresSortModel->index(0, 0));
        }
        airport = newAirport;
    }
    if(airport == 0) return;
    setMapObject(airport);

    setWindowTitle(airport->toolTip());

    lblName->setText(QString("%1\n%2").arg(airport->city).arg(airport->name));
    lblCountry->setText(QString("%1 (%2)")
                        .arg(airport->countryCode)
                        .arg(NavData::getInstance()->countryCodes[airport->countryCode]));
    lblLocation->setText(QString("%1%2 %3%4").
                         arg(airport->lat > 0? "N": "S").
                         arg(qAbs(airport->lat), 6, 'f', 3, '0').
                         arg(airport->lon > 0? "E": "W").
                         arg(qAbs(airport->lon), 7, 'f', 3, '0'));

    int utcDev = (int) (airport->lon / 180. * 12. + .5); // lets estimate the deviation from UTC and round that
    QString lt = Whazzup::getInstance()->whazzupData().whazzupTime.
                 addSecs(utcDev * 3600).time().toString("HH:mm");
    lblTime->setText(QString("%1 loc, UTC %2%3")
                        .arg(lt)
                        .arg(utcDev < 0 ? "": "+") // just a plus sign
                        .arg(utcDev));

    // arrivals
    arrivalsModel.setClients(airport->arrivals.toList());
    arrivalsSortModel->invalidate();
    treeArrivals->header()->resizeSections(QHeaderView::ResizeToContents);

    // departures
    departuresModel.setClients(airport->departures.toList());
    departuresSortModel->invalidate();
    treeDepartures->header()->resizeSections(QHeaderView::ResizeToContents);

    // set titles
    if (Settings::filterTraffic()) {
        groupBoxArrivals->setTitle(QString("Arrivals (%1 filtered, %2 total)").
                                   arg(airport->numFilteredArrivals).arg(airport->arrivals.size()));
        groupBoxDepartures->setTitle(QString("Departures (%1 filtered, %2 total)").
                                     arg(airport->numFilteredDepartures).arg(airport->departures.size()));
    } else {
        groupBoxArrivals->setTitle(QString("Arrivals (%1)").arg(airport->arrivals.size()));
        groupBoxDepartures->setTitle(QString("Departures (%1)").arg(airport->departures.size()));
    }

    QSet<Controller*> atcContent = airport->getAllControllers() + checkSectors();
    groupBoxAtc->setTitle(QString("ATC (%1)").arg(atcContent.size()));

    // ATIS
    if(cbAtis->isChecked()) {
        Controller* atis = Whazzup::getInstance()->whazzupData().controllers[airport->label + "_ATIS"];
        if (atis != 0)
            atcContent.insert(atis);
    }

    // observers
    if(cbObservers->isChecked()) {
        foreach(Controller *c, Whazzup::getInstance()->whazzupData().controllers) {
            if(c->isObserver())
                if(NavData::distance(airport->lat, airport->lon, c->lat, c->lon) < qMax(50, c->visualRange))
                    atcContent.insert(c);
        }
    }
    /*
    // booked ATC
    QList<BookedController*> bookedcontrollers = airport->getBookedControllers();
    for (int i = 0; i < bookedcontrollers.size(); i++)
        controllers.append(dynamic_cast <Controller*> (bookedcontrollers[i]));
    */

    atcModel.setClients(atcContent.values());
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

void AirportDetails::on_cbPlotRoutes_toggled(bool checked) {
    if(airport->showFlightLines != checked) {
        airport->showFlightLines = checked;
        if (Window::getInstance(false) != 0) {
            Window::getInstance(true)->mapScreen->glWidget->createPilotsList();
            Window::getInstance(true)->mapScreen->glWidget->updateGL();;
        }
        if (PilotDetails::getInstance(false) != 0)
            PilotDetails::getInstance(true)->refresh();
    }
}

void AirportDetails::on_cbObservers_toggled(bool checked) {
    refresh();
}

void AirportDetails::on_cbAtis_toggled(bool checked) {
    refresh();
}

void AirportDetails::on_pbMetar_clicked() {
    disconnect(metarModel, SIGNAL(gotMetar(QString)), this, SLOT(on_pbMetar_clicked()));
    QList<Airport*> airports;
    if (airport != 0) {
        airports += airport;
        metarModel->setData(airports);
        if(metarModel->rowCount() == 1) // means that the METAR is readily downloaded
            metarModel->modelClicked(metarModel->index(0));
        else
            connect(metarModel, SIGNAL(gotMetar(QString)), this, SLOT(on_pbMetar_clicked()));
    }
}

QSet<Controller*> AirportDetails::checkSectors() const {
    QSet<Controller*> result;

    foreach(Controller *c, Whazzup::getInstance()->whazzupData().activeSectors()) {
        int crossings = 0;
        double x1, x2;

        int size = c->sector->points.size();
        for(int ii = 0; ii < size ; ii++ ) {
            /* This is done to ensure that we get the same result when
               the line goes from left to right and right to left */
            if(c->sector->points.value(ii).first <
                 c->sector->points.value((ii + 1) % size).first) {
                x1 = c->sector->points.value(ii).first;
                x2 = c->sector->points.value((ii + 1) % size).first;
            } else {
                x1 = c->sector->points.value((ii + 1) % size).first;
                x2 = c->sector->points.value(ii).first;
            }

            /* First check if the ray is possible to cross the line */
            if(airport->lat > x1 && airport->lat <= x2 && (
                    airport->lon < c->sector->points.value(ii).second
                    || airport->lon <= c->sector->points.value((ii + 1) % 8).second)) {
                /* Calculate the equation of the line */
                double dx = c->sector->points.value((ii + 1) % size).first
                            - c->sector->points.value(ii).first;
                double dy = c->sector->points.value((ii + 1) % size).second
                            - c->sector->points.value(ii).second;
                double k;

                if(qFuzzyIsNull(dx))
                    k = INFINITY;	// math.h
                else
                    k = dy/dx;

                double m = c->sector->points.value(ii).second
                           - k * c->sector->points.value(ii).first;

                /* Find if the ray crosses the line */
                double y2 = k * airport->lat + m;
                if(airport->lon <= y2)
                    crossings++;
            }
        }
        if(crossings % 2 == 1)
            result.insert(c);
    }
    return result;
}

void AirportDetails::closeEvent(QCloseEvent *event){
   Settings::setAirportDetailsPos(pos());
   Settings::setAirportDetailsSize(size());
   Settings::setAirportDetailsGeometry(saveGeometry());
   event->accept();
}
