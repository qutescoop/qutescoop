#include "mapscreen.h"
#include <QHBoxLayout>

MapScreen::MapScreen(QWidget *parent) :
    QWidget(parent)
{

    this->setMouseTracking(true);
    xDistance = 0;




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


    L_Pilots = new OnScreenLabel(this);
    L_Pilots->typ = 1;
    connect(L_Pilots, SIGNAL(entered(int)), this, SLOT(LabelEntered(int)));
    L_Pilots->setText(tr("Pilots"));
    L_Pilots->setFont(QFont("Arial", 16, QFont::Bold));
    L_Pilots->setAutoFillBackground(true);
    L_Pilots->setMouseTracking(true);
    L_Pilots->raise();
    xDistance += 59;

    L_Controller = new OnScreenLabel(this);
    L_Controller->typ = 2;
    connect(L_Controller, SIGNAL(entered(int)), this, SLOT(LabelEntered(int)));
    L_Controller->setText(tr("Controller"));
    L_Controller->setFont(QFont("Arial", 16, QFont::Bold));
    L_Controller->setAutoFillBackground(true);
    L_Controller->setMouseTracking(true);
    L_Controller->move(xDistance, 0);
    L_Controller->raise();
    xDistance += 103;

    L_NavData = new OnScreenLabel(this);
    L_NavData->typ = 3;
    connect(L_NavData, SIGNAL(entered(int)), this, SLOT(LabelEntered(int)));
    L_NavData->setText(tr("NavData"));
    L_NavData->setFont(QFont("Arial", 16, QFont::Bold));
    L_NavData->setAutoFillBackground(true);
    L_NavData->setMouseTracking(true);
    L_NavData->move(xDistance,0);
    L_NavData->raise();
    xDistance += 87;

    L_Weather = new OnScreenLabel(this);
    L_Weather->typ = 4;
    connect(L_Weather, SIGNAL(entered(int)), this, SLOT(LabelEntered(int)));
    L_Weather->setText(tr("Weather"));
    L_Weather->setFont(QFont("Arial", 16, QFont::Bold));
    L_Weather->setAutoFillBackground(true);
    L_Weather->setMouseTracking(true);
    L_Weather->move(xDistance,0);
    L_Weather->raise();

    W_Pilots = new OnScreenWidget(this);
    W_Pilots->typ = 1;
    connect(W_Pilots, SIGNAL(entered(int)), this, SLOT(WidgetEntered(int)));
    connect(W_Pilots, SIGNAL(left(int)), this, SLOT(WidgetLeft(int)));

    W_Controller = new OnScreenWidget(this);
    W_Controller->typ = 2;
    connect(W_Controller, SIGNAL(entered(int)), this, SLOT(WidgetEntered(int)));
    connect(W_Controller, SIGNAL(left(int)), this, SLOT(WidgetLeft(int)));

    W_NavData = new OnScreenWidget(this);
    W_NavData->typ = 3;
    connect(W_NavData, SIGNAL(entered(int)), this, SLOT(WidgetEntered(int)));
    connect(W_NavData, SIGNAL(left(int)), this, SLOT(WidgetLeft(int)));

    W_Weather = new OnScreenWidget(this);
    W_Weather->typ = 4;
    connect(W_Weather, SIGNAL(entered(int)), this, SLOT(WidgetEntered(int)));
    connect(W_Weather, SIGNAL(left(int)), this, SLOT(WidgetLeft(int)));

    createPilotWidget();
    W_Pilots->move(0,L_Pilots->height()-5);
    createControllerWidget();
    W_Controller->move(0,L_Pilots->height()-5);
    createNavDataWidget();
    W_NavData->move(0,L_Pilots->height()-5);
    createWindWidget();
    W_Weather->move(0, L_Pilots->height()-5);



    qDebug() << "MapScreen::MapScreen() created";

}

////////////////////////////
// Creating the widgets
////////////////////////////

void MapScreen::createPilotWidget()
{

    W_Pilots->setMouseTracking(true);
    W_Pilots->setAutoFillBackground(true);
    //W_Pilots->setContentsMargins(3, L_Pilots->height(), 3, 3);
    W_Pilots->setMinimumWidth(xDistance+L_Weather->width());

    //Show all routes button
    P_toggleRoutes = new QPushButton();
    P_toggleRoutes->setText(tr("all routes"));
    P_toggleRoutes->setCheckable(true);
    connect(P_toggleRoutes, SIGNAL(clicked()), this, SLOT(P_toggleRoutesClicked()));
    P_layout.addWidget(P_toggleRoutes);

    //Show pilots labels button
    P_togglePilotLabels = new QPushButton();
    P_togglePilotLabels->setText(tr("pilot labels"));
    P_togglePilotLabels->setCheckable(true);
    P_togglePilotLabels->setChecked(Settings::showPilotsLabels());
    connect(P_togglePilotLabels, SIGNAL(clicked()), this, SLOT(P_togglePilotLabelsClicked()));
    P_layout.addWidget(P_togglePilotLabels);


    W_Pilots->setLayout(&P_layout);
    W_Pilots->lower();
}

void MapScreen::createControllerWidget()
{

    W_Controller->setMouseTracking(true);
    W_Controller->setAutoFillBackground(true);
    //W_Controller->setContentsMargins(3, L_Pilots->height(), 3, 3);
    W_Controller->setMinimumWidth(xDistance+L_Weather->width());


    C_toggleCTR = new QPushButton();
    C_toggleCTR->setText(tr("CTR/FSS"));
    C_toggleCTR->setCheckable(true);
    C_toggleCTR->setChecked(Settings::showCTR());
    connect(C_toggleCTR, SIGNAL(clicked()), this, SLOT(C_toggleCTRClicked()));
    C_layout.addWidget(C_toggleCTR);

    C_toggleAPP = new QPushButton();
    C_toggleAPP->setText(tr("APP"));
    C_toggleAPP->setCheckable(true);
    C_toggleAPP->setChecked(Settings::showAPP());
    connect(C_toggleAPP, SIGNAL(clicked()), this, SLOT(C_toggleAPPClicked()));
    C_layout.addWidget(C_toggleAPP);

    C_toggleTWR = new QPushButton();
    C_toggleTWR->setText(tr("TWR"));
    C_toggleTWR->setCheckable(true);
    C_toggleTWR->setChecked(Settings::showTWR());
    connect(C_toggleTWR, SIGNAL(clicked()), this, SLOT(C_toggleTWRClicked()));
    C_layout.addWidget(C_toggleTWR);

    C_toggleGND = new QPushButton();
    C_toggleGND->setText(tr("GND"));
    C_toggleGND->setCheckable(true);
    C_toggleGND->setChecked(Settings::showGND());
    connect(C_toggleGND, SIGNAL(clicked()), this, SLOT(C_toggleGNDClicked()));
    C_layout.addWidget(C_toggleGND);


    W_Controller->setLayout(&C_layout);
    W_Controller->lower();
}

void MapScreen::createNavDataWidget()
{

    W_NavData->setMouseTracking(true);
    W_NavData->setAutoFillBackground(true);
    W_NavData->setMinimumWidth(xDistance+L_Weather->width());
    //W_NavData->setContentsMargins(3, L_Pilots->height(), 3, 3);

    N_sectorsAll = new QPushButton();
    N_sectorsAll->setText(tr("all sectors"));
    N_sectorsAll->setCheckable(true);
    N_sectorsAll->setChecked(Settings::showAllSectors());
    connect(N_sectorsAll, SIGNAL(clicked()), this, SLOT(N_toggleSectorClicked()));
    N_layout.addWidget(N_sectorsAll);

    N_RouteWaypoints = new QPushButton();
    N_RouteWaypoints->setText(tr("route waypoints"));
    N_RouteWaypoints->setCheckable(true);
    N_RouteWaypoints->setChecked(Settings::showRouteFix());
    connect(N_RouteWaypoints, SIGNAL(clicked()), this, SLOT(N_toggleRouteFixClicked()));
    N_layout.addWidget(N_RouteWaypoints);

    N_InactiveAirports = new QPushButton();
    N_InactiveAirports->setText(tr("inactive airports"));
    N_InactiveAirports->setCheckable(true);
    N_InactiveAirports->setChecked(Settings::showInactiveAirports());
    connect(N_InactiveAirports, SIGNAL(clicked()), this, SLOT(N_toggleInactiveClicked()));
    N_layout.addWidget(N_InactiveAirports);



    N_Vlayout.addLayout(&N_layout);
    W_NavData->setLayout(&N_Vlayout);
    W_NavData->lower();
}

void MapScreen::createWindWidget()
{
    W_Weather->setMouseTracking(true);
    W_Weather->setAutoFillBackground(true);
    W_Weather->setMinimumWidth(xDistance+L_Weather->width());

    // On/Off buttons for wind and clouds
    W_toggleWind = new QPushButton();
    W_toggleWind->setText(tr("upperwind"));
    W_toggleWind->setCheckable(true);
    W_toggleWind->setChecked(Settings::showUpperWind());
    connect(W_toggleWind, SIGNAL(clicked()), this, SLOT(W_toggleWindClicked()));
    W_layout.addWidget(W_toggleWind);

    W_toggleClouds = new QPushButton;
    W_toggleClouds->setText(tr("clouds"));
    W_toggleClouds->setCheckable(true);
    W_toggleClouds->setChecked(Settings::showClouds());
    connect(W_toggleClouds, SIGNAL(clicked()), this, SLOT(W_toggleCloudsClicked()));
    W_layout.addWidget(W_toggleClouds);


    //upperwind settings
    W_minus = new QPushButton();
    W_minus->setText(tr("-"));
    connect(W_minus, SIGNAL(clicked()), this, SLOT(W_minusClicked()));
    W_layout1.addWidget(W_minus);

    W_slider = new QSlider();
    W_slider->setRange(0, 40);
    W_slider->setOrientation(Qt::Horizontal);
    W_slider->setValue(Settings::upperWindAlt());
    connect(W_slider, SIGNAL(valueChanged(int)), this, SLOT(W_sliderChanged()));
    W_layout1.addWidget(W_slider);

    W_plus = new QPushButton();
    W_plus->setText(tr("+"));
    connect(W_plus, SIGNAL(clicked()), this, SLOT(W_plusClicked()));
    W_layout1.addWidget(W_plus);

    W_windAlt = new QLabel();
    W_windAlt->setText(QString("%1").arg((Settings::upperWindAlt()*1000)));
    W_layout1.addWidget(W_windAlt);

    W_Vlayout.addLayout(&W_layout);
    W_Vlayout.addLayout(&W_layout1);

    W_Weather->setLayout(&W_Vlayout);
    W_Weather->lower();
}

void MapScreen::resizeEvent(QResizeEvent *event)
{
    glWidget->resize(this->width(),this->height());
}


////////////////////////////
// Mouse moved slots
////////////////////////////

void MapScreen::LabelEntered(int typ)
{
    QFont f = QFont("Arial", 16, QFont::Bold);
    f.setUnderline(true);
    switch(typ){
    case 0:
        return;
        break;
    case 1:
        W_Pilots->raise();
        L_Pilots->raise();
        L_Controller->raise();
        L_NavData->raise();
        L_Weather->raise();
        W_Controller->lower();
        W_NavData->lower();
        W_Weather->lower();

        L_Pilots->setFont(f);
        f.setUnderline(false);
        L_Controller->setFont(f);
        L_NavData->setFont(f);
        L_Weather->setFont(f);
        break;
    case 2:
        W_Controller->raise();
        L_Controller->raise();
        L_Pilots->raise();
        L_NavData->raise();
        L_Weather->raise();
        W_Pilots->lower();
        W_NavData->lower();
        W_Weather->lower();

        L_Controller->setFont(f);
        f.setUnderline(false);
        L_Pilots->setFont(f);
        L_NavData->setFont(f);
        L_Weather->setFont(f);
        break;
    case 3:
        W_NavData->raise();
        L_NavData->raise();
        L_Weather->raise();
        L_Controller->raise();
        L_Pilots->raise();
        W_Pilots->lower();
        W_Controller->lower();
        W_Weather->lower();

        L_NavData->setFont(f);
        f.setUnderline(false);
        L_Pilots->setFont(f);
        L_Controller->setFont(f);
        L_Weather->setFont(f);
        break;

    case 4:
        W_Weather->raise();
        L_Weather->raise();
        L_Pilots->raise();
        L_Controller->raise();
        L_NavData->raise();
        W_Pilots->lower();
        W_Controller->lower();
        W_NavData->lower();

        L_Weather->setFont(f);
        f.setUnderline(false);
        L_Pilots->setFont(f);
        L_Controller->setFont(f);
        L_NavData->setFont(f);
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
    switch(typ){
    case 0:
        return;
        break;
    case 1:
        L_Pilots->setFont(f);
        break;
    case 2:
        L_Controller->setFont(f);
        break;
    case 3:
        L_NavData->setFont(f);
        break;

    case 4:
        L_Weather->setFont(f);
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
    switch(typ){
    case 0:
        return;
        break;
    case 1:
        W_Pilots->raise();
        L_Pilots->raise();
        L_Controller->raise();
        L_NavData->raise();
        L_Weather->raise();
        W_Controller->lower();
        W_NavData->lower();
        W_Weather->lower();

        L_Pilots->setFont(f);
        f.setUnderline(false);
        L_Controller->setFont(f);
        L_NavData->setFont(f);
        L_Weather->setFont(f);
        break;
    case 2:
        W_Controller->raise();
        L_Controller->raise();
        L_Pilots->raise();
        L_NavData->raise();
        L_Weather->raise();
        W_Pilots->lower();
        W_NavData->lower();
        W_Weather->lower();

        L_Controller->setFont(f);
        f.setUnderline(false);
        L_Pilots->setFont(f);
        L_NavData->setFont(f);
        L_Weather->setFont(f);
        break;
    case 3:
        W_NavData->raise();
        L_NavData->raise();
        L_Pilots->raise();
        L_Controller->raise();
        L_Weather->raise();
        W_Pilots->lower();
        W_Controller->lower();
        W_Weather->lower();

        L_NavData->setFont(f);
        f.setUnderline(false);
        L_Pilots->setFont(f);
        L_Controller->setFont(f);
        L_Weather->setFont(f);
        break;

    case 4:
        W_Weather->raise();
        L_Weather->raise();
        L_Pilots->raise();
        L_Controller->raise();
        L_NavData->raise();
        W_Pilots->lower();
        W_Controller->lower();
        W_NavData->lower();

        L_Weather->setFont(f);
        f.setUnderline(false);
        L_Pilots->setFont(f);
        L_Controller->setFont(f);
        L_NavData->setFont(f);
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
    switch(typ){
    case 0:
        return;
        break;
    case 1:
        W_Pilots->lower();
        L_Pilots->raise();
        L_Controller->raise();
        L_NavData->raise();
        L_Weather->raise();

        L_Pilots->setFont(f);
        break;
    case 2:
        W_Controller->lower();
        L_Pilots->raise();
        L_Controller->raise();
        L_NavData->raise();
        L_Weather->raise();

        L_Controller->setFont(f);
        break;
    case 3:
        W_NavData->lower();
        L_Pilots->raise();
        L_Controller->raise();
        L_NavData->raise();
        L_Weather->raise();

        L_NavData->setFont(f);
        break;
    case 4:
        W_Weather->lower();
        L_Pilots->raise();
        L_Controller->raise();
        L_NavData->raise();
        L_Weather->raise();

        L_Weather->setFont(f);
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
    Settings::setShowPilotsLabels(P_togglePilotLabels->isChecked());
    glWidget->updateGL();
}

////////////////////////////
// Controller funktions
////////////////////////////

void MapScreen::C_toggleCTRClicked()
{
    Settings::setShowCTR(C_toggleCTR->isChecked());
    glWidget->updateGL();
}

void MapScreen::C_toggleAPPClicked()
{
    Settings::setShowAPP(C_toggleAPP->isChecked());
    glWidget->updateGL();
}

void MapScreen::C_toggleTWRClicked()
{
    Settings::setShowTWR(C_toggleTWR->isChecked());
    glWidget->updateGL();
}

void MapScreen::C_toggleGNDClicked()
{
    Settings::setShowGND(C_toggleGND->isChecked());
    glWidget->updateGL();
}


////////////////////////////
// NavData funktions
////////////////////////////

void MapScreen::N_toggleSectorClicked()
{
    //Settings::setShowAllSectors(N_sectorsAll->isChecked());
    emit toggleSectors(N_sectorsAll->isChecked());
}

void MapScreen::toggleSectorChanged(bool state)
{
    if(N_sectorsAll->isChecked() != state)
    {
        N_sectorsAll->setChecked(state);
    }
}

void MapScreen::N_toggleRouteFixClicked()
{
    Settings::setShowRouteFix(N_RouteWaypoints->isChecked());
    emit toggleRouteWaypoints();
}

void MapScreen::N_toggleInactiveClicked()
{
    Settings::setShowInactiveAirports(N_InactiveAirports->isChecked());
    emit toggleInactiveAirports();
}

////////////////////////////
// Weather funktions
////////////////////////////

void MapScreen::W_toggleWindClicked()
{
    if(Settings::showUpperWind() == false){
        Settings::setShowUpperWind(true);
        glWidget->update();
        return;
    }

    Settings::setShowUpperWind(false);
    glWidget->update();
    return;
}

void MapScreen::W_minusClicked()
{
    int value = W_slider->value();
    if(value == 0) return;

    value -= 1;
    W_slider->setValue(value);
}

void MapScreen::W_plusClicked()
{
    int value = W_slider->value();
    if(value == 40) return;

    value += 1;
    W_slider->setValue(value);
}

void MapScreen::W_sliderChanged()
{
    Settings::setUpperWindAlt(W_slider->value());
    W_windAlt->setText(QString("%1").arg((W_slider->value()*1000)));
    glWidget->update();
}

void MapScreen::W_toggleCloudsClicked()
{
    if(Settings::showClouds() == false){
        Settings::setShowClouds(true);
        glWidget->useClouds();
        //glWidget->update();
        return;
    }

    Settings::setShowClouds(false);
    glWidget->useClouds();
    //glWidget->update();
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





















