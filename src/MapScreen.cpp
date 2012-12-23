#include "MapScreen.h"
#include "Launcher.h"

static MapScreen *mapScreenInstance = 0;
MapScreen* MapScreen::instance(bool createIfNotExists = true) {
    if(mapScreenInstance == 0 && createIfNotExists)
        mapScreenInstance = new MapScreen;
    return mapScreenInstance;
}

MapScreen::MapScreen(QWidget *parent) :
    QWidget(parent)
{
    setMouseTracking(true);
    _xDistance = 0;

    //OpenGL config
    QSettings* settings = new QSettings();

    QGLFormat fmt;
    fmt.setDirectRendering(settings->value("gl/directrendering", fmt.defaultFormat().directRendering()).toBool());
    fmt.setDoubleBuffer(settings->value("gl/doublebuffer", fmt.defaultFormat().doubleBuffer()).toBool());
    fmt.setStencil(settings->value("gl/stencilbuffer", fmt.defaultFormat().stencil()).toBool());
    if (fmt.defaultFormat().stencilBufferSize() > 0)
        fmt.setStencilBufferSize(settings->value("gl/stencilsize", fmt.defaultFormat().stencilBufferSize()).toInt());
    fmt.setDepth(settings->value("gl/depthbuffer", fmt.defaultFormat().depth()).toBool());
    if (fmt.defaultFormat().depthBufferSize() > 0)
        fmt.setDepthBufferSize(settings->value("gl/depthsize", fmt.defaultFormat().depthBufferSize()).toInt());
    fmt.setAlpha(settings->value("gl/alphabuffer", fmt.defaultFormat().alpha()).toBool());
    if (fmt.defaultFormat().alphaBufferSize() > 0)
        fmt.setAlphaBufferSize(settings->value("gl/alphasize", fmt.defaultFormat().alphaBufferSize()).toInt());
    fmt.setSampleBuffers(settings->value("gl/samplebuffers", fmt.defaultFormat().sampleBuffers()).toBool());
    if (fmt.defaultFormat().samples() > 0)
        fmt.setSamples(settings->value("gl/samples", fmt.defaultFormat().samples()).toInt());
    fmt.setAccum(settings->value("gl/accumbuffer", fmt.defaultFormat().accum()).toBool());
    if (fmt.defaultFormat().accumBufferSize() > 0)
        fmt.setAccumBufferSize(settings->value("gl/accumsize", fmt.defaultFormat().accumBufferSize()).toInt());
    fmt.setRgba(true);


    qDebug() << "MapScreen::MapScreen() creating GLWidget";
    glWidget = new GLWidget(fmt, this);
    qDebug() << "MapScreen::MapScreen() creating GLWidget --finished";


    _oslPilots = new OnScreenLabel(this);
    _oslPilots->typ = 1;
    connect(_oslPilots, SIGNAL(entered(int)), this, SLOT(LabelEntered(int)));
    connect(_oslPilots, SIGNAL(left(int)), this,  SLOT(LabelLeft(int)));        // causes strange flickering on linux
    _oslPilots->setText(tr("Pilots"));
    _oslPilots->setFont(QFont("Arial", 16, QFont::Bold));
    _oslPilots->setAutoFillBackground(true);
    _oslPilots->setMouseTracking(true);
    _oslPilots->raise();
    _xDistance += 59;

    _oslController = new OnScreenLabel(this);
    _oslController->typ = 2;
    connect(_oslController, SIGNAL(entered(int)), this, SLOT(LabelEntered(int)));
    //connect(L_Controller, SIGNAL(left(int)), this, SLOT(LabelLeft(int)));     // see L_Pilots
    _oslController->setText(tr("Controller"));
    _oslController->setFont(QFont("Arial", 16, QFont::Bold));
    _oslController->setAutoFillBackground(true);
    _oslController->setMouseTracking(true);
    _oslController->move(_xDistance, 0);
    _oslController->raise();
    _xDistance += 103;

    _oslNavData = new OnScreenLabel(this);
    _oslNavData->typ = 3;
    connect(_oslNavData, SIGNAL(entered(int)), this, SLOT(LabelEntered(int)));
    //connect(L_NavData, SIGNAL(left(int)), this, SLOT(LabelLeft(int)));        // see L_Pilots
    _oslNavData->setText(tr("NavData"));
    _oslNavData->setFont(QFont("Arial", 16, QFont::Bold));
    _oslNavData->setAutoFillBackground(true);
    _oslNavData->setMouseTracking(true);
    _oslNavData->move(_xDistance,0);
    _oslNavData->raise();
    _xDistance += 87;

    _oslWeather = new OnScreenLabel(this);
    _oslWeather->typ = 4;
    connect(_oslWeather, SIGNAL(entered(int)), this, SLOT(LabelEntered(int)));
    //connect(L_Weather, SIGNAL(left(int)), this, SLOT(LabelLeft(int)));       // see L_Pilots
    _oslWeather->setText(tr("Weather"));
    _oslWeather->setFont(QFont("Arial", 16, QFont::Bold));
    _oslWeather->setAutoFillBackground(true);
    _oslWeather->setMouseTracking(true);
    _oslWeather->move(_xDistance,0);
    _oslWeather->raise();

    _oswPilots = new OnScreenWidget(this);
    _oswPilots->typ = 1;
    connect(_oswPilots, SIGNAL(entered(int)), this, SLOT(WidgetEntered(int)));
    connect(_oswPilots, SIGNAL(left(int)), this, SLOT(WidgetLeft(int)));

    _oswController = new OnScreenWidget(this);
    _oswController->typ = 2;
    connect(_oswController, SIGNAL(entered(int)), this, SLOT(WidgetEntered(int)));
    connect(_oswController, SIGNAL(left(int)), this, SLOT(WidgetLeft(int)));

    _oswNavData = new OnScreenWidget(this);
    _oswNavData->typ = 3;
    connect(_oswNavData, SIGNAL(entered(int)), this, SLOT(WidgetEntered(int)));
    connect(_oswNavData, SIGNAL(left(int)), this, SLOT(WidgetLeft(int)));

    _oswWeather = new OnScreenWidget(this);
    _oswWeather->typ = 4;
    connect(_oswWeather, SIGNAL(entered(int)), this, SLOT(WidgetEntered(int)));
    connect(_oswWeather, SIGNAL(left(int)), this, SLOT(WidgetLeft(int)));

    createPilotWidget();
    _oswPilots->move(0,_oslPilots->height()-5);
    createControllerWidget();
    _oswController->move(0,_oslPilots->height()-5);
    createNavDataWidget();
    _oswNavData->move(0,_oslPilots->height()-5);
    createWindWidget();
    _oswWeather->move(0, _oslPilots->height()-5);



    qDebug() << "MapScreen::MapScreen() created";

}

////////////////////////////
// Creating the widgets
////////////////////////////

void MapScreen::createPilotWidget()
{

    _oswPilots->setMouseTracking(true);
    _oswPilots->setAutoFillBackground(true);
    //W_Pilots->setContentsMargins(3, L_Pilots->height(), 3, 3);
    _oswPilots->setMinimumWidth(_xDistance+_oslWeather->width());

    //Show all routes button
    _pbRoutes = new QPushButton();
    _pbRoutes->setText(tr("all routes"));
    _pbRoutes->setCheckable(true);
    connect(_pbRoutes, SIGNAL(clicked()), this, SLOT(P_toggleRoutesClicked()));
    _hblP.addWidget(_pbRoutes);

    //Show pilots labels button
    _pbPilotLabels = new QPushButton();
    _pbPilotLabels->setText(tr("pilot labels"));
    _pbPilotLabels->setCheckable(true);
    _pbPilotLabels->setChecked(Settings::showPilotsLabels());
    connect(_pbPilotLabels, SIGNAL(clicked()), this, SLOT(P_togglePilotLabelsClicked()));
    _hblP.addWidget(_pbPilotLabels);


    _oswPilots->setLayout(&_hblP);
    _oswPilots->lower();
}

void MapScreen::createControllerWidget()
{

    _oswController->setMouseTracking(true);
    _oswController->setAutoFillBackground(true);
    //W_Controller->setContentsMargins(3, L_Pilots->height(), 3, 3);
    _oswController->setMinimumWidth(_xDistance+_oslWeather->width());



    _pbCtr = new QPushButton();
    _pbCtr->setText(tr("CTR/FSS"));
    _pbCtr->setCheckable(true);
    _pbCtr->setChecked(Settings::showCTR());
    connect(_pbCtr, SIGNAL(clicked()), this, SLOT(C_toggleCTRClicked()));
    _hblC.addWidget(_pbCtr);

    _pbApp = new QPushButton();
    _pbApp->setText(tr("APP"));
    _pbApp->setCheckable(true);
    _pbApp->setChecked(Settings::showAPP());
    connect(_pbApp, SIGNAL(clicked()), this, SLOT(C_toggleAPPClicked()));
    _hblC.addWidget(_pbApp);

    _pbTwr = new QPushButton();
    _pbTwr->setText(tr("TWR"));
    _pbTwr->setCheckable(true);
    _pbTwr->setChecked(Settings::showTWR());
    connect(_pbTwr, SIGNAL(clicked()), this, SLOT(C_toggleTWRClicked()));
    _hblC.addWidget(_pbTwr);

    _pbGnd = new QPushButton();
    _pbGnd->setText(tr("GND"));
    _pbGnd->setCheckable(true);
    _pbGnd->setChecked(Settings::showGND());
    connect(_pbGnd, SIGNAL(clicked()), this, SLOT(C_toggleGNDClicked()));
    _hblC.addWidget(_pbGnd);


    _oswController->setLayout(&_hblC);
    _oswController->lower();
}

void MapScreen::createNavDataWidget()
{

    _oswNavData->setMouseTracking(true);
    _oswNavData->setAutoFillBackground(true);
    _oswNavData->setMinimumWidth(_xDistance+_oslWeather->width());
    //W_NavData->setContentsMargins(3, L_Pilots->height(), 3, 3);

    _pbSectorsAll = new QPushButton();
    _pbSectorsAll->setText(tr("all sectors"));
    _pbSectorsAll->setCheckable(true);
    _pbSectorsAll->setChecked(Settings::showAllSectors());
    connect(_pbSectorsAll, SIGNAL(clicked()), this, SLOT(N_toggleSectorClicked()));
    _hblN.addWidget(_pbSectorsAll);

    _pbRouteWaypoints = new QPushButton();
    _pbRouteWaypoints->setText(tr("route waypoints"));
    _pbRouteWaypoints->setCheckable(true);
    _pbRouteWaypoints->setChecked(Settings::showRouteFix());
    connect(_pbRouteWaypoints, SIGNAL(clicked()), this, SLOT(N_toggleRouteFixClicked()));
    _hblN.addWidget(_pbRouteWaypoints);

    _pbInactiveAirports = new QPushButton();
    _pbInactiveAirports->setText(tr("inactive airports"));
    _pbInactiveAirports->setCheckable(true);
    _pbInactiveAirports->setChecked(Settings::showInactiveAirports());
    connect(_pbInactiveAirports, SIGNAL(clicked()), this, SLOT(N_toggleInactiveClicked()));
    _hblN.addWidget(_pbInactiveAirports);



    _vblN.addLayout(&_hblN);
    _oswNavData->setLayout(&_vblN);
    _oswNavData->lower();
}

void MapScreen::createWindWidget() {
    _oswWeather->setMouseTracking(true);
    _oswWeather->setAutoFillBackground(true);
    _oswWeather->setMinimumWidth(_xDistance+_oslWeather->width());


    // On/Off buttons for wind and clouds
    _pbWind = new QPushButton();
    _pbWind->setText(tr("upperwind"));
    _pbWind->setCheckable(true);
    _pbWind->setChecked(Settings::showWind());
    connect(_pbWind, SIGNAL(clicked()), this, SLOT(W_toggleWindClicked()));
    _hblW.addWidget(_pbWind);

    _pbClouds = new QPushButton;
    _pbClouds->setText(tr("clouds"));
    _pbClouds->setCheckable(true);
    _pbClouds->setChecked(Settings::showClouds());
    //if(!Settings::downloadClouds()) W_toggleClouds->setEnabled(false);
    connect(_pbClouds, SIGNAL(clicked()), this, SLOT(W_toggleCloudsClicked()));
    _hblW.addWidget(_pbClouds);


    //upperwind settings
    _pbWindAltDec = new QPushButton();
    _pbWindAltDec->setText(tr("-"));
    connect(_pbWindAltDec, SIGNAL(clicked()), this, SLOT(W_minusClicked()));
    _hblW1.addWidget(_pbWindAltDec);

    _slider = new QSlider();
    _slider->setRange(0, 40);
    _slider->setOrientation(Qt::Horizontal);
    _slider->setValue(Settings::windAlt());
    connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(W_sliderChanged()));
    _hblW1.addWidget(_slider);

    _pbWindAltInc = new QPushButton();
    _pbWindAltInc->setText(tr("+"));
    connect(_pbWindAltInc, SIGNAL(clicked()), this, SLOT(W_plusClicked()));
    _hblW1.addWidget(_pbWindAltInc);

    _lblWindAlt = new QLabel();
    _lblWindAlt->setText(QString("%1ft").arg((Settings::windAlt()*1000)));
    _hblW1.addWidget(_lblWindAlt);

    _vblW.addLayout(&_hblW);
    _vblW.addLayout(&_hblW1);

    _oswWeather->setLayout(&_vblW);
    _oswWeather->lower();
}

void MapScreen::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    glWidget->resize(this->width(),this->height());
}


////////////////////////////
// Mouse moved slots
////////////////////////////

void MapScreen::LabelEntered(int typ)
{
    QFont f = QFont("Arial", 16, QFont::Bold);
    f.setUnderline(true);
    switch(typ) {
    case 0:
        return;
        break;
    case 1:
        _oswPilots->raise();
        _oslPilots->raise();
        _oslController->raise();
        _oslNavData->raise();
        _oslWeather->raise();
        _oswController->lower();
        _oswNavData->lower();
        _oswWeather->lower();

        _oslPilots->setFont(f);
        f.setUnderline(false);
        _oslController->setFont(f);
        _oslNavData->setFont(f);
        _oslWeather->setFont(f);
        break;
    case 2:
        _oswController->raise();
        _oslController->raise();
        _oslPilots->raise();
        _oslNavData->raise();
        _oslWeather->raise();
        _oswPilots->lower();
        _oswNavData->lower();
        _oswWeather->lower();

        _oslController->setFont(f);
        f.setUnderline(false);
        _oslPilots->setFont(f);
        _oslNavData->setFont(f);
        _oslWeather->setFont(f);
        break;
    case 3:
        _oswNavData->raise();
        _oslNavData->raise();
        _oslWeather->raise();
        _oslController->raise();
        _oslPilots->raise();
        _oswPilots->lower();
        _oswController->lower();
        _oswWeather->lower();

        _oslNavData->setFont(f);
        f.setUnderline(false);
        _oslPilots->setFont(f);
        _oslController->setFont(f);
        _oslWeather->setFont(f);
        break;

    case 4:
        _oswWeather->raise();
        _oslWeather->raise();
        _oslPilots->raise();
        _oslController->raise();
        _oslNavData->raise();
        _oswPilots->lower();
        _oswController->lower();
        _oswNavData->lower();

        _oslWeather->setFont(f);
        f.setUnderline(false);
        _oslPilots->setFont(f);
        _oslController->setFont(f);
        _oslNavData->setFont(f);
        break;

    default:
        return;
        break;
    }
}

void MapScreen::LabelLeft(int typ)
{
    QFont f = QFont("Arial", 16, QFont::Bold);
    f.setUnderline(false);
    switch(typ) {
    case 0:
        return;
        break;
    case 1:
        _oslPilots->setFont(f);
        //W_Pilots->lower();
        break;
    case 2:
        _oslController->setFont(f);
        //W_Controller->lower();
        break;
    case 3:
        _oslNavData->setFont(f);
        //W_NavData->lower();
        break;

    case 4:
        _oslWeather->setFont(f);
        //W_Weather->lower();
        break;

    default:
        return;
        break;
    }
}

void MapScreen::WidgetEntered(int typ)
{
    QFont f = QFont("Arial", 16, QFont::Bold);
    f.setUnderline(true);
    switch(typ) {
    case 0:
        return;
        break;
    case 1:
        _oswPilots->raise();
        _oslPilots->raise();
        _oslController->raise();
        _oslNavData->raise();
        _oslWeather->raise();
        _oswController->lower();
        _oswNavData->lower();
        _oswWeather->lower();

        _oslPilots->setFont(f);
        f.setUnderline(false);
        _oslController->setFont(f);
        _oslNavData->setFont(f);
        _oslWeather->setFont(f);
        break;
    case 2:
        _oswController->raise();
        _oslController->raise();
        _oslPilots->raise();
        _oslNavData->raise();
        _oslWeather->raise();
        _oswPilots->lower();
        _oswNavData->lower();
        _oswWeather->lower();

        _oslController->setFont(f);
        f.setUnderline(false);
        _oslPilots->setFont(f);
        _oslNavData->setFont(f);
        _oslWeather->setFont(f);
        break;
    case 3:
        _oswNavData->raise();
        _oslNavData->raise();
        _oslPilots->raise();
        _oslController->raise();
        _oslWeather->raise();
        _oswPilots->lower();
        _oswController->lower();
        _oswWeather->lower();

        _oslNavData->setFont(f);
        f.setUnderline(false);
        _oslPilots->setFont(f);
        _oslController->setFont(f);
        _oslWeather->setFont(f);
        break;

    case 4:
        _oswWeather->raise();
        _oslWeather->raise();
        _oslPilots->raise();
        _oslController->raise();
        _oslNavData->raise();
        _oswPilots->lower();
        _oswController->lower();
        _oswNavData->lower();

        _oslWeather->setFont(f);
        f.setUnderline(false);
        _oslPilots->setFont(f);
        _oslController->setFont(f);
        _oslNavData->setFont(f);
        break;
    default:
        return;
        break;
    }
}

void MapScreen::WidgetLeft(int typ)
{
    QFont f = QFont("Arial", 16, QFont::Bold);
    f.setUnderline(false);
    switch(typ) {
    case 0:
        return;
        break;
    case 1:
        _oswPilots->lower();
        _oslPilots->raise();
        _oslController->raise();
        _oslNavData->raise();
        _oslWeather->raise();

        _oslPilots->setFont(f);
        break;
    case 2:
        _oswController->lower();
        _oslPilots->raise();
        _oslController->raise();
        _oslNavData->raise();
        _oslWeather->raise();

        _oslController->setFont(f);
        break;
    case 3:
        _oswNavData->lower();
        _oslPilots->raise();
        _oslController->raise();
        _oslNavData->raise();
        _oslWeather->raise();

        _oslNavData->setFont(f);
        break;
    case 4:
        _oswWeather->lower();
        _oslPilots->raise();
        _oslController->raise();
        _oslNavData->raise();
        _oslWeather->raise();

        _oslWeather->setFont(f);
        break;
    default:
        return;
        break;
    }
}


////////////////////////////
// Pilots funktions
////////////////////////////

void MapScreen::P_toggleRoutesClicked()
{
    emit toggleRoutes();

}

void MapScreen::P_togglePilotLabelsClicked()
{
    Settings::setShowPilotsLabels(_pbPilotLabels->isChecked());
    glWidget->updateGL();
}

////////////////////////////
// Controller funktions
////////////////////////////

void MapScreen::C_toggleCTRClicked()
{
    Settings::setShowCTR(_pbCtr->isChecked());
    glWidget->updateGL();
}

void MapScreen::C_toggleAPPClicked()
{
    Settings::setShowAPP(_pbApp->isChecked());
    glWidget->updateGL();
}

void MapScreen::C_toggleTWRClicked()
{
    Settings::setShowTWR(_pbTwr->isChecked());
    glWidget->updateGL();
}

void MapScreen::C_toggleGNDClicked()
{
    Settings::setShowGND(_pbGnd->isChecked());
    glWidget->updateGL();
}


////////////////////////////
// NavData funktions
////////////////////////////

void MapScreen::N_toggleSectorClicked()
{
    //Settings::setShowAllSectors(N_sectorsAll->isChecked());
    emit toggleSectors(_pbSectorsAll->isChecked());
}

void MapScreen::toggleSectorChanged(bool state)
{
    if(_pbSectorsAll->isChecked() != state)
    {
        _pbSectorsAll->setChecked(state);
    }
}

void MapScreen::N_toggleRouteFixClicked()
{
    Settings::setShowRouteFix(_pbRouteWaypoints->isChecked());
    emit toggleRouteWaypoints();
}

void MapScreen::N_toggleInactiveClicked() {
    Settings::setShowInactiveAirports(_pbInactiveAirports->isChecked());
    emit toggleInactiveAirports();
}

////////////////////////////
// Weather funktions
////////////////////////////

void MapScreen::W_toggleWindClicked() {
    Settings::setShowWind(!Settings::showWind());
    if (WindData::instance()->status() == -1) // not yet downloaded
        Launcher::instance(true)->startWindDownload();
    glWidget->update();
}

void MapScreen::W_minusClicked()
{
    int value = _slider->value();
    if(value == 0) return;

    value -= 1;
    _slider->setValue(value);
}

void MapScreen::W_plusClicked()
{
    int value = _slider->value();
    if(value == 40) return;

    value += 1;
    _slider->setValue(value);
}

void MapScreen::W_sliderChanged()
{
    Settings::setWindAlt(_slider->value());
    _lblWindAlt->setText(QString("%1ft").arg((_slider->value()*1000)));
    glWidget->update();
}

void MapScreen::W_toggleCloudsClicked()
{
    if(!Settings::showClouds()) {
        Settings::setShowClouds(true);
        glWidget->useClouds();
        return;
    }

    Settings::setShowClouds(false);
    glWidget->useClouds();
    return;
}

///////////////////////////
// Label and Widget Class
///////////////////////////

OnScreenLabel::OnScreenLabel(QWidget *parent) :
        QLabel(parent)
{
    typ = 0;
}

void OnScreenLabel::enterEvent(QEvent *)
{
    emit entered(typ);
}

void OnScreenLabel::leaveEvent(QEvent *)
{
    emit left(typ);
}

OnScreenWidget::OnScreenWidget(QWidget *parent) :
        QWidget(parent)
{
    typ = 0;
}

void OnScreenWidget::enterEvent(QEvent *)
{
    emit entered(typ);
}

void OnScreenWidget::leaveEvent(QEvent *)
{
    emit left(typ);
}





















