#ifndef MAPSCREEN_H
#define MAPSCREEN_H

#include "_pch.h"

#include "GLWidget.h"
#include "Settings.h"


class OnScreenLabel : public QLabel {
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

class OnScreenWidget : public QWidget {
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

class MapScreen : public QWidget {
        Q_OBJECT

    public:
        MapScreen(QWidget *parent = 0);
        GLWidget *glWidget;
        static MapScreen* instance(bool createIfNoInstance = true);
    protected:
        void resizeEvent(QResizeEvent *event);
    signals:
        void toggleRoutes();
        void toggleSectors(bool);
        void toggleRouteWaypoints();
        void toggleInactiveAirports();
    public slots:
        void on_label_entered(int typ);
        void on_label_left(int typ);

        void on_widget_entered(int typ);
        void on_widget_left(int typ);

        //Pilots
        void on_pbRoutes_clicked();
        void on_pbPilotLabels_clicked();

        //Controller
        void on_pbCtr_clicked();
        void on_pbApp_clicked();
        void on_pbTwr_clicked();
        void on_pbGnd_clicked();

        //NavData
        void on_pbSectorsAll_clicked();
        void on_sectorsAll_changed(bool);
        void on_pbRouteWaypoints_clicked();
        void on_pbInactiveAirports_clicked();

        //Weather
        void on_pbWind_clicked();
        void on_pbWindAltDec_clicked();
        void on_pbWindAltInc_clicked();
        void on_slider_clicked();
        void on_pbClouds_clicked();
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
