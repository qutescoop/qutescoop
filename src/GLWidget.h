#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include "ClientSelectionWidget.h"
#include "Controller.h"
#include "MapObject.h"
#include "Sector.h"
#include "src/helpers.h"

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

class GLWidget
    : public QGLWidget {
    Q_OBJECT
    public:
        GLWidget(QGLFormat format, QWidget* parent = 0);
        ~GLWidget();

        DoublePair currentLatLon() const;
        ClientSelectionWidget* clientSelection;
        void invalidatePilots();
        void invalidateAirports();
        void invalidateControllers();
        void setStaticSectors(QList<Sector*>);
        void savePosition();

        struct FontRectangle {
            QRectF rect = QRectF();
            MapObject* object = 0;
        };
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
        void restorePosition(int nr, bool isSilent = false);
        void configureHoverDebounce();
        void configureUpdateTimer();
    signals:
        void mapClicked(int x, int y, QPoint absolutePos);
    protected:
        virtual void paintGL() override;
        virtual void resizeGL(int width, int height) override;
        QList<MapObject*> objectsAt(int x, int y, double radius = 0) const;

        void mouseDoubleClickEvent(QMouseEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;
        void leaveEvent(QEvent* event) override;
        bool event(QEvent* event) override;
    private:
        void resetZoom();
        void handleRotation(QMouseEvent* event);
        bool local2latLon(int x, int y, double &lat, double &lon) const;
        bool latLon2local(double lat, double lon, int* px = 0, int* py = 0) const;
        QPair<DoublePair, DoublePair> shownLatLonExtent() const;

        void drawSelectionRectangle();
        void drawCoordinateAxii() const;
        void drawCoordinateAxiiCurrentMatrix() const;

        const QPair<double, double> sunZenith(const QDateTime &dt) const;

        void createPilotsList();
        void createAirportsList();
        void createControllerLists();
        void createStaticLists();
        void createStaticSectorLists();
        void createHoveredControllersLists(const QSet<Controller*>& controllers);

        void parseTexture();
        void createLights();

        QList<Sector*> m_staticSectors;
        QPoint _lastPos, _mouseDownPos;
        bool m_isMapMoving, m_isMapZooming, m_isMapRectSelecting, _lightsGenerated;
        bool m_isPilotsListDirty = true, m_isAirportsListDirty = true, m_isControllerListsDirty = true, m_isStaticSectorListsDirty = true,
            m_isAirportsMapObjectsDirty = true, m_isControllerMapObjectsDirty = true, m_isPilotMapObjectsDirty = true, m_isUsedWaypointMapObjectsDirty = true;
        GLUquadricObj* _earthQuad;
        GLuint _earthTex, _fadeOutTex,
            _earthList, _coastlinesList, _countriesList, _gridlinesList,
            _pilotsList, _activeAirportsList, _inactiveAirportsList,
            _usedWaypointsList, _plannedRouteList,
            _sectorPolygonsList, _sectorPolygonBorderLinesList, _congestionsList,
            _staticSectorPolygonsList, _staticSectorPolygonBorderLinesList,
            _hoveredSectorPolygonsList, _hoveredSectorPolygonBorderLinesList;
        QSet<Controller*> m_hoveredControllers;
        double _pilotLabelZoomTreshold, _activeAirportLabelZoomTreshold, _inactiveAirportLabelZoomTreshold,
            _controllerLabelZoomTreshold, _usedWaypointsLabelZoomThreshold,
            _xRot, _yRot, _zRot, _zoom, _aspectRatio;
        QTimer* m_updateTimer;
        QTimer* m_hoverDebounceTimer;
        QList< QPair<double, double> > m_friendPositions;
        QList<MapObject*> m_hoveredObjects, m_newHoveredObjects;
        QMultiHash<QPair<int, int>, MapObject*> m_inactiveAirportMapObjectsByLatLng;
        QList<MapObject*> m_activeAirportMapObjects, m_controllerMapObjects, m_pilotMapObjects,
            m_usedWaypointMapObjects;

        void renderLabels();
        void renderLabels(
            const QList<MapObject*>& objects,
            const double zoomTreshold,
            const QFont& font,
            const QColor color,
            const QFont& secondaryFont = QFont(),
            const QColor secondaryColor = QColor(),
            const bool isFastBail = false,
            const unsigned short tryNOtherPositions = 3,
            const bool isHoverRenderPass = false
        );
        bool shouldDrawLabel(const QRectF &rect);
        struct RenderLabelsCommand {
            QList<MapObject*> objects;
            double zoomTreshold;
            QFont font;
            QColor color;
            QFont secondaryFont;
            QColor secondaryColor;
            bool isFastBail;
            unsigned short tryNOtherPositions;
        };
        QList<RenderLabelsCommand> m_prioritizedLabels;
        QList<FontRectangle>fontRectanglesPrioritized() const;
        QSet<FontRectangle> m_fontRectangles;

        void drawTestTextures();
        void drawBillboardScreenSize(GLfloat lat, GLfloat lon, const QSize& size);
        void drawBillboardWorldSize(GLfloat lat, GLfloat lon, const QSizeF& size);
        inline void drawBillboard(GLfloat lat, GLfloat lon, GLfloat halfWidth, GLfloat halfHeight, GLfloat alpha = 1.);
        void orthoMatrix(GLfloat lat, GLfloat lon);

    private slots:
        void updateHoverState();
};

inline bool operator==(const GLWidget::FontRectangle &e1, const GLWidget::FontRectangle &e2) {
    return e1.object == e2.object
        && e1.rect == e2.rect;
}

inline uint qHash(const GLWidget::FontRectangle &key, uint seed) {
    return qHash(key.object, seed);
}

#endif
