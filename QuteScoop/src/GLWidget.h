/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <QGLFormat>
#include <QGLWidget>
#include <QHash>
#include <QList>

#include "MapObject.h"
#include "Sector.h"
#include "Airport.h"
#include "WhazzupData.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QGLFormat format, QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    bool plotFlightPlannedRoute;
    QPair<double, double> currentPosition();

public slots:
    void newWhazzupData(bool isNew = true); // could be solved more elegantly, but it gets called for
                            // updating the statusbar as well - we do not want a full GL update here sometimes
	void setMapPosition(double lat, double lon, double newZoom);
    void scrollBy(int moveByX, int moveByY);
	void rightClick(const QPoint& pos);
    void zoomIn()  { return zoomIn(0.6);  }
    void zoomOut() { return zoomOut(0.6); }
    void zoomIn(double factor);
    void zoomOut(double factor);

    void rememberPosition(int nr);
    void restorePosition(int nr);

        void displayAllSectors(bool value);
	void showInactiveAirports(bool value);

    void createPilotsList();

	/**
	 * Return a list of all clients at given lat/lon, within radius miles
	 */
	QList<MapObject*> objectsAt(int x, int y, double radius = 0) const;

signals:
	void mapClicked(int x, int y, QPoint absolutePos);
    void newPosition();

protected:
	void initializeGL();
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
	void normalizeAngle(double *angle) const;
	void resetZoom();
	void handleRotation(QMouseEvent *event);

	void determineActiveSectors();
	void updateAirports();

	/**
	 * Convert mouse coordinatex (x/y) into lat/lon on the globe.
	 * Returns false if the mouse is not on the globe (but pointing on space),
	 * true otherwise
	 */
	bool mouse2latlon(int x, int y, double& lat, double& lon) const;

	GLuint orbList;
	void createOrbList();
	GLuint coastlineList;
	void createCoastlineList();
	GLuint gridlinesList;
	void createGridlinesList();
	GLuint countriesList;
	void createCountriesList();
	GLuint pilotsList;
	GLuint airportsList;
	GLuint airportsInactiveList;
	void createAirportsList();

	GLuint fixesList;
	void createFixesList();

        GLuint sectorPolygonsList;
	GLuint airportControllersList;
        GLuint sectorPolygonBorderLinesList;
	GLuint appBorderLinesList;
	GLuint congestionsList;
	void prepareDisplayLists();

        QList<Controller*> sectorsToDraw;

	double pilotLabelZoomTreshold;
	double airportLabelZoomTreshold;
	double inactiveAirportLabelZoomTreshold;
	double controllerLabelZoomTreshold;

    double inactiveAirportDotZoomTreshold;

	double fixZoomTreshold;

	double xRot;
	double yRot;
	double zRot;
	double zoom;
	double aspectRatio;
	QPoint lastPos;
	QPoint mouseDownPos;

	bool pointIsVisible(double lat, double lon, int *px = 0, int *py = 0) const;

	class FontRectangle {
        public:
            FontRectangle(QRectF rectangle, MapObject *mapObject): _rect(rectangle), _object(mapObject) {}
            const QRectF& rect() const { return _rect; }
            MapObject* object() const { return _object; }
        private:
            QRectF _rect;
            MapObject *_object;
	};
    QList<FontRectangle> fontRectangles;
    QList<FontRectangle> allFontRectangles;
    bool shouldDrawLabel(const FontRectangle& rect);

	void renderLabels();
	void renderLabels(const QList<MapObject*>& objects, const QFont& font, double zoomTreshold, QColor color);

    bool allSectorsDisplayed;
    bool mapIsMoving;
};

#endif /*GLWIDGET_H_*/
