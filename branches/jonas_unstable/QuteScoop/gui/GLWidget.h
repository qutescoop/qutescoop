/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <QGLFormat>
#include <QGLWidget>
#include <QHash>
#include <QList>

#include "MapObject.h"
#include "Fir.h"
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

public slots:
	void newWhazzupData();
	void setMapPosition(double lat, double lon, double newZoom);
	void rightClick(const QPoint& pos);
	void zoomIn()  { return zoomIn(1);  }
	void zoomOut() { return zoomOut(1); }
	void zoomIn(int factor);
	void zoomOut(int factor);

	void rememberPosition();
	void restorePosition();

	void displayAllFirs(bool value);
	void showInactiveAirports(bool value);

	/**
	 * Return a list of all clients at given lat/lon, within radius miles
	 */
	QList<MapObject*> objectsAt(int x, int y, double radius = 0) const;

signals:
	void mapClicked(int x, int y, QPoint absolutePos);

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
	void createPilotsList();
	GLuint airportsList;
	GLuint airportsInactiveList;
	void createAirportsList();

	GLuint fixesList;
	void createFixesList();

	GLuint firPolygonsList;
	GLuint airportControllersList;
	GLuint firPolygonBorderLinesList;
	GLuint appBorderLinesList;
	void prepareDisplayLists();

	QList<Controller*> firsToDraw;

	double pilotLabelZoomTreshold;
	double airportLabelZoomTreshold;
	double inactiveAirportLabelZoomTreshold;
	double controllerLabelZoomTreshold;

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
	bool shouldDrawLabel(const FontRectangle& rect);

	void renderLabels();
	void renderLabels(const QList<MapObject*>& objects, const QFont& font, double zoomTreshold, QColor color);

	bool allFirsDisplayed;
};

#endif /*GLWIDGET_H_*/
