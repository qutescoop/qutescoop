/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
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

#include <QtGui>
#include <QtOpenGL>
#include <QFontMetricsF>
#include <QDebug>
#include <math.h>

#include "helpers.h"
#include "GLWidget.h"
#include "LineReader.h"

#include "Pilot.h"
#include "Controller.h"
#include "Whazzup.h"
#include "NavData.h"
#include "Settings.h"
#include "Waypoint.h"
#include "PlanFlightDialog.h"
#include "ListClientsDialog.h"
#include "Window.h"

GLWidget::GLWidget(QGLFormat fmt, QWidget *parent) :
    QGLWidget(fmt, parent) {
    xRot = 0;
    yRot = 0;
    zRot = 0;
    zoom = 2;
    aspectRatio = 1;
    emit newPosition();

    pilotsList = 0;
    airportsList = 0;

    setMouseTracking(true);

    pilotLabelZoomTreshold = 0.5;
    airportLabelZoomTreshold = 1;
    inactiveAirportDotZoomTreshold = 0.15;
    inactiveAirportLabelZoomTreshold = 0.08;
    controllerLabelZoomTreshold = 3;
    fixZoomTreshold = 0.05;

    sectorPolygonsList = 0;
    airportControllersList = 0;
    sectorPolygonBorderLinesList = 0;
    appBorderLinesList = 0;
    congestionsList = 0;

    plotFlightPlannedRoute = false;

    allSectorsDisplayed = false;
    mapIsMoving = false;
}

GLWidget::~GLWidget() {
    makeCurrent();
    glDeleteLists(orbList, 1);
    glDeleteLists(coastlineList, 1);
    glDeleteLists(gridlinesList, 1);
    glDeleteLists(countriesList, 1);
    glDeleteLists(pilotsList, 1);

    QList<Airport*> airportList = NavData::getInstance()->airports().values();
    for (int i = 0; i < airportList.size(); i++) {
        delete airportList[i];
    }

    QList<Sector*> sectorList = NavData::getInstance()->sectors().values();
    for (int i = 0; i < sectorList.size(); i++) {
        delete sectorList[i];
    }
}

QSize GLWidget::minimumSizeHint() const {
    return QSize(150, 200);
}

QSize GLWidget::sizeHint() const {
    return QSize(600, 700);
}

void GLWidget::setMapPosition(double lat, double lon, double newZoom) {
    xRot = 360 - lat;
    yRot = 360 - lon;
    normalizeAngle(&xRot);
    normalizeAngle(&yRot);
    zoom = newZoom;
    resetZoom();
    updateGL();
    emit newPosition();
}

QPair<double, double> GLWidget::currentPosition() {
    double lat = 360 - xRot;
    double lon = 360 - yRot;
    while (lat > 180) lat -= 360;
    while (lat < -180) lat += 360;
    if (lat > 90) lat = 180 - lat;
    if (lat < -90) lat = -180 + lat;
    while (lon > 180) lon -= 360;
    while (lon < -180) lon += 360;

    return QPair<double, double>(lat, lon);
}

void GLWidget::prepareDisplayLists() {

    // FIR polygons
    if(sectorPolygonsList == 0)
        sectorPolygonsList = glGenLists(1);

    // make sure all the lists are there to avoid nested glNewList calls
    for(int i = 0; i < sectorsToDraw.size(); i++) {
        Sector *f = sectorsToDraw[i]->sector;
        if(f == 0) continue;
        f->getPolygon();
    }

    // create a list of lists
    glNewList(sectorPolygonsList, GL_COMPILE);
        for(int i = 0; i < sectorsToDraw.size(); i++) {
            Sector *f = sectorsToDraw[i]->sector;
            if(f == 0) continue;
            glCallList(f->getPolygon());
        }
    glEndList();

    // Controllers at airports
    if(airportControllersList == 0)
        airportControllersList = glGenLists(1);

    // first, make sure all lists are there
    QList<Airport*> airportList = NavData::getInstance()->airports().values();
    for(int i = 0; i < airportList.size(); i++) {
        if(airportList[i] == 0) continue;
        if(!airportList[i]->getApproaches().isEmpty())
            airportList[i]->getAppDisplayList();
        if(!airportList[i]->getTowers().isEmpty())
            airportList[i]->getTwrDisplayList();
        if(!airportList[i]->getGrounds().isEmpty())
            airportList[i]->getGndDisplayList();
        if(!airportList[i]->getDeliveries().isEmpty())
            airportList[i]->getDelDisplayList();
    }

    // create a list of lists
    glNewList(airportControllersList, GL_COMPILE);
        // Approaches
        for(int i = 0; i < airportList.size(); i++) {
            if(airportList[i] == 0) continue;
            if(!airportList[i]->getApproaches().isEmpty())
                glCallList(airportList[i]->getAppDisplayList());
        }

        // Towers
        for(int i = 0; i < airportList.size(); i++) {
            if(airportList[i] == 0) continue;
            if(!airportList[i]->getTowers().isEmpty())
                glCallList(airportList[i]->getTwrDisplayList());
        }

        // Grounds
        for(int i = 0; i < airportList.size(); i++) {
            if(airportList[i] == 0) continue;
            if(!airportList[i]->getGrounds().isEmpty())
                glCallList(airportList[i]->getGndDisplayList());
        }

        // Deliveries
        for(int i = 0; i < airportList.size(); i++) {
            if(airportList[i] == 0) continue;
            if(!airportList[i]->getDeliveries().isEmpty())
                glCallList(airportList[i]->getDelDisplayList());
        }
    glEndList();

    // FIR borders
    if(sectorPolygonBorderLinesList == 0)
        sectorPolygonBorderLinesList = glGenLists(1);

    if(!allSectorsDisplayed) {
        // first, make sure all lists are there
        for(int i = 0; i < sectorsToDraw.size(); i++) {
            Sector *f = sectorsToDraw[i]->sector;
            if(f == 0) continue;
            f->getBorderLine();
        }

        if(Settings::firBorderLineStrength() > 0) {
            glNewList(sectorPolygonBorderLinesList, GL_COMPILE);
                for(int i = 0; i < sectorsToDraw.size(); i++) {
                    Sector *f = sectorsToDraw[i]->sector;
                    if(f == 0) continue;
                    glCallList(f->getBorderLine());
                }
            glEndList();
        }

    } else {
        // display ALL fir borders
        QList<Sector*> sectors = NavData::getInstance()->sectors().values();
        for(int i = 0; i < sectors.size(); i++) {
            if(sectors[i] == 0) continue;
            sectors[i]->getBorderLine();
        }

        glNewList(sectorPolygonBorderLinesList, GL_COMPILE);
            for(int i = 0; i < sectors.size(); i++) {
                if(sectors[i] == 0) continue;
                glCallList(sectors[i]->getBorderLine());
            }
        glEndList();
    }


    // APP border lines
    if(appBorderLinesList == 0)
        appBorderLinesList = glGenLists(1);

    for(int i = 0; i < airportList.size(); i++) {
        if(airportList[i] == 0) continue;
        if(!airportList[i]->getApproaches().isEmpty())
            airportList[i]->getAppBorderDisplayList();
    }

    if(Settings::appBorderLineStrength() > 0) {
        glNewList(appBorderLinesList, GL_COMPILE);
            for(int i = 0; i < airportList.size(); i++) {
                if(airportList[i] == 0) continue;
                if(!airportList[i]->getApproaches().isEmpty())
                    glCallList(airportList[i]->getAppBorderDisplayList());
            }
        glEndList();
    }
}

void GLWidget::newWhazzupData(bool isNew) {
    if(isNew) {
        qDebug() << "GLWidget/newWhazzupData";
        updateAirports();
        sectorsToDraw = Whazzup::getInstance()->whazzupData().activeSectors();

        createPilotsList();
        createAirportsList();
        prepareDisplayLists();

        updateGL();
    }
    qDebug() << "GLWidget/newWhazzupData -- finished";
}

void GLWidget::displayAllSectors(bool value) {
    allSectorsDisplayed = value;
    newWhazzupData();
}

void GLWidget::showInactiveAirports(bool value) {
    Settings::setShowInactiveAirports(value);
    newWhazzupData();
}

void GLWidget::initializeGL() {
    qglClearColor(Settings::backgroundColor().dark());
    createObjects();

    if(Settings::displaySmoothDots()) {
       glEnable(GL_POINT_SMOOTH);
    }
    if(Settings::displaySmoothLines()) {
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
    }
    if(Settings::enableBlend())
        glEnable(GL_BLEND);

    glEnable(GL_LINE_STIPPLE);
    glEnable(GL_TEXTURE_2D);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -10);
    glRotated(xRot, 1.0, 0.0, 0.0);
    glRotated(yRot, 0.0, 1.0, 0.0);
    glRotated(zRot, 0.0, 0.0, 1.0);

    if(Settings::enableBlend())
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glCallList(orbList);
    glCallList(gridlinesList);

    glCallList(sectorPolygonsList);
    glCallList(airportControllersList);

    glCallList(countriesList);
    glCallList(coastlineList);

    glCallList(sectorPolygonBorderLinesList);
    glCallList(appBorderLinesList);

    if(Settings::showAirportCongestion())
        glCallList(congestionsList);

    glCallList(airportsList);

    glCallList(pilotsList);

    if(Settings::showInactiveAirports() && zoom < inactiveAirportDotZoomTreshold)
        glCallList(airportsInactiveList);

    if(zoom < fixZoomTreshold && Settings::showFixes())
        glCallList(fixesList);

    renderLabels();
}

void GLWidget::updateAirports() {
    NavData::getInstance()->updateData(Whazzup::getInstance()->whazzupData());
}

void GLWidget::resizeGL(int width, int height) {
    aspectRatio = (double)width / (double)height;
    glViewport(0, 0, width, height);
    resetZoom();
}

void GLWidget::resetZoom() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5 * zoom * aspectRatio, +0.5 * zoom * aspectRatio,
            +0.5 * zoom, -0.5 * zoom, 0, 10);
    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    QToolTip::hideText();

    lastPos = mouseDownPos = QPoint(); // fake to disallow mapClicked()

    double lat, lon;
    mouse2latlon(event->x(), event->y(), lat, lon);
    setMapPosition(lat, lon, zoom);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    QToolTip::hideText();
    lastPos = mouseDownPos = event->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    QToolTip::hideText();

    if(mouseDownPos == event->pos() && event->button() == Qt::LeftButton) { // Left-Button needed on Windows explicitly
        emit mapClicked(event->x(), event->y(), event->globalPos());
    }
    if(mapIsMoving == true)
    {
        emit newPosition();
    }
}

void GLWidget::handleRotation(QMouseEvent *event) {
    const double zoomFactor = zoom / 10;

    double dx = (event->x() - lastPos.x()) * zoomFactor / aspectRatio;

    // compensate for longitude differences, but only if xRot < 80¡
    // otherwise we get a division by (almost) zero, crashing the application
    const double limit = cos(80 * Pi180);
    double xfactor = cos(xRot * Pi180);
    if(fabs(xfactor) > limit)
        dx /= xfactor;

    double dy = (-event->y() + lastPos.y()) * zoomFactor;

    if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::RightButton)) {
        xRot = xRot + dy;
        yRot = yRot + dx;
        normalizeAngle(&xRot);
        normalizeAngle(&yRot);
        updateGL();
    }

    lastPos = event->pos();
    //emit newPosition();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() != 0) {
        handleRotation(event);
        mapIsMoving = true;
    }
}

void GLWidget::zoomIn(double factor) {
    zoom -= zoom * 0.2f * factor * Settings::zoomFactor();
    resetZoom();
    updateGL();
}

void GLWidget::zoomOut(double factor) {
    zoom += zoom * 0.2f * factor * Settings::zoomFactor();
    resetZoom();
    updateGL();
}

void GLWidget::wheelEvent(QWheelEvent* event) {
    QToolTip::hideText();
    //if(event->orientation() == Qt::Vertical) {
    if (abs(event->delta()) > Settings::wheelMax()) { // always recalibrate if bigger values are found
        Settings::setWheelMax(abs(event->delta()));
    }
    zoomIn((double) event->delta() / Settings::wheelMax());
}

void GLWidget::createOrbList() {
    orbList = glGenLists(1);
    glNewList(orbList, GL_COMPILE);
    // Globe (solid sphere)
    qglColor(Settings::globeColor());
    for (int lat = -90; lat <= 90; lat += 2) {
        for (int lon = -180; lon <= 180; lon += 10) {
            glBegin(GL_POLYGON);
            VERTEX(lat, lon);
            VERTEX(lat + 2, lon);
            VERTEX(lat + 2, lon + 10);
            VERTEX(lat, lon + 10);
            glEnd();
        }
    }
    glEndList();
}

void GLWidget::createCoastlineList() {
    coastlineList = glGenLists(1);

    glNewList(coastlineList, GL_COMPILE);
    // coastlines
    qglColor(Settings::coastLineColor());
    glLineWidth(Settings::coastLineStrength());

    LineReader lineReader(Settings::dataDirectory() + "coastline.dat");
    QList<QPair<double, double> > line = lineReader.readLine();
    while (!line.isEmpty()) {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < line.size(); i++) {
            QPair<double, double> p = line[i];
            VERTEX(p.first, p.second);
        }
        glEnd();
        line = lineReader.readLine();
    }
    glEndList();
}

void GLWidget::createGridlinesList() {
    gridlinesList = glGenLists(1);

    glNewList(gridlinesList, GL_COMPILE);
    // meridians
    qglColor(Settings::gridLineColor());
    glLineWidth(Settings::gridLineStrength());
    for (int lon = -180; lon <= 180; lon += 10) {
        glBegin(GL_LINE_STRIP);
        for (int lat = -80; lat <= 80; lat += 2) {
            VERTEX(lat, lon);
        }
        glEnd();
    }

    // parallels
    for (int lat = -80; lat <= 80; lat += 10) {
        glBegin(GL_LINE_STRIP);
        for (int lon = -180; lon <= 180; lon += 2) {
            VERTEX(lat, lon);
        }
        glEnd();
    }
    glEndList();
}

void GLWidget::createPilotsList() {
    makeCurrent();

    if(pilotsList == 0)
        pilotsList = glGenLists(1);

    glNewList(pilotsList, GL_COMPILE);

    QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().getPilots();

    // aircraft dots
    glPointSize(Settings::pilotDotSize());
    glBegin(GL_POINTS);
    qglColor(Settings::pilotDotColor());
    for (int i = 0; i < pilots.size(); i++) {
        const Pilot *p = pilots[i];
        VERTEX(p->lat, p->lon);
    }
    glEnd();

    // timelines
    int seconds = Settings::timelineSeconds();
    if(seconds > 0) {
        glLineWidth(Settings::timeLineStrength());
        glBegin(GL_LINES);
        qglColor(Settings::timeLineColor());
        for (int i = 0; i < pilots.size(); i++) {
            const Pilot *p = pilots[i];
            if(p->groundspeed < 30)
                continue;
            VERTEX(p->lat, p->lon);
            double lat, lon;
            p->positionInFuture(&lat, &lon, seconds);
            VERTEX(lat, lon);
        }
        glEnd();
    }

    // flight paths
    pilots = Whazzup::getInstance()->whazzupData().getAllPilots();
    for (int i = 0; i < pilots.size(); i++) {
        const Pilot *p = pilots[i];
        p->plotFlightPath();
    }

    // planned Route from Flight Plan Dialog
    if(plotFlightPlannedRoute)
        PlanFlightDialog::getInstance()->plotPlannedRoute();

    glEndList();
}

void GLWidget::createAirportsList() {
    makeCurrent();
    if(airportsList == 0)
        airportsList = glGenLists(1);
    QList<Airport*> airportList = NavData::getInstance()->airports().values();

    glNewList(airportsList, GL_COMPILE);
    glPointSize(Settings::airportDotSize());
    qglColor(Settings::airportDotColor());
    glBegin(GL_POINTS);
    for (int i = 0; i < airportList.size(); i++) {
        Airport *a = airportList[i];
        if(a == 0) continue;
        if(a->isActive()) {
            VERTEX(a->lat, a->lon);
        }
    }
    glEnd();
    glEndList();

    if(airportsInactiveList == 0)
        airportsInactiveList = glGenLists(1);
    glNewList(airportsInactiveList, GL_COMPILE);
    if(Settings::showInactiveAirports()) {
        glPointSize(Settings::inactiveAirportDotSize());
        qglColor(Settings::inactiveAirportDotColor());
        glBegin(GL_POINTS);
        for (int i = 0; i < airportList.size(); i++) {
            Airport *a = airportList[i];
            if(a == 0) continue;
            if(!a->isActive()) {
                VERTEX(a->lat, a->lon);
            }
        }
        glEnd();
    }
    glEndList();

    // airport congestion based on filtered traffic
    if(congestionsList == 0)
        congestionsList = glGenLists(1);
    glNewList(congestionsList, GL_COMPILE);
    if(Settings::showAirportCongestion()) {
        QColor borderLine = Settings::airportCongestionBorderLineColor();
        glColor4f(borderLine.redF(), borderLine.greenF(), borderLine.blueF(), borderLine.alphaF());
        glLineWidth(Settings::airportCongestionBorderLineStrength());
        for(int i = 0; i < airportList.size(); i++) {
            if(airportList[i] == 0) continue;
            if(!airportList[i]->isActive()) continue;
            int congested = airportList[i]->numFilteredArrivals() + airportList[i]->numFilteredDepartures();
            if(congested < Settings::airportCongestionMinimum()) continue;
            GLdouble circle_distort = cos(airportList[i]->lat * Pi180);
            QList<QPair<double, double> > points;
            for(int h = 0; h <= 360; h += 10) {
                double x = airportList[i]->lat + Nm2Deg(congested*5) * circle_distort *cos(h * Pi180);
                double y = airportList[i]->lon + Nm2Deg(congested*5) * sin(h * Pi180);
                points.append(QPair<double, double>(x, y));
            }
            glBegin(GL_LINE_LOOP);
            for (int h = 0; h < points.size(); h++) {
                VERTEX(points[h].first, points[h].second);
            }
            glEnd();
        }
    }
    glEndList();
}

bool GLWidget::pointIsVisible(double lat, double lon, int *px, int *py) const {
    // create a feedback buffer
    GLfloat buffer[6];
    glFeedbackBuffer(6, GL_3D, buffer);

    // set to feedback mode
    glRenderMode(GL_FEEDBACK);
    // send a point to GL
    glBegin(GL_POINTS);
    VERTEX(lat, lon);
    glEnd();
    int size = glRenderMode(GL_RENDER);

    // if the feedback buffer size is zero, the point was clipped
    if(size > 0) {
        if(px != 0) *px = (int)buffer[1];
        if(py != 0) *py = height() - (int)buffer[2];
    }
    return size > 0;
}

void GLWidget::renderLabels() {
    fontRectangles.clear();
    allFontRectangles.clear();

    // FIR labels
    QList<MapObject*> objects;
    for(int i = 0; i < sectorsToDraw.size(); i++) objects.append(sectorsToDraw[i]);
    renderLabels(objects, Settings::firFont(), controllerLabelZoomTreshold, Settings::firFontColor());

    // Airport labels
    objects.clear();
    QList<Airport*> airportList = NavData::getInstance()->airportsTrafficSorted(); //ordered by congestion, big airport's labels will always be drawn first
    for(int i = 0; i < airportList.size(); i++) {
        if(airportList[i] == 0) continue;
        if(airportList[i]->isActive()) objects.append(airportList[i]);
    }
    renderLabels(objects, Settings::airportFont(), airportLabelZoomTreshold, Settings::airportFontColor());

    // Pilot labels
    objects.clear();
    QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().getPilots();
    for(int i = 0; i < pilots.size(); i++) {
        objects.append(pilots[i]);
    }
    renderLabels(objects, Settings::pilotFont(), pilotLabelZoomTreshold, Settings::pilotFontColor());

    // Inactive airports
    if(Settings::showInactiveAirports()) { // + inactive labels
        objects.clear();
        for(int i = 0; i < airportList.size(); i++) {
            if(airportList[i] == 0) continue;
            if(!airportList[i]->isActive()) objects.append(airportList[i]);
        }
        renderLabels(objects, Settings::inactiveAirportFont(), inactiveAirportLabelZoomTreshold, Settings::inactiveAirportFontColor());
    }

    // Fixes
    if(Settings::showFixes()) {
        const Airac& airac = NavData::getInstance()->getAirac();
        const QList<Waypoint*>& fixes = airac.getAllWaypoints();
        objects.clear();
        for(int i = 0; i < fixes.size(); i++) {
            if(fixes[i] == 0) continue;
            objects.append(fixes[i]);
        }
        renderLabels(objects, Settings::inactiveAirportFont(), fixZoomTreshold, Settings::countryLineColor());
    }
}

void GLWidget::renderLabels(const QList<MapObject*>& objects, const QFont& font, double zoomTreshold, QColor color) {
    if(zoom > zoomTreshold)
        return; // don't draw if too far away

    int maxLabels = Settings::maxLabels();
    double alpha = (zoomTreshold - zoom) / zoomTreshold * 1.5;
    if(alpha > 1) alpha = 1; if(alpha < 0) alpha = 0;
    color.setAlphaF(alpha);

    QFontMetricsF fontMetrics(font, this);
    for (int i = 0; i < objects.size() && fontRectangles.size() < maxLabels; i++) {
        MapObject *o = objects[i];
        if(o == 0) continue;

        int x, y;
        double lat = o->lat;
        double lon = o->lon;
        if(pointIsVisible(lat, lon, &x, &y)) {
            QString text = o->mapLabel();
            QRectF rect = fontMetrics.boundingRect(text);
            int drawX, drawY;
            drawX = static_cast<int>(x - rect.width() / 2); // center horizontally
            drawY = static_cast<int>(y - rect.height() - 5); // some px above dot
            rect.moveTo(drawX, drawY);

            FontRectangle fontRect = FontRectangle(rect, o);
            allFontRectangles.append(fontRect);
            if(shouldDrawLabel(fontRect)) {
                qglColor(color);
                renderText(drawX, static_cast<int>(drawY + rect.height()), text, font);
                fontRectangles.append(fontRect); // let's add it not only when it it's drawn but always to get more selection for nearby objects
            }
        }
    }
}

bool GLWidget::shouldDrawLabel(const FontRectangle& rect) {
    int shrinkCheckRectByFactor = static_cast<int>(1.5); // be less conservative in edge overlap-checking
    for(int i = 0; i < fontRectangles.size(); i++) {
        QRectF checkrect = fontRectangles[i].rect();
        checkrect.setWidth(checkrect.width() / shrinkCheckRectByFactor); // make them smaller to allow a tiny bit of intersect
        checkrect.setHeight(checkrect.height() / shrinkCheckRectByFactor);
        checkrect.moveCenter(fontRectangles[i].rect().center());

        if(rect.rect().intersects(checkrect))
            return false;
    }
    return true;
}

void GLWidget::createCountriesList() {
    countriesList = glGenLists(1);

    glNewList(countriesList, GL_COMPILE);
    // countries
    qglColor(Settings::countryLineColor());
    glLineWidth(Settings::countryLineStrength());
    LineReader countries = LineReader(Settings::dataDirectory() + "countries.dat");
    QList<QPair<double, double> > line = countries.readLine();
    while (!line.isEmpty()) {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < line.size(); i++) {
            QPair<double, double> p = line[i];
            VERTEX(p.first, p.second);
        }
        glEnd();
        line = countries.readLine();
    }
    glEndList();
}

#define TRIANGLE_SIZE 0.01

void GLWidget::createFixesList() {
    fixesList = glGenLists(1);

    glNewList(fixesList, GL_COMPILE);
    // fixes
    qglColor(Settings::countryLineColor());
    glLineWidth(Settings::countryLineStrength());

    const Airac& airac = NavData::getInstance()->getAirac();
    const QList<Waypoint*>& fixes = airac.getAllWaypoints();


    double sin30 = 0.5;
    double cos30 = 0.8660254037;
    double tri_c = TRIANGLE_SIZE;
    double tri_a = tri_c * cos30;
    double tri_b = tri_c * sin30;

    glBegin(GL_TRIANGLES);
    QList<Waypoint*>::const_iterator iter = fixes.begin();
    while(iter != fixes.end()) {
        double lat = (*iter)->lat;
        double lon = (*iter)->lon;
        double circle_distort = cos(lat * Pi180);
        double tri_b_c = tri_b * circle_distort;

        VERTEX(lat - tri_b_c, lon - tri_a);
        VERTEX(lat - tri_b_c, lon + tri_a);
        VERTEX(lat + tri_c * circle_distort, lon);
        ++iter;
    }
    glEnd();

    glEndList();
}

void GLWidget::createObjects() {
    createOrbList();
    createCoastlineList();
    createGridlinesList();
    createCountriesList();

    if(!NavData::getInstance()->getAirac().isEmpty()) {
        createFixesList();
    }
}

void GLWidget::normalizeAngle(double *angle) const {
    while (*angle < 0)
        *angle += 360;
    while (*angle > 360)
        *angle -= 360;
}

bool GLWidget::mouse2latlon(int x, int y, double& lat, double& lon) const {
    // This function converts screen mouse coordinates into latitude/longitude of the map.
    // A rotation matrix can be decomposed as a product of three elemental rotations.
    // Euler angles are a means of representing the spatial orientation of any frame
    // of the space as a composition of rotations from a reference frame.

    // First, we convert the pointer coorinates to Cartesian coordinates of the opengl environment
    double xs = ((double)(2*x) / (double)(width()) - 1) * aspectRatio * zoom / 2;
    double ys = ((double)(2*y) / (double)(height()) - 1) * zoom / 2;
    double zs = sqrt(1 - (xs*xs) - (ys*ys)); // As the radius of globe is 1
    if(!(zs <= 0) && !(zs > 0))
        return false; // z is NaN - mouse is not on globe

    // the formulae use in fact three angles - fi, theta, psi
    // in our case, due to the fact that we rotate the globe only around a single axis,
    // we have fi = pi/2 and psi = 0. Therefore the only equation remaining:
    double theta = xRot * Pi180;

    // Determine x0 y0 z0 (the new Cartesian coordinates after derotation)
    double x0 = ys * sin(theta) - zs * cos(theta);
    double y0 = ys * cos(theta) + zs * sin(theta);
    double z0 = xs;

    // After derotation, we can determine lat/lon as follows:
    lat = -atan(y0 / sqrt(1 - (y0*y0))) * 180 / Pi;
    lon = atan(x0 / z0) * 180 / Pi - 90;

    // rem the sign was lost when applying Atn so:
    if (xs >= 0)
        lon += 180;

    // Furthermore:
    lon -= yRot;
    if (lon < -180) lon += 360;
    if (lon > 180) lon -= 360;

    return true;
}

void GLWidget::rightClick(const QPoint& pos) {
    mouseDownPos = QPoint(); // make sure that we don't handle mouse-up for this right-click

    QList<MapObject*> objects = objectsAt(pos.x(), pos.y());
    int countRelevant = 0;
    Pilot *pilot = 0;
    Airport *airport = 0;
    for(int i = 0; i < objects.size(); i++) {
        Pilot *p = dynamic_cast<Pilot*>(objects[i]);
        if(p != 0) {
            if(pilot != 0)
                return; // abort search, too many pilots around
            pilot = p;
            countRelevant++;
        }

        Airport *a = dynamic_cast<Airport*>(objects[i]);
        if(a != 0) {
            if(airport != 0)
                return; // abort search, too much stuff around
            airport = a;
            countRelevant++;
            break; // priorise airports
        }

        if(countRelevant > 1) {
            Window::getInstance()->statusBar()->showMessage("Too many objects under cursor", 3000);
            return; // area too crowded
        }
    }
    if(countRelevant == 0) {
        Window::getInstance()->statusBar()->showMessage("No object under cursor", 3000);
        return;
    }
    if(countRelevant != 1) return; // ambiguous search result

    if(airport != 0) {
        Window::getInstance()->statusBar()->showMessage(QString("Toggled routes for %1").arg(airport->label), 2000);
        airport->toggleFlightLines();
        createPilotsList();
        updateGL();
        return;
    }

    if(pilot != 0) {
        // display flight path for pilot
        Window::getInstance()->statusBar()->showMessage(QString("Toggled route for %1").arg(pilot->label), 2000);
        pilot->toggleDisplayPath();
        createPilotsList();
        updateGL();
        return;
    }
}

bool GLWidget::event(QEvent *event) {
    if(event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QList<MapObject*> objects = objectsAt(helpEvent->pos().x(), helpEvent->pos().y());
        if(objects.isEmpty()) QToolTip::hideText();
        else {
            QString toolTip;
            for(int i = 0; i < objects.size(); i++) {
                if(i > 0) toolTip += "\n";
                toolTip += objects[i]->toolTip();
            }
            QToolTip::showText(helpEvent->globalPos(), toolTip);
        }
    }
    if(event->type() == QEvent::ContextMenu) {
        QContextMenuEvent *contextEvent = static_cast<QContextMenuEvent *>(event);
        rightClick(contextEvent->pos());
    }
    return QGLWidget::event(event);
}

QList<MapObject*> GLWidget::objectsAt(int x, int y, double radius) const {
    QList<MapObject*> result;

    // scan text labels
    for(int i = 0; i < allFontRectangles.size(); i++) {
        if(allFontRectangles[i].rect().contains(x, y))
            result.append(allFontRectangles[i].object());
    }

    double lat, lon;
    bool onGlobe = mouse2latlon(x, y, lat, lon);
    if(!onGlobe)
        return result;

    if(radius == 0) radius = 30*zoom;
    double radiusDeg = Nm2Deg(radius);
    radiusDeg *= radiusDeg;

    QList<Airport*> airportList = NavData::getInstance()->airports().values();
    for (int i = 0; i < airportList.size(); i++) {
        Airport* a = airportList[i];
        if(a == 0) continue;
        if(a->isActive()) {
            double x = a->lat - lat;
            double y = a->lon - lon;
            if(x*x + y*y <= radiusDeg) {
                result.removeAll(a);
                result.append(a);
            }
        }
    }

    QList<Controller*> controllers = Whazzup::getInstance()->whazzupData().getControllers();
    QList<MapObject*> observers;
    for(int i = 0; i < controllers.size(); i++) {
        Controller *c = controllers[i];
        double x = c->lat - lat;
        double y = c->lon - lon;
        if(x*x + y*y <= radiusDeg) {
            if(c->isObserver())
                observers.append(c);
            else {
                result.removeAll(c);
                result.append(c);
            }
        }
    }

    QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().getPilots();
    for(int i = 0; i < pilots.size(); i++) {
        Pilot *p = pilots[i];
        double x = p->lat - lat;
        double y = p->lon - lon;
        if(x*x + y*y <= radiusDeg) {
            result.removeAll(p);
            result.append(p);
        }
    }

    result += observers;
    return result;
}

void GLWidget::rememberPosition(int nr) {
    Window::getInstance()->statusBar()->showMessage(QString("Remembered position %1").arg(nr), 2000);
    Settings::setRememberedMapPosition(xRot, yRot, zRot, zoom, nr);
}

void GLWidget::restorePosition(int nr) {
    Settings::getRememberedMapPosition(&xRot, &yRot, &zRot, &zoom, nr);
    normalizeAngle(&xRot);
    normalizeAngle(&yRot);
    resetZoom();
    updateGL();
    emit newPosition();
}

void GLWidget::scrollBy(int moveByX, int moveByY) {
    double lat, lon;
    int fakeMousePosX = static_cast<int>(width() * (0.5 + (float) moveByX / 6));
    int fakeMousePosY = static_cast<int>(height() * (0.5 + (float) moveByY / 6));
    mouse2latlon(fakeMousePosX, fakeMousePosY, lat, lon);
    setMapPosition(lat, lon, zoom);
    updateGL();
}
