#ifndef MAPSCREEN_H
#define MAPSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

#include "GLWidget.h"
#include "Settings.h"



class MapScreen : public QWidget
{
Q_OBJECT
public:
    MapScreen(QWidget *parent = 0);
    GLWidget *glWidget;

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent (QMouseEvent * event );

signals:
    void toggleRoutes();
    void toggleSectors();
    void toggleRouteWaypoints();
    void toggleInactiveAirports();

public slots:
    void glMouseMoved(QMouseEvent *event);

    //Pilots
    void P_toggleRoutesClicked();

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

    QLabel *L_Controller, *L_Pilots, *L_NavData;
    QWidget *W_Controller, *W_Pilots, *W_NavData;
    QHBoxLayout C_layout, P_layout, N_layout;

    //For PilotTab
    QPushButton *P_toggleRoutes;

    //For ControllerTab
    QPushButton *C_toggleCTR, *C_toggleAPP, *C_toggleTWR, *C_toggleGND;


    //For NavDataTab
    QPushButton  *N_sectorsAll, *N_InactiveAirports, *N_RouteWaypoints;


    int xDistance;
    bool P_visible;
    bool C_visible;
    bool N_visible;

};

#endif // MAPSCREEN_H
