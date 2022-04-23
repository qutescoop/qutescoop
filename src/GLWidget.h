/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include "_pch.h"
#include <QPoint>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include "MapObject.h"
#include "Sector.h"
#include "Airport.h"
#include "WhazzupData.h"
#include "GuiMessage.h"
#include "ClientSelectionWidget.h"
#include "Airac.h"
#include "SondeData.h"


class GLWidget : public QOpenGLWidget {
        Q_OBJECT
    public:
        GLWidget(QWidget *parent = 0);
        ~GLWidget();

        QPair<double, double> currentPosition() const;
        ClientSelectionWidget *clientSelection;
        void savePosition();
        bool isInGlContext = false;
    public slots:
        virtual void initializeGL();
        void newWhazzupData(bool isNew); // could be solved more elegantly, but it gets called for
        // updating the statusbar as well - we do not want a full GL update here sometimes
        void setMapPosition(double lat, double lon, double newZoom, bool updateGL = true);
        void scrollBy(int moveByX, int moveByY);
        void rightClick(const QPoint& pos);
        void zoomIn(double factor);
        void zoomTo(double _zoom);

        void rememberPosition(int nr);
        void restorePosition(int nr);

        void displayAllSectors(bool value);
        void showInactiveAirports(bool value);

        void createPilotsList();
        void createAirportsList();
        void createControllersLists();
        void createStaticLists();
        void createStaticSectorLists(QList<Sector*> sectors);
        void createHoveredControllersLists(QSet<Controller*> controllers);

        void useClouds();

        void destroyFriendHightlighter();
        void renderStaticSectors(bool value) { _renderStaticSectors = value; }
    signals:
        void mapClicked(int x, int y, QPoint absolutePos);
        void newPosition();
    protected:
        virtual void paintGL();
        virtual void resizeGL(int width, int height);

        // Return a list of all clients at given lat/lon, within radius miles, distance-ordered
        QList<MapObject *> objectsAt(int x, int y, double radius = 0) const;

        void mouseDoubleClickEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent* event);
        bool event(QEvent *event);
        void qglColor(const QColor&);
        void renderText(double x, double y, double z, const QString &str, const QFont & font = QFont());
        void renderText(int x, int y, const QString &str, const QFont & font = QFont());
  private:
        void resetZoom();
        void handleRotation(QMouseEvent *event);
        bool isOnGlobe(int x, int y) const;
        bool mouse2latlon(int x, int y, double &lat, double &lon) const;
        bool isPointVisible(double lat, double lon, int *px = 0, int *py = 0) const;

        void drawSelectionRectangle();
        void drawCoordinateAxii() const;

        const QPair<double, double> sunZenith(const QDateTime &dt) const;

        void renderLabels();
        void renderLabels(const QList<MapObject*>& objects, const QFont& font,
                          const double zoomTreshold, QColor color, QColor bgColor = QColor());
        void renderLabelsSimple(const QList<MapObject*>& objects, const QFont& font,
                                const double zoomTreshold, QColor color, QColor bgColor = QColor());
        void renderLabelsComplex(const QList<MapObject*>& objects, const QFont& font,
                                 const double zoomTreshold, QColor color, QColor bgColor = QColor());

        void parseEarthClouds();
        void createLights();

        void createFriendHighlighter();

        class FontRectangle {
            public:
                FontRectangle(QRectF rectangle, MapObject *mapObject):
                    rect(rectangle), object(mapObject) {}
                QRectF rect;
                MapObject *object;
        };
        bool shouldDrawLabel(const QRectF &rect);

        QSet<FontRectangle*> _fontRectangles, _allFontRectangles;
        QPoint _lastPos, _mouseDownPos;
        bool _mapMoving, _mapZooming, _mapRectSelecting, _renderStaticSectors,
        _lightsGenerated, _allSectorsDisplayed;
        QImage _completedEarthIm;
        GLUquadricObj *_earthQuad;
        GLuint _earthTex, _cloudTex,
        _earthList, _coastlinesList, _countriesList, _gridlinesList,
        _pilotsList, _activeAirportsList, _inactiveAirportsList,
        _fixesList, _usedWaypointsList, _plannedRouteList,
        _sectorPolygonsList, _sectorPolygonBorderLinesList, _congestionsList,
        _staticSectorPolygonsList, _staticSectorPolygonBorderLinesList,
        _hoveredSectorPolygonsList, _hoveredSectorPolygonBorderLinesList;
        QSet<Controller*> _sectorsToDraw, _hoveredControllers;
        double _sondeLabelZoomTreshold, _pilotLabelZoomTreshold,
                _activeAirportLabelZoomTreshold, _inactiveAirportLabelZoomTreshold,
        _controllerLabelZoomTreshold, _allWaypointsLabelZoomTreshold, _usedWaypointsLabelZoomThreshold,
        _xRot, _yRot, _zRot, _zoom, _aspectRatio;
        QTimer *_highlighter;
        QList< QPair<double , double> > _friends;
};

#endif /*GLWIDGET_H_*/
