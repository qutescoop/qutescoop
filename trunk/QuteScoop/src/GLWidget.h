/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include "_pch.h"

#include "MapObject.h"
#include "Sector.h"
#include "Airport.h"
#include "WhazzupData.h"
#include "GuiMessage.h"
#include "ClientSelectionWidget.h"
#include "Airac.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QGLFormat format, QWidget *parent = 0);
    ~GLWidget();

    QPair<double, double> currentPosition() const;
    ClientSelectionWidget *clientSelection;

public slots:
    virtual void initializeGL();
    void newWhazzupData(bool isNew); // could be solved more elegantly, but it gets called for
    // updating the statusbar as well - we do not want a full GL update here sometimes
    void setMapPosition(double lat, double lon, double newZoom, bool updateGL = true);
    void scrollBy(int moveByX, int moveByY);
    void rightClick(const QPoint& pos);
    void zoomIn(double factor);
    void zoomTo(double zoom);

    // Return a list of all clients at given lat/lon, within radius miles, distance-ordered
    QList<MapObject*> objectsAt(int x, int y, double radius = 0) const;

    void rememberPosition(int nr);
    void restorePosition(int nr);

    void displayAllSectors(bool value);
    void showInactiveAirports(bool value);

    void createPilotsList();
    void createAirportsList();
    void createControllersLists();
    void createStaticLists();


signals:
    void mapClicked(int x, int y, QPoint absolutePos);
    void newPosition();

protected:
    virtual void paintGL();
    virtual void resizeGL(int width, int height);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    bool event(QEvent *event);

private:

    void resetZoom();
    void handleRotation(QMouseEvent *event);
    bool mouseOnGlobe(int x, int y) const;
    bool mouse2latlon(int x, int y, double &lat, double &lon) const;
    bool pointIsVisible(double lat, double lon, int *px = 0, int *py = 0) const;

    void drawSelectionRectangle() const;
    void drawCoordinateAxii() const;

    const QPair<double, double> sunZenith(const QDateTime &dt) const;

    void renderLabels();
    void renderLabels(const QList<MapObject*>& objects, const QFont& font, double zoomTreshold, QColor color);




    class FontRectangle {
    public:
        FontRectangle(QRectF rectangle, MapObject *mapObject): rect(rectangle), object(mapObject) {}
        QRectF rect;
        MapObject *object;
    };
    bool shouldDrawLabel(const QRectF &rect);
    QSet<FontRectangle*> fontRectangles, allFontRectangles;

    QPoint lastPos, mouseDownPos;
    bool mapIsMoving, mapIsZooming, mapIsRectangleSelecting;
    double xRot, yRot, zRot, zoom, aspectRatio;

        GLUquadricObj *earthQuad;
        GLuint earthTex;
        GLuint earthList, coastlinesList, countriesList, gridlinesList;
        GLuint pilotsList, activeAirportsList, inactiveAirportsList;
        GLuint FixesList, usedWaypointsList, plannedRouteList;
        GLuint sectorPolygonsList, sectorPolygonBorderLinesList, appBorderLinesList, congestionsList;
        //GLuint airportControllersList,
        bool allSectorsDisplayed;

        QSet<Controller*> sectorsToDraw;

    double pilotLabelZoomTreshold, activeAirportLabelZoomTreshold, inactiveAirportLabelZoomTreshold,
        controllerLabelZoomTreshold, allWaypointsLabelZoomTreshold, usedWaypointsLabelZoomThreshold;
};

#endif /*GLWIDGET_H_*/
