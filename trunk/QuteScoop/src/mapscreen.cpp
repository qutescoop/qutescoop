#include "mapscreen.h"
#include <QHBoxLayout>

MapScreen::MapScreen(QWidget *parent) :
    QWidget(parent)
{

    this->setMouseTracking(true);
    xDistance = 0;
    P_visible = false;
    C_visible = false;
    N_visible = false;



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
    glWidget->setMouseTracking(true);
    connect(glWidget,SIGNAL(mouseMoved(QMouseEvent*)), this, SLOT(glMouseMoved(QMouseEvent*))); //to recive MouseMove Events form GLWindget
    qDebug() << "MapScreen::MapScreen() creating GLWidget --finished";


    L_Pilots = new QLabel(this);
    L_Pilots->setText(tr("Pilots"));
    L_Pilots->setFont(QFont("Arial", 16, QFont::Bold));
    L_Pilots->setAutoFillBackground(true);
    L_Pilots->setMouseTracking(true);
    L_Pilots->raise();
    xDistance += 60;

    L_Controller = new QLabel(this);
    L_Controller->setText(tr("Controller"));
    L_Controller->setFont(QFont("Arial", 16, QFont::Bold));
    L_Controller->setAutoFillBackground(true);
    L_Controller->setMouseTracking(true);
    L_Controller->move(xDistance, 0);
    L_Controller->raise();
    xDistance += 103;

    L_NavData = new QLabel(this);
    L_NavData->setText(tr("NavData"));
    L_NavData->setFont(QFont("Arial", 16, QFont::Bold));
    L_NavData->setAutoFillBackground(true);
    L_NavData->setMouseTracking(true);
    L_NavData->move(xDistance,0);
    L_NavData->raise();

    W_Pilots = new QWidget(this);
    W_Controller = new QWidget(this);
    W_NavData = new QWidget(this);

    createPilotWidget();
    createControllerWidget();
    createNavDataWidget();

    qDebug() << "MapScreen::MapScreen() created";

}

////////////////////////////
// Creating the widgets
////////////////////////////

void MapScreen::createPilotWidget()
{

    W_Pilots->setMouseTracking(true);
    W_Pilots->setAutoFillBackground(true);
    W_Pilots->setContentsMargins(3, L_Pilots->height(), 3, 3);
    W_Pilots->setMinimumWidth(xDistance+L_NavData->width());

    //Show all routes button
    P_toggleRoutes = new QPushButton();
    P_toggleRoutes->setText(tr("Show all Routes"));
    P_toggleRoutes->setCheckable(true);
    connect(P_toggleRoutes, SIGNAL(clicked()), this, SLOT(P_toggleRoutesClicked()));
    P_layout.addWidget(P_toggleRoutes);


    W_Pilots->setLayout(&P_layout);
    W_Pilots->lower();
}

void MapScreen::createControllerWidget()
{

    W_Controller->setMouseTracking(true);
    W_Controller->setAutoFillBackground(true);
    W_Controller->setContentsMargins(3, L_Pilots->height(), 3, 3);
    W_Controller->setMinimumWidth(xDistance+L_NavData->width());


    C_toggleCTR = new QPushButton();
    C_toggleCTR->setText(tr("Center"));
    C_toggleCTR->setCheckable(true);
    C_toggleCTR->setChecked(Settings::showCTR());
    connect(C_toggleCTR, SIGNAL(clicked()), this, SLOT(C_toggleCTRClicked()));
    C_layout.addWidget(C_toggleCTR);

    C_toggleAPP = new QPushButton();
    C_toggleAPP->setText(tr("Approach"));
    C_toggleAPP->setCheckable(true);
    C_toggleAPP->setChecked(Settings::showAPP());
    connect(C_toggleAPP, SIGNAL(clicked()), this, SLOT(C_toggleAPPClicked()));
    C_layout.addWidget(C_toggleAPP);

    C_toggleTWR = new QPushButton();
    C_toggleTWR->setText(tr("Tower"));
    C_toggleTWR->setCheckable(true);
    C_toggleTWR->setChecked(Settings::showTWR());
    connect(C_toggleTWR, SIGNAL(clicked()), this, SLOT(C_toggleTWRClicked()));
    C_layout.addWidget(C_toggleTWR);

    C_toggleGND = new QPushButton();
    C_toggleGND->setText(tr("Ground"));
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
    W_NavData->setMinimumWidth(xDistance+L_NavData->width());
    W_NavData->setContentsMargins(3, L_Pilots->height(), 3, 3);

    N_sectorsAll = new QPushButton();
    N_sectorsAll->setText(tr("Show all Sectors"));
    N_sectorsAll->setCheckable(true);
    N_sectorsAll->setChecked(Settings::showAllSectors());
    connect(N_sectorsAll, SIGNAL(clicked()), this, SLOT(N_toggleSectorClicked()));
    N_layout.addWidget(N_sectorsAll);

    N_RouteWaypoints = new QPushButton();
    N_RouteWaypoints->setText(tr("Show Route Waypoints"));
    N_RouteWaypoints->setCheckable(true);
    N_RouteWaypoints->setChecked(Settings::showRouteFix());
    connect(N_RouteWaypoints, SIGNAL(clicked()), this, SLOT(N_toggleRouteFixClicked()));
    N_layout.addWidget(N_RouteWaypoints);

    N_InactiveAirports = new QPushButton();
    N_InactiveAirports->setText(tr("Show inactive Airports"));
    N_InactiveAirports->setCheckable(true);
    N_InactiveAirports->setChecked(Settings::showInactiveAirports());
    connect(N_InactiveAirports, SIGNAL(clicked()), this, SLOT(N_toggleInactiveClicked()));
    N_layout.addWidget(N_InactiveAirports);

    W_NavData->setLayout(&N_layout);
    W_NavData->lower();
}



void MapScreen::resizeEvent(QResizeEvent *event)
{
    glWidget->resize(this->width(),this->height());
}


////////////////////////////
// Mouse moved
////////////////////////////

void MapScreen::glMouseMoved(QMouseEvent *event)
{
    mouseMoveEvent(event);
}

void MapScreen::mouseMoveEvent (QMouseEvent *event)
{

    //Check if couser over Labels
    if(event->y()<25 && event->x()< 60){
        P_visible = true;
        W_Pilots->raise();
        L_Pilots->raise();
        C_visible = false;
        W_Controller->lower();
        N_visible = false;
        W_NavData->lower();
    }

    if(event->y()<25 && event->x()>60  && event->x()<163) {
        C_visible = true;
        W_Controller->raise();
        L_Controller->raise();
        P_visible = false;
        N_visible = false;
    }

    if(event->y()<25 && event->x()>163 && event->x()<243){
        N_visible = true;
        W_NavData->raise();
        L_NavData->raise();
        P_visible = false;
        C_visible = false;
    }

    //Check if couser over on of the widgets
    if(P_visible && event->y()<W_Pilots->height() && event->x()<W_Pilots->width()){
        W_Pilots->raise();
        L_Pilots->raise();
    }else{
        W_Pilots->lower();
    }

    if(C_visible && event->y()<W_Controller->height() && event->x()<W_Controller->width()){
        W_Controller->raise();
        L_Controller->raise();

    }else{
        W_Controller->lower();
    }

    if(N_visible && event->y()<W_NavData->height() && event->x()<W_NavData->width()){
        W_NavData->raise();
        L_NavData->raise();
    }else{
        W_NavData->lower();
    }

    return;
}

////////////////////////////
// Pilots funktions
////////////////////////////

void MapScreen::P_toggleRoutesClicked()
{
    emit toggleRoutes();

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
    Settings::setShowAllSectors(N_sectorsAll->isChecked());
    emit toggleSectors();
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



