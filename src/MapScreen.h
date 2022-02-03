#ifndef MAPSCREEN_H
#define MAPSCREEN_H

#include "_pch.h"

#include "GLWidget.h"
#include "Settings.h"

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

    private:
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
