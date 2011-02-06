/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "AirportDetails.h"
#include <QLocale>
#include <QHeaderView>
#include <QPolygon>
#include <math.h>

#include "helpers.h"
#include "NavData.h"
#include "Window.h"

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

// function only used here
QString lat2str(double lat) {
    QString result = "N";
    if (lat < 0) {
        result += "S";
        lat *= -1;
    }

    int lat1 = (int)lat;
    double lat2 = (lat - (int)lat) * 60.0;
    result += QString("%1 %2'")
    .arg(lat1, 2, 10, QChar('0'))
    .arg(lat2, 2, 'f', 3, QChar('0'));

    return result;
}

// function only used here
QString lon2str(double lon) {
    QString result = "E";
    if (lon < 0) {
        lon *= -1;
        result = "W";
    }

    int lon1 = (int)lon;
    double lon2 = (lon - (int)lon) * 60.0;
    result += QString("%1 %2'")
    .arg(lon1, 3, 10, QChar('0'))
    .arg(lon2, 2, 'f', 3, QChar('0'));

    return result;
}

AirportDetails::AirportDetails(QWidget *parent):
    ClientDetails(parent),
    airport(0)
{
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);

    connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
    connect(this, SIGNAL(showOnMap(double, double)), qobject_cast<Window *>(this->parent()), SLOT(showOnMap(double, double)));

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
    treeArrivals->sortByColumn(9, Qt::AscendingOrder);
    treeArrivals->header()->setResizeMode(QHeaderView::Interactive);

    connect(treeArrivals->header(), SIGNAL(sectionClicked(int)), treeArrivals, SLOT(sortByColumn(int)));
    connect(treeArrivals, SIGNAL(clicked(const QModelIndex&)), this, SLOT(arrivalSelected(const QModelIndex&)));

    // departures
    departuresSortModel = new QSortFilterProxyModel;
    departuresSortModel->setDynamicSortFilter(true);
    departuresSortModel->setSourceModel(&departuresModel);
    treeDepartures->setModel(departuresSortModel);
    treeDepartures->sortByColumn(8, Qt::AscendingOrder);
    treeDepartures->header()->setResizeMode(QHeaderView::Interactive);

    connect(treeDepartures->header(), SIGNAL(sectionClicked(int)), treeDepartures, SLOT(sortByColumn(int)));
    connect(treeDepartures, SIGNAL(clicked(const QModelIndex&)), this, SLOT(departureSelected(const QModelIndex&)));

    metarModel = new MetarModel(qobject_cast<Window *>(this->parent()));

    refresh();
}

void AirportDetails::refresh(Airport* newAirport) {
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
    groupBoxArrivals->setTitle(QString("Arrivals (%1 filtered, %2 total)").arg(airport->numFilteredArrivals).arg(airport->getArrivals().size()));
    groupBoxDepartures->setTitle(QString("Departures (%1 filtered, %2 total)").arg(airport->numFilteredDepartures).arg(airport->getDepartures().size()));


    QList<Controller*> atcContent = airport->getAllControllers();
    atcContent += checkSectors();
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
        qobject_cast<Window *>(this->parent())->updateGLPilots();
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

void AirportDetails::on_pbMetar_clicked()
{
    disconnect(metarModel, SIGNAL(gotMetar(QString)), this, SLOT(on_pbMetar_clicked()));
    QList<Airport*> airports;
    if (airport != 0) {
        airports += airport;
        metarModel->setData(airports);
        if(metarModel->rowCount() == 1) { // means that the METAR is readily downloaded
            metarModel->modelClicked(metarModel->index(0));
            //this->lower(); // does not what expected: lowers all of QuteScoop, not just this dialog
        } else {
            connect(metarModel, SIGNAL(gotMetar(QString)), this, SLOT(on_pbMetar_clicked()));
        }
    }
}

QList<Controller*> AirportDetails::checkSectors()
{
    QList<Controller*> result;
    QList<Controller*> allSectors = Whazzup::getInstance()->whazzupData().activeSectors();

    for(int i = 0; i < allSectors.size(); i++)
    {
        int crossings = 0;
        double x1, x2;

        //To make to code more clear
        int size = allSectors[i]->sector->sector().size();

        for ( int ii = 0; ii < size ; ii++ )
        {
            /* This is done to ensure that we get the same result when
               the line goes from left to right and right to left */
            if ( allSectors[i]->sector->sector().value(ii).first <
                 allSectors[i]->sector->sector().value((ii+1)%size).first)
            {
                    x1 = allSectors[i]->sector->sector().value(ii).first;
                    x2 = allSectors[i]->sector->sector().value((ii+1)%size).first;
            }
            else
            {
                    x1 = allSectors[i]->sector->sector().value((ii+1)%size).first;
                    x2 = allSectors[i]->sector->sector().value(ii).first;
            }

            /* First check if the ray is possible to cross the line */
            if ( airport->lat > x1 && airport->lat <= x2 && (
                    airport->lon < allSectors[i]->sector->sector().value(ii).second
                    || airport->lon <= allSectors[i]->sector->sector().value((ii+1)%8).second))
            {
                    static const float eps = 0.000001;

                    /* Calculate the equation of the line */
                    double dx = allSectors[i]->sector->sector().value((ii+1)%size).first
                                - allSectors[i]->sector->sector().value(ii).first;
                    double dy = allSectors[i]->sector->sector().value((ii+1)%size).second
                                - allSectors[i]->sector->sector().value(ii).second;
                    double k;

                    if ( fabs(dx) < eps )
                    {
                            k = INFINITY;	// math.h
                    }
                    else
                    {
                            k = dy/dx;
                    }

                    double m = allSectors[i]->sector->sector().value(ii).second
                               - k * allSectors[i]->sector->sector().value(ii).first;

                    /* Find if the ray crosses the line */
                    double y2 = k * airport->lat + m;
                    if ( airport->lon <= y2 )
                    {
                            crossings++;
                    }
                }
        }

        if(crossings%2 == 1) result.append(allSectors.value(i));
    }

    return result;
}
