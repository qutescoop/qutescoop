#ifndef MAPSCREEN_H
#define MAPSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

#include "GLWidget.h"
#include "Settings.h"


class OnScreenLabel : public QLabel
{
Q_OBJECT
public:
    OnScreenLabel(QWidget *parent = 0);
    int typ;                            //Typ   0:Default   1:Pilot  2:Controller 3:NavData

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
    int typ;                            //Typ   0:Default   1:Pilot  2:Controller 3:NavData

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


protected:
    void resizeEvent(QResizeEvent *event);



signals:
    void toggleRoutes();
    void toggleSectors();
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
    void N_toggleRouteFixClicked();
    void N_toggleInactiveClicked();


private:
    void createPilotWidget();
    void createControllerWidget();
    void createNavDataWidget();

    OnScreenLabel *L_Controller, *L_Pilots, *L_NavData;
    OnScreenWidget *W_Controller, *W_Pilots, *W_NavData;


    QHBoxLayout C_layout, P_layout, N_layout, N_layout1;
    QVBoxLayout N_Vlayout;

    //For PilotTab
    QPushButton *P_toggleRoutes, *P_togglePilotLabels;

    //For ControllerTab
    QPushButton *C_toggleCTR, *C_toggleAPP, *C_toggleTWR, *C_toggleGND;


    //For NavDataTab
    QPushButton  *N_sectorsAll, *N_InactiveAirports, *N_RouteWaypoints;
    QPushButton  *N_FIX, *N_VOR, *N_NDB;

    int xDistance;
};

#endif // MAPSCREEN_H
