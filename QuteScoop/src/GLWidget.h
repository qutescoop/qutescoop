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

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QGLFormat format, QWidget *parent = 0);
    ~GLWidget();

//    QSize minimumSizeHint() const { return QSize(150, 200); }
//    QSize sizeHint() const { return QSize(600, 700); }

    bool plotFlightPlannedRoute;
    QPair<double, double> currentPosition();
    ClientSelectionWidget *clientSelection;

public slots:
    void initializeGL();
    void newWhazzupData(bool isNew); // could be solved more elegantly, but it gets called for
    // updating the statusbar as well - we do not want a full GL update here sometimes
    void setMapPosition(double lat, double lon, double newZoom, bool updateGL = true);
    void scrollBy(int moveByX, int moveByY);
    void rightClick(const QPoint& pos);
    void zoomIn(double factor);
    void zoomTo(double zoom);

    // Return a list of all clients at given lat/lon, within radius miles
    QList<MapObject*> objectsAt(int x, int y, double radius = 0) const;

    void rememberPosition(int nr);
    void restorePosition(int nr);

    void displayAllSectors(bool value);
    void showInactiveAirports(bool value);

    void createPilotsList();
    void createAirportsList();
    void prepareDisplayLists();

    void shutDownAnimation();

signals:
    void mapClicked(int x, int y, QPoint absolutePos);
    void newPosition();
    void hasGuiMessage(QString, GuiMessage::GuiMessageType = GuiMessage::Temporary,
                       QString = QString(), int = 0, int = 0);

protected:
    void paintGL();
    void resizeGL(int width, int height);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    bool event(QEvent *event);

private:
    void createObjects();

    void resetZoom();
    void handleRotation(QMouseEvent *event);
    bool mouse2latlon(int x, int y, double& lat, double& lon) const;
    bool pointIsVisible(double lat, double lon, int *px = 0, int *py = 0) const;

    void drawSelectionRectangle();
    void drawCoordinateAxii();

    const QPair<double, double> sunZenith(const QDateTime &dt);

    void renderLabels();
    void renderLabels(const QList<MapObject*>& objects, const QFont& font, double zoomTreshold, QColor color);

    class FontRectangle {
    public:
        FontRectangle(QRectF rectangle, MapObject *mapObject): _rect(rectangle), _object(mapObject) {}
        const QRectF& rect() const { return _rect; }
        MapObject* object() const { return _object; }
    private:
        QRectF _rect;
        MapObject *_object;
    };
    bool shouldDrawLabel(const FontRectangle& rect);
    QList<FontRectangle> fontRectangles, allFontRectangles;

    QPoint lastPos, mouseDownPos;
    bool mapIsMoving, isRectangleSelecting;
    double xRot, yRot, zRot, zoom, aspectRatio;

	GLUquadricObj *earthQuad;
	GLuint earthTex;
	GLuint earthList, coastlinesList, countriesList, gridlinesList;
	GLuint pilotsList, airportsList, airportsInactiveList;
	GLuint fixesList;
	GLuint sectorPolygonsList, sectorPolygonBorderLinesList, airportControllersList, appBorderLinesList,
		congestionsList;
	bool allSectorsDisplayed;

	QList<Controller*> sectorsToDraw;

    double pilotLabelZoomTreshold, airportLabelZoomTreshold, inactiveAirportLabelZoomTreshold,
        inactiveAirportDotZoomTreshold, controllerLabelZoomTreshold, fixZoomTreshold;

    qint64 shutDownAnim_t;
};

#endif /*GLWIDGET_H_*/