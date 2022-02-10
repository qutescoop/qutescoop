/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "AirportDetails.h"

#include "helpers.h"
#include "NavData.h"
#include "Window.h"
#include "PilotDetails.h"
#include "Settings.h"

AirportDetails *airportDetails = 0;
AirportDetails *AirportDetails::instance(bool createIfNoInstance, QWidget *parent) {
    if(airportDetails == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::instance();
            airportDetails = new AirportDetails(parent);
        }
    return airportDetails;
}

void AirportDetails::destroyInstance() {
    delete airportDetails;
    airportDetails = 0;
}

AirportDetails::AirportDetails(QWidget *parent):
        ClientDetails(parent),
        _airport(0) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
    //    setWindowFlags(Qt::Tool);

    connect(btnShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));

    // ATC list
    _atcSortModel = new QSortFilterProxyModel;
    _atcSortModel->setDynamicSortFilter(true);
    _atcSortModel->setSourceModel(&_atcModel);
    treeAtc->setModel(_atcSortModel);
    treeAtc->sortByColumn(0, Qt::AscendingOrder);
    treeAtc->header()->setSectionResizeMode(QHeaderView::Interactive);

    connect(treeAtc->header(), SIGNAL(sectionClicked(int)),
            treeAtc, SLOT(sortByColumn(int)));
    connect(treeAtc, SIGNAL(clicked(QModelIndex)),
            this, SLOT(atcSelected(QModelIndex)));

    // arrivals
    _arrivalsSortModel = new QSortFilterProxyModel;
    _arrivalsSortModel->setDynamicSortFilter(true);
    _arrivalsSortModel->setSourceModel(&_arrivalsModel);
    treeArrivals->setModel(_arrivalsSortModel);
    treeArrivals->sortByColumn(9, Qt::AscendingOrder);
    treeArrivals->header()->setSectionResizeMode(QHeaderView::Interactive);

    connect(treeArrivals->header(), SIGNAL(sectionClicked(int)),
            treeArrivals, SLOT(sortByColumn(int)));
    connect(treeArrivals, SIGNAL(clicked(QModelIndex)),
            this, SLOT(arrivalSelected(QModelIndex)));

    // departures
    _departuresSortModel = new QSortFilterProxyModel;
    _departuresSortModel->setDynamicSortFilter(true);
    _departuresSortModel->setSourceModel(&_departuresModel);
    treeDepartures->setModel(_departuresSortModel);
    treeDepartures->sortByColumn(8, Qt::AscendingOrder);
    treeDepartures->header()->setSectionResizeMode(QHeaderView::Interactive);

    connect(treeDepartures->header(), SIGNAL(sectionClicked(int)),
            treeDepartures, SLOT(sortByColumn(int)));
    connect(treeDepartures, SIGNAL(clicked(QModelIndex)),
            this, SLOT(departureSelected(QModelIndex)));

    // METAR
    _metarModel = new MetarModel(qobject_cast<Window *>(this->parent()));

    // Geometry
    if (!Settings::airportDetailsSize().isNull()) resize(Settings::airportDetailsSize());
    if (!Settings::airportDetailsPos().isNull()) move(Settings::airportDetailsPos());
    if (!Settings::airportDetailsGeometry().isNull()) restoreGeometry(Settings::airportDetailsGeometry());

    refresh();
}

void AirportDetails::refresh(Airport* newAirport) {
    if(newAirport != 0) {
        if(newAirport != _airport) {
            // scroll Boxes to top on new Data
            treeAtc->scrollTo(_atcSortModel->index(0, 0));
            treeArrivals->scrollTo(_arrivalsSortModel->index(0, 0));
            treeDepartures->scrollTo(_departuresSortModel->index(0, 0));
        }
        _airport = newAirport;
    }
    if(_airport == 0) return;
    setMapObject(_airport);

    setWindowTitle(_airport->toolTip());

    // info panel
    lblName->setText(QString("%1\n%2").arg(_airport->city, _airport->name));
    lblCountry->setText(QString("%1 (%2)")
                        .arg(_airport->countryCode, NavData::instance()->countryCodes[_airport->countryCode]));
    lblLocation->setText(QString("%1%2 %3%4").
                         arg(_airport->lat > 0? "N": "S").
                         arg(qAbs(_airport->lat), 6, 'f', 3, '0').
                         arg(_airport->lon > 0? "E": "W").
                         arg(qAbs(_airport->lon), 7, 'f', 3, '0'));

    int utcDev = (int) (_airport->lon / 180. * 12. + .5); // lets estimate the deviation from UTC and round that
    QString lt = Whazzup::instance()->whazzupData().whazzupTime.
                 addSecs(utcDev * 3600).time().toString("HH:mm");
    lblTime->setText(QString("%1 loc, UTC %2%3")
                        .arg(lt, utcDev < 0 ? "": "+") // just a plus sign
                        .arg(utcDev));
    // fetch METAR
    connect(_metarModel, &MetarModel::gotMetar, this, &AirportDetails::onGotMetar);
    on_pbMetar_clicked();

    // arrivals
    _arrivalsModel.setClients(_airport->arrivals.values());
    _arrivalsSortModel->invalidate();
    treeArrivals->header()->resizeSections(QHeaderView::ResizeToContents);

    // departures
    _departuresModel.setClients(_airport->departures.values());
    _departuresSortModel->invalidate();
    treeDepartures->header()->resizeSections(QHeaderView::ResizeToContents);

    // set titles
    if (Settings::filterTraffic()) {
        groupBoxArrivals->setTitle(QString("Arrivals (%1 filtered, %2 total)").
                                   arg(_airport->numFilteredArrivals).arg(_airport->arrivals.size()));
        groupBoxDepartures->setTitle(QString("Departures (%1 filtered, %2 total)").
                                     arg(_airport->numFilteredDepartures).arg(_airport->departures.size()));
    } else {
        groupBoxArrivals->setTitle(QString("Arrivals (%1)").arg(_airport->arrivals.size()));
        groupBoxDepartures->setTitle(QString("Departures (%1)").arg(_airport->departures.size()));
    }

    QSet<Controller*> atcContent = _airport->allControllers() + checkSectors();
    groupBoxAtc->setTitle(QString("ATC (%1)").arg(atcContent.size()));

    // ATIS
    if(cbAtis->isChecked()) {
        Controller* atis = Whazzup::instance()->whazzupData().controllers[_airport->label + "_ATIS"];
        if (atis != 0)
            atcContent.insert(atis);
    }

    // non-ATC
    if(cbObservers->isChecked()) {
        foreach(Controller *c, Whazzup::instance()->whazzupData().controllers) {
            //if(c->isObserver()) // don't need this anymore - we want them all
                if(NavData::distance(_airport->lat, _airport->lon, c->lat, c->lon) < qMax(50, c->visualRange))
                    atcContent.insert(c);
        }
    }

    _atcModel.setClients(atcContent.values());
    _atcSortModel->invalidate();
    treeAtc->header()->resizeSections(QHeaderView::ResizeToContents);

    cbPlotRoutes->setChecked(_airport->showFlightLines);
}

void AirportDetails::atcSelected(const QModelIndex& index) {
    _atcModel.modelSelected(_atcSortModel->mapToSource(index));
}

void AirportDetails::arrivalSelected(const QModelIndex& index) {
    _arrivalsModel.modelSelected(_arrivalsSortModel->mapToSource(index));
}

void AirportDetails::departureSelected(const QModelIndex& index) {
    _departuresModel.modelSelected(_departuresSortModel->mapToSource(index));
}

void AirportDetails::on_cbPlotRoutes_toggled(bool checked) {
    if(_airport->showFlightLines != checked) {
        _airport->showFlightLines = checked;
        if (Window::instance(false) != 0) {
            Window::instance()->mapScreen->glWidget->createPilotsList();
            Window::instance()->mapScreen->glWidget->updateGL();;
        }
        if (PilotDetails::instance(false) != 0)
            PilotDetails::instance()->refresh();
    }
}

void AirportDetails::on_cbObservers_toggled(bool) {
    refresh();
}

void AirportDetails::on_cbAtis_toggled(bool) {
    refresh();
}

void AirportDetails::on_pbMetar_clicked() {
    qDebug() << "AirportDetails::on_pbMetar_clicked";
    lblMetar->setText("…");
    QList<Airport*> airports;
    if (_airport != 0) {
        airports += _airport;
        _metarModel->setAirports(airports);
    }
}

void AirportDetails::onGotMetar(const QString &airportLabel, const QString &encoded, const QString &humanHtml)
{
    qDebug() << "AirportDetails::onGotMetar" << airportLabel << encoded;
    if (_airport->label == airportLabel) {
        lblMetar->setText(encoded);
        lblMetar->setToolTip(humanHtml);
    }
}

QSet<Controller*> AirportDetails::checkSectors() const {
    QSet<Controller*> result;

    foreach(Controller *c, Whazzup::instance()->whazzupData().controllersWithSectors()) {
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

            /* First check if the ray is able to cross the line */
            if(_airport->lat > x1 && _airport->lat <= x2 && (
                    _airport->lon < c->sector->points.value(ii).second
                    || _airport->lon <= c->sector->points.value((ii + 1) % 8).second)) {
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

                /* Check if the ray crosses the line */
                double y2 = k * _airport->lat + m;
                if(_airport->lon <= y2)
                    crossings++;
            }
        }
        if(crossings % 2 == 1)
            result.insert(c);
    }
    return result;
}

void AirportDetails::closeEvent(QCloseEvent *event) {
   Settings::setAirportDetailsPos(pos());
   Settings::setAirportDetailsSize(size());
   Settings::setAirportDetailsGeometry(saveGeometry());
   event->accept();
}
