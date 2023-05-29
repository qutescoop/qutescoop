/**************************************************************************
 * This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "AirportDetails.h"

#include "NavData.h"
#include "Window.h"
#include "PilotDetails.h"
#include "Settings.h"
#include "Whazzup.h"

//singleton instance
AirportDetails *airportDetails = 0;
AirportDetails *AirportDetails::instance(bool createIfNoInstance, QWidget *parent) {
    if(airportDetails == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::instance();
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
        _airport(0) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);

    connect(btnShowOnMap, &QAbstractButton::clicked, this, &ClientDetails::showOnMap);
    connect(pbMetar, &QAbstractButton::clicked, this, &AirportDetails::refreshMetar);
    connect(cbPlotRoutes, &QCheckBox::clicked, this, &AirportDetails::togglePlotRoutes);
    connect(cbOtherAtc, &QCheckBox::clicked, this, &AirportDetails::toggleShowOtherAtc);

    // ATC list
    _atcSortModel = new QSortFilterProxyModel;
    _atcSortModel->setDynamicSortFilter(true);
    _atcSortModel->setSourceModel(&_atcModel);
    treeAtc->setModel(_atcSortModel);
    treeAtc->sortByColumn(0, Qt::AscendingOrder);
    treeAtc->header()->setSectionResizeMode(QHeaderView::Interactive);

    connect(treeAtc, &QAbstractItemView::clicked, this, &AirportDetails::atcSelected);

    // arrivals
    _arrivalsSortModel = new QSortFilterProxyModel;
    _arrivalsSortModel->setDynamicSortFilter(true);
    _arrivalsSortModel->setSourceModel(&_arrivalsModel);
    _arrivalsSortModel->setSortRole(Qt::UserRole);
    treeArrivals->setModel(_arrivalsSortModel);
    treeArrivals->sortByColumn(8, Qt::AscendingOrder);
    treeArrivals->header()->setSectionResizeMode(QHeaderView::Interactive);

    connect(treeArrivals, &QAbstractItemView::clicked, this, &AirportDetails::arrivalSelected);

    // departures
    _departuresSortModel = new QSortFilterProxyModel;
    _departuresSortModel->setDynamicSortFilter(true);
    _departuresSortModel->setSourceModel(&_departuresModel);
    _departuresSortModel->setSortRole(Qt::UserRole);
    treeDepartures->setModel(_departuresSortModel);
    treeDepartures->sortByColumn(6, Qt::AscendingOrder);
    treeDepartures->header()->setSectionResizeMode(QHeaderView::Interactive);

    connect(treeDepartures, &QAbstractItemView::clicked, this, &AirportDetails::departureSelected);

    // METAR
    _metarModel = new MetarModel(qobject_cast<Window *>(this->parent()));

    // Geometry
    auto preferences = Settings::dialogPreferences(m_preferencesName);
    if (!preferences.size.isNull()) { resize(preferences.size); }
    if (!preferences.pos.isNull()) { move(preferences.pos); }
    if (!preferences.geometry.isNull()) { restoreGeometry(preferences.geometry); }

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
    lblCity->setText(_airport->city);
    lblCity->setHidden(_airport->city.isEmpty());
    lblCityLabel->setHidden(_airport->city.isEmpty());

    lblName->setText(_airport->name);
    lblName->setHidden(_airport->name.isEmpty());
    lblNameLabel->setHidden(_airport->name.isEmpty());

    lblCountry->setText(QString("%1 (%2)")
                        .arg(_airport->countryCode, NavData::instance()->countryCodes[_airport->countryCode]));
    lblCharts->setText(QString("[chartfox.org/%1](https://chartfox.org/%1)").arg(_airport->label));

    // fetch METAR
    connect(_metarModel, &MetarModel::gotMetar, this, &AirportDetails::onGotMetar);
    refreshMetar();

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

    // non-ATC
    if(cbOtherAtc->isChecked()) {
        foreach(Controller *c, Whazzup::instance()->whazzupData().controllers) {
            // add those within visual range or max. 50 NM away
            if(NavData::distance(_airport->lat, _airport->lon, c->lat, c->lon) < qMax(50, c->visualRange))
                atcContent.insert(c);
        }
    }

    _atcModel.setClients(atcContent.values());
    _atcSortModel->invalidate();
    groupBoxAtc->setTitle(QString("ATC (%1)").arg(atcContent.size()));
    treeAtc->header()->resizeSections(QHeaderView::ResizeToContents);

    cbPlotRoutes->setChecked(_airport->showRoutes);
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

void AirportDetails::togglePlotRoutes(bool checked) {
    if(_airport->showRoutes != checked) {
        _airport->showRoutes = checked;
        if (Window::instance(false) != 0) {
            Window::instance()->mapScreen->glWidget->invalidatePilots();
        }
        if (PilotDetails::instance(false) != 0) {
            PilotDetails::instance()->refresh();
        }
    }
}

void AirportDetails::refreshMetar() {
    qDebug() << "AirportDetails::refreshMetar";
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
        if(c->sector->containsPoint(QPointF(_airport->lat, _airport->lon)))
            result.insert(c);
    }
    return result;
}

void AirportDetails::closeEvent(QCloseEvent *event) {
    Settings::setDialogPreferences(
        m_preferencesName,
        Settings::DialogPreferences {
            .size = size(),
            .pos = pos(),
            .geometry = saveGeometry()
        }
    );
    event->accept();
}

void AirportDetails::toggleShowOtherAtc(bool)
{
    refresh();
}
