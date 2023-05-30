/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include "ClientSelectionWidget.h"
#include "Controller.h"
#include "MapObject.h"
#include "Sector.h"

#include <qglobal.h>
#ifdef Q_OS_WIN
    #include <windows.h>
#endif
#ifdef __APPLE__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

#include <QPoint>

class GLWidget : public QGLWidget {
        Q_OBJECT
    public:
        GLWidget(QGLFormat format, QWidget *parent = 0);
        ~GLWidget();

        QPair<double, double> currentPosition() const;
        ClientSelectionWidget *clientSelection;
        void invalidatePilots() { m_isPilotsDirty = true; update(); }
        void invalidateAirports() { m_isAirportsDirty = true; update(); }
        void invalidateControllers() { m_isControllersDirty = true; update(); }
        void setStaticSectors(QList<Sector*>);
        void savePosition();
    public slots:
        virtual void initializeGL() override;
        void newWhazzupData(bool isNew); // could be solved more elegantly, but it gets called for
        // updating the statusbar as well - we do not want a full GL update here sometimes
        void setMapPosition(double lat, double lon, double newZoom);
        void scrollBy(int moveByX, int moveByY);
        void rightClick(const QPoint& pos);
        void zoomIn(double factor);
        void zoomTo(double _zoom);

        void rememberPosition(int nr);
        void restorePosition(int nr);

        void showInactiveAirports(bool value);

        void destroyFriendHighlighter();
    signals:
        void mapClicked(int x, int y, QPoint absolutePos);
    protected:
        virtual void paintGL() override;
        virtual void resizeGL(int width, int height) override;

        // Return a list of all clients at given lat/lon, within radius miles, distance-ordered
        QList<MapObject *> objectsAt(int x, int y, double radius = 0) const;

        void mouseDoubleClickEvent(QMouseEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void wheelEvent(QWheelEvent* event) override;
        bool event(QEvent *event) override;
    private:
        void resetZoom();
        void handleRotation(QMouseEvent *event);
        bool isOnGlobe(int x, int y) const;
        bool mouse2latlon(int x, int y, double &lat, double &lon) const;
        bool isPointVisible(double lat, double lon, int *px = 0, int *py = 0) const;

        void drawSelectionRectangle();
        void drawCoordinateAxii() const;

        const QPair<double, double> sunZenith(const QDateTime &dt) const;

        void createPilotsList();
        void createAirportsList();
        void createControllerLists();
        void createStaticLists();
        void createStaticSectorLists();
        void createHoveredControllersLists(QSet<Controller*> controllers);

        void renderLabels();
        void renderLabels(const QList<MapObject*>& objects, const QFont& font,
                          const double zoomTreshold, QColor color, QColor bgColor = QColor());
        void renderLabelsSimple(const QList<MapObject*>& objects, const QFont& font,
                                const double zoomTreshold, QColor color, QColor bgColor = QColor());
        void renderLabelsComplex(const QList<MapObject*>& objects, const QFont& font,
                                 const double zoomTreshold, QColor color, QColor bgColor = QColor());

        void parseTexture();
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

        QList<Sector*> m_staticSectors;
        QSet<FontRectangle*> _fontRectangles, _allFontRectangles;
        QPoint _lastPos, _mouseDownPos;
        bool _mapMoving, _mapZooming, _mapRectSelecting, _lightsGenerated;
        bool m_isPilotsDirty = true, m_isAirportsDirty = true, m_isControllersDirty = true, m_isStaticSectorsDirty = true;
        GLUquadricObj *_earthQuad;
        GLuint _earthTex,
        _earthList, _coastlinesList, _countriesList, _gridlinesList,
        _pilotsList, _activeAirportsList, _inactiveAirportsList,
        _fixesList, _usedWaypointsList, _plannedRouteList,
        _sectorPolygonsList, _sectorPolygonBorderLinesList, _congestionsList,
        _staticSectorPolygonsList, _staticSectorPolygonBorderLinesList,
        _hoveredSectorPolygonsList, _hoveredSectorPolygonBorderLinesList;
        QSet<Controller*> _hoveredControllers;
        double _pilotLabelZoomTreshold, _activeAirportLabelZoomTreshold, _inactiveAirportLabelZoomTreshold,
            _controllerLabelZoomTreshold, _allWaypointsLabelZoomTreshold, _usedWaypointsLabelZoomThreshold,
            _xRot, _yRot, _zRot, _zoom, _aspectRatio;
        QTimer *_highlighter;
        QList< QPair<double , double> > m_friendPositions;
};

#endif /*GLWIDGET_H_*/
