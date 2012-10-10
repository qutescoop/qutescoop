#ifndef MAPSCREEN_H
#define MAPSCREEN_H

#include "_pch.h"

#include "GLWidget.h"
#include "Settings.h"


class OnScreenLabel : public QLabel
{
Q_OBJECT
public:
    OnScreenLabel(QWidget *parent = 0);
    int typ;                            //Typ   0:Default   1:Pilot  2:Controller 3:NavData 4:Weather

signals:
    void entered(int t);
    void left(int t);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
};

class OnScreenWidget : public QWidget
{
Q_OBJECT
public:
    OnScreenWidget(QWidget *parent = 0);
    int typ;                            //Typ   0:Default   1:Pilot  2:Controller 3:NavData 4:Weather

signals:
    void entered(int t);
    void left(int t);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
};

class MapScreen : public QWidget
{
Q_OBJECT

public:
    MapScreen(QWidget *parent = 0);
    GLWidget *glWidget;
    static MapScreen* getInstance(bool);


protected:
    void resizeEvent(QResizeEvent *event);



signals:
    void toggleRoutes();
    void toggleSectors(bool);
    void toggleRouteWaypoints();
    void toggleInactiveAirports();

public slots:

    void LabelEntered(int typ);
    void LabelLeft(int typ);

    void WidgetEntered(int typ);
    void WidgetLeft(int typ);

    //Pilots
    void P_toggleRoutesClicked();
    void P_togglePilotLabelsClicked();

    //Controller
    void C_toggleCTRClicked();
    void C_toggleAPPClicked();
    void C_toggleTWRClicked();
    void C_toggleGNDClicked();

    //NavData
    void N_toggleSectorClicked();
    void toggleSectorChanged( bool );
    void N_toggleRouteFixClicked();
    void N_toggleInactiveClicked();

    //Weather
    void W_toggleWindClicked();
    void W_minusClicked();
    void W_plusClicked();
    void W_sliderChanged();
    void W_toggleCloudsClicked();


private:
    void createPilotWidget();
    void createControllerWidget();
    void createNavDataWidget();
    void createWindWidget();

    OnScreenLabel *L_Controller, *L_Pilots, *L_NavData, *L_Weather;
    OnScreenWidget *W_Controller, *W_Pilots, *W_NavData, *W_Weather;


    QHBoxLayout C_layout, P_layout, N_layout, N_layout1, W_layout, W_layout1;
    QVBoxLayout N_Vlayout, W_Vlayout;

    //for PilotTab
    QPushButton *P_toggleRoutes, *P_togglePilotLabels;

    //for ControllerTab
    QPushButton *C_toggleCTR, *C_toggleAPP, *C_toggleTWR, *C_toggleGND;


    //for NavDataTab
    QPushButton  *N_sectorsAll, *N_InactiveAirports, *N_RouteWaypoints;
    QPushButton  *N_FIX, *N_VOR, *N_NDB;

    //for WeatherTab
    QPushButton *W_toggleWind, *W_plus, *W_minus, *W_toggleClouds;
    QSlider *W_slider;
    QLabel *W_windAlt;

    int xDistance;
};

#endif // MAPSCREEN_H
