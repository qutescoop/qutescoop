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
        static MapScreen* instance(bool createIfNotExists);

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

        OnScreenLabel *_oslController, *_oslPilots, *_oslNavData, *_oslWeather;
        OnScreenWidget *_oswController, *_oswPilots, *_oswNavData, *_oswWeather;


        QHBoxLayout _hblC, _hblP, _hblN, _hblN1, _hblW, _hblW1;
        QVBoxLayout _vblN, _vblW;

        //for PilotTab
        QPushButton *_pbRoutes, *_pbPilotLabels;

        //for ControllerTab
        QPushButton *_pbCtr, *_pbApp, *_pbTwr, *_pbGnd;

        //for NavDataTab
        QPushButton  *_pbSectorsAll, *_pbInactiveAirports, *_pbRouteWaypoints;
        //QPushButton  *pbFixes, *_pbVors, *_pbNdbs;

        //for WeatherTab
        QPushButton *_pbWind, *_pbWindAltInc, *_pbWindAltDec, *_pbClouds;
        QSlider *_slider;
        QLabel *_lblWindAlt;

        int _xDistance;
};

#endif // MAPSCREEN_H
