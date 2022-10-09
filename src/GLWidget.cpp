/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "GLWidget.h"

#include "helpers.h"
#include "LineReader.h"
#include "Tessellator.h"
#include "Pilot.h"
#include "Controller.h"
#include "Whazzup.h"
#include "NavData.h"
#include "Settings.h"
#include "Waypoint.h"
#include "PlanFlightDialog.h"
#include "AirportDetails.h"
#include "PilotDetails.h"
//#include <GL/glext.h>   // Multitexturing - not platform-independant

GLWidget::GLWidget(QGLFormat fmt, QWidget *parent) :
        QGLWidget(fmt, parent),
        _mapMoving(false), _mapZooming(false), _mapRectSelecting(false),
        _lightsGenerated(false),
        _earthTex(0),
        _earthList(0), _coastlinesList(0), _countriesList(0), _gridlinesList(0),
        _pilotsList(0), _activeAirportsList(0), _inactiveAirportsList(0), _fixesList(0),
        _usedWaypointsList(0), _sectorPolygonsList(0), _sectorPolygonBorderLinesList(0),
        _congestionsList(0),
        _sondeLabelZoomTreshold(3.),
        _pilotLabelZoomTreshold(.9),
        _activeAirportLabelZoomTreshold(1.2), _inactiveAirportLabelZoomTreshold(.15),
        _controllerLabelZoomTreshold(2.), _allWaypointsLabelZoomTreshold(.1),
        _usedWaypointsLabelZoomThreshold(1.2),
        _xRot(0), _yRot(0), _zRot(0), _zoom(2), _aspectRatio(1),
        _highlighter(0) {
    setAutoFillBackground(false);
    setMouseTracking(true);

    _allSectorsDisplayed = Settings::showAllSectors();

    // call default (=9) map position
    Settings::rememberedMapPosition(&_xRot, &_yRot, &_zRot, &_zoom, 9);
    _xRot = Helpers::modPositive(_xRot, 360.);
    _yRot = Helpers::modPositive(_yRot, 360.);
    _zRot = Helpers::modPositive(_zRot, 360.);
    resetZoom();
    emit newPosition();

    clientSelection = new ClientSelectionWidget();
}

GLWidget::~GLWidget() {
    makeCurrent();
    glDeleteLists(_earthList, 1); glDeleteLists(_gridlinesList, 1);
    glDeleteLists(_coastlinesList, 1); glDeleteLists(_countriesList, 1);
    glDeleteLists(_fixesList, 1);
    glDeleteLists(_usedWaypointsList, 1); glDeleteLists(_pilotsList, 1);
    glDeleteLists(_activeAirportsList, 1); glDeleteLists(_inactiveAirportsList, 1);
    glDeleteLists(_congestionsList, 1);
    glDeleteLists(_sectorPolygonsList, 1); glDeleteLists(_sectorPolygonBorderLinesList, 1);

    if (_earthTex != 0)
        deleteTexture(_earthTex);
        //glDeleteTextures(1, &earthTex); // handled Qt'ish by deleteTexture
    if (_cloudTex != 0)
        deleteTexture(_cloudTex);
    gluDeleteQuadric(_earthQuad);

    delete clientSelection;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Methods handling position: world lat/lon <-> scene x/y/z <-> mouse x/y and related.
//
// scene -> unrotated world (looking onto N0/E0):
//      (1,0,0)->(0,90), (0,1,0)->(0,180), (0,0,1)->(-90,0) [Southpole].
// The scene is then rotated by xRot/yRot/zRot. When looking onto N0/E0, -90°/0°/0°
// This looks a bit anarchic, but it fits the automatically created texture coordinates.
// call drawCoordinateAxii() inside paintGL() to se where the axii are.
void GLWidget::setMapPosition(double lat, double lon, double newZoom, bool updateGL) {
    _xRot = Helpers::modPositive(270. - lat, 360.);
    _zRot = Helpers::modPositive(     - lon, 360.);
    _zoom = newZoom;
    resetZoom();
    if (updateGL)
        this->updateGL();
    emit newPosition();
}

/**
  current lat/lon
**/
QPair<double, double> GLWidget::currentPosition() const {
    return QPair<double, double>(Helpers::modPositive(-90. - _xRot + 180., 360.) - 180.,
                                 Helpers::modPositive(     - _zRot + 180., 360.) - 180.);
}

/**
  rotate according to mouse movement
**/
void GLWidget::handleRotation(QMouseEvent*) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    const double zoomFactor = _zoom / 10.;
    double dx = (  currentPos.x() - _lastPos.x()) * zoomFactor;
    double dy = (- currentPos.y() + _lastPos.y()) * zoomFactor;
    _xRot = Helpers::modPositive(_xRot + dy + 180., 360.) - 180.;
    _zRot = Helpers::modPositive(_zRot + dx + 180., 360.) - 180.;
    updateGL();
    _lastPos = currentPos;
    emit newPosition();
}

/**
  check if the given point (relative to this widget) is on the globe
**/
bool GLWidget::isOnGlobe(int x, int y) const {
    double xGl = (2. * x / width()  - 1.) * _aspectRatio * _zoom / 2;
    double zGl = (2. * y / height() - 1.) * _zoom / 2;
    double yGl = sqrt(1 - (xGl*xGl) - (zGl*zGl)); // As the radius of globe is 1
    return !qIsNaN(yGl);
}

/**
 Converts screen mouse coordinates into latitude/longitude of the map.
 Calculation based on Euler angles.
 @returns false if x/y is not on the globe
**/
bool GLWidget::mouse2latlon(int x, int y, double &lat, double &lon) const {
    // 1) mouse coordinates to Cartesian coordinates of the openGL environment [-1...+1]
    double xGl = (2. * x / width()  - 1.) * _aspectRatio * _zoom / 2;
    double zGl = (2. * y / height() - 1.) * _zoom / 2;
    double yGl = sqrt(1 - (xGl*xGl) - (zGl*zGl)); // As the radius of globe is 1
    if(qIsNaN(yGl))
            return false; // mouse is not on globe

    // 2) skew (rotation around the x-axis, where 0° means looking onto the equator)
    double theta = (_xRot + 90.) * Pi180;

    // 3) new cartesian coordinates, taking skew into account
    double x0 = -zGl * qSin(theta) + yGl * qCos(theta);
    double z0 = +zGl * qCos(theta) + yGl * qSin(theta);
    double y0 = xGl;

    // 4) now to lat/lon
    lat = qAtan(-z0 / qSqrt(1 - (z0*z0))) * 180 / M_PI;
    lon = qAtan(-x0 / y0) * 180 / M_PI - 90;

    // 5) qAtan might have lost the sign
    if (xGl >= 0)
            lon += 180;

    lon = Helpers::modPositive(lon - _zRot + 180., 360.) - 180.;
    return true;
}

void GLWidget::scrollBy(int moveByX, int moveByY) {
    QPair<double, double> cur = currentPosition();
    setMapPosition(cur.first  - (double) moveByY * _zoom * 6., // 6° on zoom=1
                   cur.second + (double) moveByX * _zoom * 6., _zoom);
    updateGL();
}

void GLWidget::resetZoom() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5 * _zoom * _aspectRatio, +0.5 * _zoom * _aspectRatio, // clipping left/right/bottom/top/near/far
            +0.5 * _zoom, -0.5 * _zoom, 8, 10); // or gluPerspective for perspective viewing
    //gluPerspective(zoom, aspectRatio, 8, 10); // just for reference, if you want to try it
    glMatrixMode(GL_MODELVIEW);
}

/**
  Check if a point is visible to the current viewport
**/
bool GLWidget::isPointVisible(double lat, double lon, int *px, int *py) const {
    GLfloat buffer[6];
    glFeedbackBuffer(6, GL_3D, buffer); // create a feedback buffer
    glRenderMode(GL_FEEDBACK); // set to feedback mode
    glBegin(GL_POINTS); // send a point to GL
    VERTEX(lat, lon);
    glEnd();
    if(glRenderMode(GL_RENDER) > 0) { // if the feedback buffer size is zero, the point was clipped
        if(px != 0) *px = (int)buffer[1];
        if(py != 0) *py = height() - (int)buffer[2];
        return true;
    }
    return false;
}

void GLWidget::rememberPosition(int nr) {
    GuiMessages::message(QString("Remembered map position %1").arg(nr));
    Settings::setRememberedMapPosition(_xRot, _yRot, _zRot, _zoom, nr);
}

void GLWidget::restorePosition(int nr) {
    GuiMessages::message(QString("Recalled map position %1").arg(nr));
    Settings::rememberedMapPosition(&_xRot, &_yRot, &_zRot, &_zoom, nr);
    _xRot = Helpers::modPositive(_xRot, 360.);
    _zRot = Helpers::modPositive(_zRot, 360.);
    resetZoom();
    updateGL();
    emit newPosition();
}

const QPair<double, double> GLWidget::sunZenith(const QDateTime &dateTime) const {
    // dirtily approximating present zenith Lat/Lon (where the sun is directly above).
    // scientific solution: http://openmap.bbn.com/svn/openmap/trunk/src/openmap/com/bbn/openmap/layer/daynight/SunPosition.java
    // [sunPosition()] - that would have been at least 100 lines of code...
    return QPair<double, double>(
                -23. * qCos((double) dateTime.date().dayOfYear() /
                        (double)dateTime.date().daysInYear() * 2.*M_PI),
                -((double) dateTime.time().hour() +
                        (double) dateTime.time().minute() / 60.) * 15. - 180.
    );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Methods preparing displayLists
//
void GLWidget::createPilotsList() {
    qDebug() << "GLWidget::createPilotsList()";
    makeCurrent();

    if(_pilotsList == 0)
        _pilotsList = glGenLists(1);

    glNewList(_pilotsList, GL_COMPILE);

    QList<Pilot*> pilots = Whazzup::instance()->whazzupData().pilots.values();

    // aircraft dots
    if (Settings::pilotDotSize() > 0.) {
        glPointSize(Settings::pilotDotSize());
        glBegin(GL_POINTS);
        qglColor(Settings::pilotDotColor());
        foreach(const Pilot *p, pilots) {
            if (qFuzzyIsNull(p->lat) && qFuzzyIsNull(p->lon))
                continue;

            VERTEX(p->lat, p->lon);
        }
        glEnd();
    }

    // timelines / leader lines
    if(Settings::timelineSeconds() > 0 && Settings::timeLineStrength() > 0.) {
        glLineWidth(Settings::timeLineStrength());
        glBegin(GL_LINES);
        qglColor(Settings::leaderLineColor());
        foreach(const Pilot *p, pilots) {
            if (p->groundspeed < 30)
                continue;

            if (qFuzzyIsNull(p->lat) && qFuzzyIsNull(p->lon))
                continue;

            VERTEX(p->lat, p->lon);
            QPair<double, double> pos = p->positionInFuture(Settings::timelineSeconds());
            VERTEX(pos.first, pos.second);
        }
        glEnd();
    }


    // flight paths, also for booked flights
    foreach(Pilot *p, Whazzup::instance()->whazzupData().allPilots()) {
        if (qFuzzyIsNull(p->lat) && qFuzzyIsNull(p->lon))
            continue;

        if (!p->showDepLine() && !p->showDestLine())
            continue;

        QList<Waypoint*> waypoints = p->routeWaypointsWithDepDest();
        int next = p->nextPointOnRoute(waypoints);
        QList<DoublePair> points; // these are the points that really get drawn
        if (p->showDepLine()) { // Dep -> plane
            for (int i = 0; i < next; i++) {
                points.append(DoublePair(waypoints[i]->lat, waypoints[i]->lon));
            }
        }

        // plane ok: draw to plane and reset list for DestLine
        points.append(DoublePair(p->lat, p->lon));
        if (Settings::depLineDashed())
            glLineStipple(3, 0xAAAA);
        qglColor(Settings::depLineColor());
        glLineWidth(Settings::depLineStrength());
        glBegin(GL_LINE_STRIP);
        NavData::plotPointsOnEarth(points);
        glEnd();
        if(Settings::depLineDashed())
            glLineStipple(1, 0xFFFF);

        points.clear();
        points.append(DoublePair(p->lat, p->lon));

        if (p->showDestLine()) { // plane -> Dest
            for (int i = next; i < waypoints.size(); i++)
                points.append(DoublePair(waypoints[i]->lat, waypoints[i]->lon));
            if (Settings::destLineDashed())
                glLineStipple(3, 0xAAAA);
            qglColor(Settings::destLineColor());
            glLineWidth(Settings::destLineStrength());
            glBegin(GL_LINE_STRIP);
            NavData::plotPointsOnEarth(points);
            glEnd();
            if(Settings::destLineDashed())
                glLineStipple(1, 0xFFFF);
        }
    }

    // planned route from Flightplan Dialog (does not really belong to pilots lists, but is convenient here)
    if(PlanFlightDialog::instance(false) != 0)
        PlanFlightDialog::instance()->plotPlannedRoute();

    glEndList();

    // used waypoints (dots)
    if(_usedWaypointsList == 0)
            _usedWaypointsList = glGenLists(1);

    if(Settings::showUsedWaypoints() && Settings::waypointsDotSize() > 0.) {
        glNewList(_usedWaypointsList, GL_COMPILE);
        qglColor(Settings::waypointsDotColor());
        glPointSize(Settings::waypointsDotSize());
        glBegin(GL_POINTS);
        foreach(Pilot *p, Whazzup::instance()->whazzupData().allPilots()) {
            if (p->showDepLine() || p->showDestLine()) {
                QList<Waypoint*> waypoints = p->routeWaypoints();
                int next = p->nextPointOnRoute(waypoints);
                if (p->showDepLine())
                    for (int i = 0; i < next; i++)
                        VERTEX(waypoints[i]->lat, waypoints[i]->lon);
                if (p->showDestLine())
                    for (int i = next; i < waypoints.size(); i++)
                        VERTEX(waypoints[i]->lat, waypoints[i]->lon);
            }
        }
        glEnd();
        glEndList();
    }

    qDebug() << "GLWidget::createPilotsList() -- finished";
}

void GLWidget::createAirportsList() {
    qDebug() << "GLWidget::createAirportsList() ";
    makeCurrent();
    if (_activeAirportsList == 0)
        _activeAirportsList = glGenLists(1);
    QList<Airport*> airportList = NavData::instance()->airports.values();

    // active airports
    glNewList(_activeAirportsList, GL_COMPILE);
    if (Settings::airportDotSize() > 0.) {
        glPointSize(Settings::airportDotSize());
        qglColor(Settings::airportDotColor());
        glBegin(GL_POINTS);
        foreach(const Airport *a, airportList) {
            //if(a == 0) continue;
            if(a->active)
                VERTEX(a->lat, a->lon);
        }
        glEnd();
    }
    glEndList();

    // inactive airports
    if(_inactiveAirportsList == 0)
        _inactiveAirportsList = glGenLists(1);
    glNewList(_inactiveAirportsList, GL_COMPILE);
    if(Settings::showInactiveAirports() && Settings::inactiveAirportDotSize() > 0.) {
        glPointSize(Settings::inactiveAirportDotSize());
        qglColor(Settings::inactiveAirportDotColor());
        glBegin(GL_POINTS);
        foreach(const Airport *a, airportList) {
            //if(a == 0) continue;
            if(!a->active)
                VERTEX(a->lat, a->lon);
        }
        glEnd();
    }
    glEndList();

    // airport congestion based on filtered traffic
    if(_congestionsList == 0)
        _congestionsList = glGenLists(1);
    glNewList(_congestionsList, GL_COMPILE);
    if(Settings::showAirportCongestion()) {
        qglColor(Settings::airportCongestionBorderLineColor());
        glLineWidth(Settings::airportCongestionBorderLineStrength());
        for(int i = 0; i < airportList.size(); i++) {
            if(airportList[i] == 0) continue;
            if(!airportList[i]->active) continue;
            int congested = airportList[i]->numFilteredArrivals + airportList[i]->numFilteredDepartures;
            if(congested < Settings::airportCongestionMinimum()) continue;
            GLdouble circle_distort = qCos(airportList[i]->lat * Pi180);
            QList<QPair<double, double> > points;
            for(int h = 0; h <= 360; h += 6) {
                double x = airportList[i]->lat + Nm2Deg(congested*5) * circle_distort *qCos(h * Pi180);
                double y = airportList[i]->lon + Nm2Deg(congested*5) * qSin(h * Pi180);
                points.append(QPair<double, double>(x, y));
            }
            glBegin(GL_LINE_LOOP);
            for (int h = 0; h < points.size(); h++)
                VERTEX(points[h].first, points[h].second);
            glEnd();
        }
    }
    glEndList();
    qDebug() << "GLWidget::createAirportsList() -- finished";
}

void GLWidget::createControllersLists() {
    qDebug() << "GLWidget::createControllersLists() ";

    // FIR polygons
    if(_sectorPolygonsList == 0)
        _sectorPolygonsList = glGenLists(1);

    // make sure all the lists are there to avoid nested glNewList calls
    foreach(const Controller *c, _sectorsToDraw) {
        if(c->sector != 0)
            c->sector->glPolygon();
    }

    // create a list of lists
    glNewList(_sectorPolygonsList, GL_COMPILE);
    foreach(const Controller *c, _sectorsToDraw) {
        if(c->sector != 0)
            glCallList(c->sector->glPolygon());
    }
    glEndList();

    // FIR borders
    if(_sectorPolygonBorderLinesList == 0)
        _sectorPolygonBorderLinesList = glGenLists(1);

    if(Settings::firBorderLineStrength() > 0.) {
        if(!_allSectorsDisplayed) {
            // first, make sure all lists are there
            foreach(const Controller *c, _sectorsToDraw) {
                if(c->sector != 0)
                    c->sector->glBorderLine();
            }
            glNewList(_sectorPolygonBorderLinesList, GL_COMPILE);
            foreach(const Controller *c, _sectorsToDraw) {
                if(c->sector != 0)
                    glCallList(c->sector->glBorderLine());
            }
            glEndList();
        } else {
            // display ALL fir borders
            foreach(Sector *s, NavData::instance()->sectors.values())
                s->glBorderLine();
            glNewList(_sectorPolygonBorderLinesList, GL_COMPILE);
            foreach(Sector *s, NavData::instance()->sectors.values())
                glCallList(s->glBorderLine());
            glEndList();
        }
    }
    qDebug() << "GLWidget::createControllersLists() -- finished";
}


void GLWidget::createHoveredControllersLists(QSet<Controller*> controllers) {
    qDebug() << "GLWidget::createHoveredSectorsLists() ";

    //Polygon
    if (_hoveredSectorPolygonsList == 0)
            _hoveredSectorPolygonsList = glGenLists(1);

    // make sure all the lists are there to avoid nested glNewList calls
    foreach(Controller *c, controllers) {
        if (c->sector != 0)
                c->sector->glPolygonHighlighted();
    }

    // create a list of lists
    glNewList(_hoveredSectorPolygonsList, GL_COMPILE);
    foreach(Controller *c, controllers) {
        if (c->sector != 0)
                glCallList(c->sector->glPolygonHighlighted());
    }
    glEndList();


    // FIR borders
    if (_hoveredSectorPolygonBorderLinesList == 0)
            _hoveredSectorPolygonBorderLinesList = glGenLists(1);


    if (!_allSectorsDisplayed && Settings::firHighlightedBorderLineStrength() > 0.) {
        // first, make sure all lists are there
        foreach(Controller *c, controllers) {
            if (c->sector != 0)
                    c->sector->glBorderLineHighlighted();
        }
        glNewList(_hoveredSectorPolygonBorderLinesList, GL_COMPILE);
        foreach(Controller *c, controllers) {
            if(c->sector != 0)
                    glCallList(c->sector->glBorderLineHighlighted());
        }
        glEndList();
    }
    qDebug() << "GLWidget::createHoveredSectorsLists() -- finished";
}

void GLWidget::createStaticLists() {
    // earth
    qDebug() << "GLWidget::createStaticLists() earth";
    _earthQuad = gluNewQuadric();
    qDebug() << "Generating quadric texture coordinates";
    gluQuadricTexture(_earthQuad, GL_TRUE); // prepare texture coordinates
    gluQuadricDrawStyle(_earthQuad, GLU_FILL); // FILL, LINE, SILHOUETTE or POINT
    gluQuadricNormals(_earthQuad, GLU_SMOOTH); // NONE, FLAT or SMOOTH
    gluQuadricOrientation(_earthQuad, GLU_OUTSIDE); // GLU_INSIDE

    parseEarthClouds();
    /*if (Settings::glTextures()) {
         QString earthTexFile = Settings::applicationDataDirectory(
                    QString("textures/%1").arg(Settings::glTextureEarth()));
                    //QString("textures/overlays/gfs.png"));
                    //QString("textures/clouds_4096.jpg"));
        QImage earthTexIm = QImage(earthTexFile);
        //QImage earthTexIm2 = QImage(earthTexFile2).copy(59, 142, 1220 - 59, 854 - 177);
        if (completedEarthTexIm.isNull())
            qWarning() << "Unable to load texture file: " << Settings::applicationDataDirectory(
                              QString("textures/%1").arg(Settings::glTextureEarth()));
        else {
            GLint max_texture_size;  glGetIntegerv(GL_MAX_TEXTURE_SIZE,  &max_texture_size);
            qDebug() << "OpenGL reported MAX_TEXTURE_SIZE as" << max_texture_size;

            // multitexturing units, if we need it once (headers in GL/glext.h, on Windows not available ?!)
            //GLint max_texture_units; glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);
            //qDebug() << "OpenGL reported MAX_TEXTURE_UNITS as" << max_texture_units;
            qDebug() << "Binding parsed texture as" << completedEarthTexIm.width()
                     << "x" << completedEarthTexIm.height() << "px texture";
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            //glGenTextures(1, &earthTex); // bindTexture does this the Qt'ish way already
            glGetError(); // empty the error buffer
            earthTex = bindTexture(completedEarthTexIm, GL_TEXTURE_2D,
                                   GL_RGBA, QGLContext::LinearFilteringBindOption); // QGLContext::MipmapBindOption
            if (GLenum glError = glGetError())
                qCritical() << QString("OpenGL returned an error (0x%1)").arg((int) glError, 4, 16, QChar('0'));
            gluQuadricTexture(earthQuad, GL_TRUE); // prepare texture coordinates
        }
    }*/



    _earthList = glGenLists(1);
    glNewList(_earthList, GL_COMPILE);
    qglColor(Settings::globeColor());
    gluSphere(_earthQuad, 1, qRound(360. / Settings::glCirclePointEach()), // draw a globe with..
              qRound(180. / Settings::glCirclePointEach())); // ..radius, slicesX, stacksZ
    // drawing the sphere by hand and setting TexCoords for Multitexturing (if we need it once):
//    glBegin(GL_TRIANGLE_STRIP);
//    for(double lat = -90; lat <= 90 - Settings::glCirclePointEach(); lat += Settings::glCirclePointEach()) {
//        for(double lon = -180; lon <= 180; lon += Settings::glCirclePointEach()) {
//            GLdouble dLat = lat;
//            GLdouble dLon = lon;
//            for(int tmu=0; tmu < 8; tmu++)
//                glMultiTexCoord2d(GL_TEXTURE0 + tmu, (dLon + 180.) / 360., (-dLat + 90.) / 180.);
//            glNormal3d(SX(lat, lon), SY(lat, lon), SZ(lat, lon));
//            VERTEX(dLat, dLon);
//            dLat = lat + Settings::glCirclePointEach();
//            for(int tmu=0; tmu < 8; tmu++)
//                glMultiTexCoord2d(GL_TEXTURE0 + tmu, (dLon + 180.) / 360., (-dLat + 90.) / 180.);
//            glNormal3d(SX(lat, lon), SY(lat, lon), SZ(lat, lon));
//            VERTEX(dLat, dLon);
//        }
//    }
//    glEnd();
    glEndList();

    // grid
    qDebug() << "GLWidget::createStaticLists() gridLines";
    _gridlinesList = glGenLists(1);
    glNewList(_gridlinesList, GL_COMPILE);
    if (Settings::gridLineStrength() > 0.0) {
        // meridians
        qglColor(Settings::gridLineColor());
        glLineWidth(Settings::gridLineStrength());
        for (int lon = 0; lon < 180; lon += Settings::earthGridEach()) {
            glBegin(GL_LINE_LOOP);
            for (int lat = 0; lat < 360; lat += Settings::glCirclePointEach())
                VERTEX(lat, lon);
            glEnd();
        }
        // parallels
        for (int lat = -90 + Settings::earthGridEach(); lat < 90; lat += Settings::earthGridEach()) {
            glBegin(GL_LINE_LOOP);
            for (int lon = -180; lon < 180;
                 lon += qCeil(Settings::glCirclePointEach() / qCos(lat * Pi180)))
                VERTEX(lat, lon);
            glEnd();
        }
    }
    glEndList();

    // coastlines
    qDebug() << "GLWidget::createStaticLists() coastLines";
    _coastlinesList = glGenLists(1);
    glNewList(_coastlinesList, GL_COMPILE);
    if (Settings::coastLineStrength() > 0.0) {
        qglColor(Settings::coastLineColor());
        glLineWidth(Settings::coastLineStrength());
        LineReader lineReader(Settings::dataDirectory("data/coastline.dat"));
        QList<QPair<double, double> > line = lineReader.readLine();
        while (!line.isEmpty()) {
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i < line.size(); i++)
                VERTEX(line[i].first, line[i].second);
            glEnd();
            line = lineReader.readLine();
        }
    }
    glEndList();

    // countries
    qDebug() << "GLWidget::createStaticLists() countries";
    _countriesList = glGenLists(1);
    glNewList(_countriesList, GL_COMPILE);
    if (Settings::countryLineStrength() > 0.0) {
        qglColor(Settings::countryLineColor());
        glLineWidth(Settings::countryLineStrength());
        LineReader countries = LineReader(Settings::dataDirectory("data/countries.dat"));
        QList<QPair<double, double> > line = countries.readLine();
        glBegin(GL_LINE);
        while (!line.isEmpty()) {
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i < line.size(); i++)
                VERTEX(line[i].first, line[i].second);
            glEnd();
            line = countries.readLine();
        }
        glEnd();
    }
    glEndList();

    // all waypoints (fixes + navaids)
    _fixesList = glGenLists(1);
    if(Settings::showAllWaypoints()) {
        qDebug() << "GLWidget::createStaticLists() allWaypoints";
        glNewList(_fixesList, GL_COMPILE);
        qglColor(Settings::waypointsDotColor());
        glLineWidth(Settings::countryLineStrength());
        double sin30 = .5; double cos30 = .8660254037;
        double tri_c = .01; double tri_a = tri_c * cos30; double tri_b = tri_c * sin30;
        glBegin(GL_TRIANGLES);
        foreach( Waypoint *w, Airac::instance()->allPoints) {
            if(w->type() == 1) {
                double circle_distort = qCos(w->lat * Pi180);
                double tri_b_c = tri_b * circle_distort;
                VERTEX(w->lat - tri_b_c, w->lon - tri_a);
                VERTEX(w->lat - tri_b_c, w->lon + tri_a);
                VERTEX(w->lat + tri_c * circle_distort, w->lon);
            }
        }
        glEnd();
        glEndList();
    }
}

void GLWidget::createStaticSectorLists(QList<Sector*> sectors) {
    //Polygon
    if (_staticSectorPolygonsList == 0)
            _staticSectorPolygonsList = glGenLists(1);

    // make sure all the lists are there to avoid nested glNewList calls
    foreach(Sector *sector, sectors) {
        if (sector != 0)
                sector->glPolygon();
    }

    // create a list of lists
    glNewList(_staticSectorPolygonsList, GL_COMPILE);
    foreach(Sector *sector, sectors) {
        if (sector != 0)
                glCallList(sector->glPolygon());
    }
    glEndList();


    // FIR borders
    if (_staticSectorPolygonBorderLinesList == 0)
            _staticSectorPolygonBorderLinesList = glGenLists(1);


    if (!_allSectorsDisplayed && Settings::firBorderLineStrength() > 0.) {
        // first, make sure all lists are there
        foreach(Sector *sector, sectors) {
            if (sector != 0)
                    sector->glBorderLine();
        }
        glNewList(_staticSectorPolygonBorderLinesList, GL_COMPILE);
        foreach(Sector *sector, sectors) {
            if(sector != 0)
                    glCallList(sector->glBorderLine());
        }
        glEndList();
    }
}

//////////////////////////////////////////
// initializeGL(), paintGL() & resizeGL()
//////////////////////////////////////////
/**
  gets called on instantiation by QGLWidget. The preferred place for anything that does not
  need to be done when just rotating the globe or on a Whazzup update.
  Sets up the whole OpenGL environment and prepares static artefacts for quick access later.
**/
void GLWidget::initializeGL() {
    qDebug() << "GLWidget::initializeGL()";
    qDebug() << "OpenGL support: " << context()->format().hasOpenGL()
            << "; 1.1:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_1_1)
            << "; 1.2:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_1_2)
            << "; 1.3:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_1_3) // multitexturing
            << "; 1.4:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_1_4)
            << "; 1.5:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_1_5)
            << "; 2.0:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_2_0)
            << "; 2.1:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_2_1)
            << "; 3.0:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_3_0)
            << "; 3.1:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_3_1)
            << "; 3.2:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_3_2)
            << "; 3.3:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_3_3)
            << "; 4.0:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_4_0);
    qDebug() << "GL_VENDOR:  " << reinterpret_cast<char const*> (glGetString(GL_VENDOR));
    qDebug() << "GL_RENDERER:" << reinterpret_cast<char const*> (glGetString(GL_RENDERER));
    qDebug() << "GL_VERSION: " << reinterpret_cast<char const*> (glGetString(GL_VERSION));
    qDebug() << "GL_SHADING_LANGUAGE_VERSION:"
             << reinterpret_cast<char const*> (glGetString(GL_SHADING_LANGUAGE_VERSION));
    qDebug() << "GL_EXTENSIONS:" << reinterpret_cast<char const*> (glGetString(GL_EXTENSIONS));
    qglClearColor(Settings::backgroundColor());

    if (Settings::glStippleLines())
        glEnable(GL_LINE_STIPPLE);
    else
        glDisable(GL_LINE_STIPPLE);
    if(Settings::displaySmoothDots()) {
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST); // GL_FASTEST, GL_NICEST, GL_DONT_CARE
    } else {
        glDisable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST); // GL_FASTEST, GL_NICEST, GL_DONT_CARE
    }
    if(Settings::displaySmoothLines()) {
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // GL_FASTEST, GL_NICEST, GL_DONT_CARE
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    } else {
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_POLYGON_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST); // GL_FASTEST, GL_NICEST, GL_DONT_CARE
        glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
    }
    if (Settings::glBlending()) {
        glEnable(GL_BLEND);
        //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // for texture blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // source,dest:
        // ...GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
        // ...GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR,
        // ...GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_SRC_ALPHA_SATURATE

        glEnable(GL_FOG); // fog - fading Earth's borders
        glFogi(GL_FOG_MODE, GL_LINEAR); // GL_EXP2, GL_EXP, GL_LINEAR
        GLfloat fogColor[] = {
            (GLfloat) Settings::backgroundColor().redF(),
            (GLfloat) Settings::backgroundColor().greenF(),
            (GLfloat) Settings::backgroundColor().blueF(),
            (GLfloat) Settings::backgroundColor().alphaF()
        };
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, 1.);
        glHint(GL_FOG_HINT, GL_DONT_CARE);
        glFogf(GL_FOG_START, 9.8);
        glFogf(GL_FOG_END, 10.);
    } else {
        glBlendFunc(GL_ONE, GL_ZERO);
        glDisable(GL_BLEND);
        glDisable(GL_FOG);
    }

    glDisable(GL_DEPTH_TEST); // this helps against sectors and coastlines that..
                                //are "farer" away than the earth superficie
                                // - also we do not need that. We just draw from far to near...
    //glDepthFunc(GL_LEQUAL); // when using DEPTH_TEST
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // 1st: GL_FOG_HINT, GL_GENERATE_MIPMAP_HINT,
    // ...GL_LINE_SMOOTH_HINT, GL_PERSPECTIVE_CORRECTION_HINT, GL_POINT_SMOOTH_HINT,
    // ...GL_POLYGON_SMOOTH_HINT, GL_TEXTURE_COMPRESSION_HINT, GL_FRAGMENT_SHADER_DERIVATIVE_HINT
    // ...2nd: GL_FASTEST, GL_NICEST, GL_DONT_CARE

    /* OpenGL lighting
    * AMBIENT - light that comes from all directions equally and is scattered in all directions equally by the polygons
        in your scene. This isn't quite true of the real world - but it's a good first approximation for light that comes
        pretty much uniformly from the sky and arrives onto a surface by bouncing off so many other surfaces that it might
        as well be uniform.
    * DIFFUSE - light that comes from a particular point source (like the Sun) and hits surfaces with an intensity
        that depends on whether they face towards the light or away from it. However, once the light radiates from the
        surface, it does so equally in all directions. It is diffuse lighting that best defines the shape of 3D objects.
    * SPECULAR - as with diffuse lighting, the light comes from a point souce, but with specular lighting, it is reflected
        more in the manner of a mirror where most of the light bounces off in a particular direction defined by the surface
        shape. Specular lighting is what produces the shiney highlights and helps us to distinguish between flat, dull
        surfaces such as plaster and shiney surfaces like polished plastics and metals.
    * EMISSION - in this case, the light is actually emitted by the polygon - equally in all directions.                  */

    if (Settings::glLighting()) {
        //const GLfloat earthAmbient[]  = {0, 0, 0, 1};
        const GLfloat earthDiffuse[]  = {1, 1, 1, 1};
        const GLfloat earthSpecular[] = {
            (GLfloat) Settings::specularColor().redF(),
            (GLfloat) Settings::specularColor().greenF(),
            (GLfloat) Settings::specularColor().blueF(),
            (GLfloat) Settings::specularColor().alphaF()
        };
        const GLfloat earthEmission[] = {0, 0, 0, 1};
        const GLfloat earthShininess[] = {(GLfloat) Settings::earthShininess()};
        //glMaterialfv(GL_FRONT, GL_AMBIENT, earthAmbient); // GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
        glMaterialfv(GL_FRONT, GL_DIFFUSE, earthDiffuse); // ...GL_EMISSION, GL_SHININESS, GL_AMBIENT_AND_DIFFUSE,
        glMaterialfv(GL_FRONT, GL_SPECULAR, earthSpecular); // ...GL_COLOR_INDEXES
        glMaterialfv(GL_FRONT, GL_EMISSION, earthEmission);   //
        glMaterialfv(GL_FRONT, GL_SHININESS, earthShininess); //... only DIFFUSE has an own alpha channel!
        glColorMaterial(GL_FRONT, GL_AMBIENT); // GL_EMISSION, GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR, GL_AMBIENT_AND_DIFFUSE
        glEnable(GL_COLOR_MATERIAL);    // controls if glColor will drive the given values in glColorMaterial

        const GLfloat sunAmbient[] = {0., 0., 0., 1.};
        QColor adjustSunDiffuse = Settings::sunLightColor();
        if (Settings::glLights() > 1)
            adjustSunDiffuse = adjustSunDiffuse.darker(100. * (Settings::glLights() - // reduce light intensity
                                                                                        // by number of lights...
                                                               Settings::glLightsSpread() / 180. *
                                                                (Settings::glLights() - 1))); // ...and increase again
                                                                                            // by their distribution
        const GLfloat sunDiffuse[] = {
            (GLfloat) adjustSunDiffuse.redF(),
            (GLfloat) adjustSunDiffuse.greenF(),
            (GLfloat) adjustSunDiffuse.blueF(),
            (GLfloat) adjustSunDiffuse.alphaF()
        };
        //const GLfloat sunSpecular[] = {1, 1, 1, 1}; // we drive this via material values
        for (int light = 0; light < 8; light++) {
            if (light < Settings::glLights()) {
                glLightfv(GL_LIGHT0 + light, GL_AMBIENT, sunAmbient); // GL_AMBIENT, GL_DIFFUSE,
                                                        // GL_SPECULAR, GL_POSITION, GL_SPOT_CUTOFF,
                glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, sunDiffuse); // ...GL_SPOT_DIRECTION, GL_SPOT_EXPONENT,
                                                                // GL_CONSTANT_ATTENUATION,
                //glLightfv(GL_LIGHT0 + light, GL_SPECULAR, sunSpecular);// ...GL_LINEAR_ATTENUATION
                                                                        // GL_QUADRATIC_ATTENUATION
                glEnable(GL_LIGHT0 + light);
            } else
                glDisable(GL_LIGHT0 + light);
        }
        const GLfloat modelAmbient[] = {.2, .2, .2, 1.}; // the "background" ambient light
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmbient); // GL_LIGHT_MODEL_AMBIENT, GL_LIGHT_MODEL_COLOR_CONTROL,
        //glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // ...GL_LIGHT_MODEL_LOCAL_VIEWER, GL_LIGHT_MODEL_TWO_SIDE

        glShadeModel(GL_SMOOTH); // SMOOTH or FLAT
        glEnable(GL_NORMALIZE);
        _lightsGenerated = true;
    }

    createStaticLists();
    qDebug() << "GLWidget::initializeGL() -- finished";
}

/**
  gets called whenever a screen refresh is needed. If you want to force a repaint,
  call update() (or updateGL(), when already initialized) instead which is the
  preferred method on a QGLWidget.
*/
void GLWidget::paintGL() {
    //qint64 started = QDateTime::currentMSecsSinceEpoch(); // for method execution time calculation.
                                    // See last line of method.
    //qDebug() << "GLWidget::paintGL()";

    // blank out the screen (buffered, of course)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0, 0, -10);
    glRotated(_xRot, 1, 0, 0);
    glRotated(_yRot, 0, 1, 0);
    glRotated(_zRot, 0, 0, 1);

    if (Settings::glLighting()) {
        if(!_lightsGenerated)
            createLights();

        glEnable(GL_LIGHTING);
        // moving sun's position
        QPair<double, double> zenith = sunZenith(Whazzup::instance()->whazzupData().whazzupTime.isValid()?
                                                 Whazzup::instance()->whazzupData().whazzupTime:
                                                 QDateTime::currentDateTimeUtc());
        GLfloat sunVertex0[] = {SX(zenith.first, zenith.second), SY(zenith.first, zenith.second),
                               SZ(zenith.first, zenith.second), 0}; // sun has parallel light -> dist=0
        glLightfv(GL_LIGHT0, GL_POSITION, sunVertex0); // light 0 always has the real (center) position
        if (Settings::glLights() > 1) {
            for (int light = 1; light < Settings::glLights(); light++) { // setting the other lights' positions
                double fraction = 2*M_PI / (Settings::glLights() - 1) * light;
                double spreadLat = zenith.first  + qSin(fraction) * Settings::glLightsSpread();
                double spreadLon = zenith.second + qCos(fraction) * Settings::glLightsSpread();
                GLfloat sunVertex[] = {SX(spreadLat, spreadLon), SY(spreadLat, spreadLon), SZ(spreadLat, spreadLon), 0};
                glLightfv(GL_LIGHT0 + light, GL_POSITION, sunVertex); // GL_LIGHTn is conveniently GL_LIGHT0 + n
            }
        }
    }
    if (Settings::glTextures() && _earthTex != 0 && Settings::glLighting()) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE
        glBindTexture(GL_TEXTURE_2D, _earthTex);
    }
    if (Settings::glTextures() && _earthTex != 0 && !Settings::glLighting()) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE
        glBindTexture(GL_TEXTURE_2D, _earthTex);
    }


    glCallList(_earthList);
    if (Settings::glLighting())
        glDisable(GL_LIGHTING); // disable lighting after drawing earth...

    if (Settings::glTextures() && _earthTex != 0) // disable textures after drawing earth...
        glDisable(GL_TEXTURE_2D);

    glCallList(_coastlinesList);
    glCallList(_countriesList);
    glCallList(_gridlinesList);
    if(Settings::showAllWaypoints() && _zoom < _allWaypointsLabelZoomTreshold * .7)
        glCallList(_fixesList);
    if(Settings::showUsedWaypoints() && _zoom < _usedWaypointsLabelZoomThreshold * .7)
        glCallList(_usedWaypointsList);

    //render sectors
    if(Settings::showCTR()) {
        glCallList(_sectorPolygonsList);
        glCallList(_sectorPolygonBorderLinesList);
    }

    //render hovered sectors
    if(_hoveredControllers.size() > 0) {
        glCallList(_hoveredSectorPolygonsList);
        glCallList(_hoveredSectorPolygonBorderLinesList);
    }

    //Static Sectors (for editing Sectordata)
    if(_renderStaticSectors) {
        glCallList(_staticSectorPolygonsList);
        glCallList(_staticSectorPolygonBorderLinesList);
    }

    QList<Airport*> airportList = NavData::instance()->airports.values();
    //render Approach
    if(Settings::showAPP()) {
        foreach(Airport *a, airportList) {
            if(!a->approaches.isEmpty())
                glCallList(a->appDisplayList());
        }
    }

    //render Tower
    if(Settings::showTWR()) {
        foreach(Airport *a, airportList) {
            if(!a->towers.isEmpty())
                glCallList(a->twrDisplayList());
        }
    }

    //render Ground/Delivery
    if(Settings::showGND()) {
        foreach(Airport *a, airportList) {
            if(!a->deliveries.isEmpty())
                glCallList(a->delDisplayList());
            if(!a->grounds.isEmpty())
                glCallList(a->gndDisplayList());
        }
    }


    if(Settings::showAirportCongestion())
            glCallList(_congestionsList);
    glCallList(_activeAirportsList);
    if(Settings::showInactiveAirports() && (_zoom < _inactiveAirportLabelZoomTreshold * .7))
            glCallList(_inactiveAirportsList);

    glCallList(_pilotsList);


    //Highlight friends
    if(Settings::highlightFriends()) {
        if(_highlighter == 0) createFriendHighlighter();
        QTime time = QTime::currentTime();
        double range = (time.second()%5);
        range += (time.msec()%500)/1000;

        GLfloat red = Settings::friendsHighlightColor().redF();
        GLfloat green = Settings::friendsHighlightColor().greenF();
        GLfloat blue = Settings::friendsHighlightColor().blueF();
        GLfloat alpha = Settings::friendsHighlightColor().alphaF();
        double lineWidth = Settings::highlightLineWidth();
        if(!Settings::useHighlightAnimation()) {
            range = 0;
            destroyFriendHightlighter();
        }

        foreach(const auto &_friend, _friends) {
            if (qFuzzyIsNull(_friend.first) && qFuzzyIsNull(_friend.second))
                continue;

            glBegin(GL_LINE_LOOP);
            glLineWidth(lineWidth);
            glColor4f(red, green, blue, alpha);
            GLdouble circle_distort = qCos(_friend.first * Pi180);
            for(int i = 0; i <= 360; i += 10) {
                double x = _friend.first  + Nm2Deg((80-(range*20))) * circle_distort * qCos(i * Pi180);
                double y = _friend.second + Nm2Deg((80-(range*20))) * qSin(i * Pi180);
                VERTEX(x, y);
            }
            glEnd();
        }
    }

    // render Wind
    if(Settings::showSonde()) {
        // show wind arrows of altitudes near the selected one
        for(quint8 span = 1; span <= Settings::sondeAltSecondarySpan_1k(); span++) {
            glCallList(SondeData::instance()->
                       windArrows(Settings::sondeAlt_1k() - span, true));
            glCallList(SondeData::instance()->
                       windArrows(Settings::sondeAlt_1k() + span, true));
        }
        glCallList(SondeData::instance()->windArrows(Settings::sondeAlt_1k()));
    }

    // render labels
    renderLabels();

    // selection rectangle
    if (_mapRectSelecting)
            drawSelectionRectangle();


    // some preparations to draw small textures on the globe (plane symbols, wind data...).
//    QPixmap planePm(":/icons/images/arrowup16.png");
//    GLuint planeTex = bindTexture(planePm, GL_TEXTURE_2D,
//                                  GL_RGBA, QGLContext::LinearFilteringBindOption); // QGLContext::MipmapBindOption
//    glEnable(GL_TEXTURE_2D);
//    glBindTexture(GL_TEXTURE_2D, planeTex);
//    glColor3f(1., 1., 1.);

//    for (double lat = -90.; lat <= 90.; lat += 45.) {
//        for (double lon = -180.; lon <= 180.; lon += 45.) {
//            glPushMatrix();
//            glTranslatef(SX(lat, lon), SY(lat, lon), SZ(lat, lon));
//            glRotatef(0, 1, 0, 0);
//            glRotatef(90, 0, 1, 0);
//            glRotatef(90, 0, 0, 1);

//            glBegin(GL_QUADS);
//            glTexCoord2i(0, 1);
//            glVertex2f(-.05,  .15);
//            glTexCoord2i(1, 1);
//            glVertex2f( .05,  .15);
//            glTexCoord2i(1, 0);
//            glVertex2f( .05, -.15);
//            glTexCoord2f(.4, 0);
//            glVertex2f(-.05, -.15);
//            glEnd();
//            glPopMatrix();
//        }
//    }
//    glDisable(GL_TEXTURE_2D);

//    drawCoordinateAxii(); // use this to see where the axii are (x = red, y = green, z = blue)


    glFlush(); // http://www.opengl.org/sdk/docs/man/xhtml/glFlush.xml

    // just for performance measurement:
    //qDebug() << "GLWidget::paintGL() -- finished in" << QDateTime::currentMSecsSinceEpoch() - started << "ms";
}

void GLWidget::resizeGL(int width, int height) {
    _aspectRatio = (double)width / (double)height;
    glViewport(0, 0, width, height);
    resetZoom();
}

////////////////////////////////////////////////////////////
// SLOTS: mouse, key... and general user-map interaction
//
void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    if(event->buttons().testFlag(Qt::RightButton) || // check before left button if useSelectionRectangle=off
            (!Settings::useSelectionRectangle() && event->buttons().testFlag(Qt::LeftButton))) { // rotate
        _mapMoving = true;
        handleRotation(event);
    } else if (event->buttons().testFlag(Qt::MiddleButton)) { // zoom
        _mapZooming = true;
        zoomIn((currentPos.x() - _lastPos.x() - currentPos.y() + _lastPos.y()) / 100. * Settings::zoomFactor());
        _lastPos = currentPos;
    } else if (event->buttons().testFlag(Qt::LeftButton)) { // selection rectangle
        _mapRectSelecting = true;
        updateGL();
    }


    double currLat, currLon;
    if (mouse2latlon(currentPos.x(), currentPos.y(), currLat, currLon)) {
        QSet<Controller*> _newHoveredControllers;
        foreach(Controller *c, Whazzup::instance()->whazzupData().controllersWithSectors().values()) {
            if (c->sector->containsPoint(QPointF(currLat, currLon))) {
                _newHoveredControllers.insert(c);
            }
        }
        if (_newHoveredControllers != _hoveredControllers) {
            _hoveredControllers = _newHoveredControllers;
            createHoveredControllersLists(_hoveredControllers);
            qDebug() << "hovered controllers" << _hoveredControllers;
            updateGL();
        }
    }
}

void GLWidget::mousePressEvent(QMouseEvent*) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    QToolTip::hideText();
    if (_mapMoving || _mapZooming || _mapRectSelecting) {
        _mapMoving = false;
        _mapZooming = false;
        _mapRectSelecting = false;
        updateGL();
    }
    if (!_mapRectSelecting)
        _lastPos = _mouseDownPos = currentPos;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    QToolTip::hideText();
    if (_mapMoving)
        _mapMoving = false;
    else if (_mapZooming)
        _mapZooming = false;
    else if (_mapRectSelecting) {
        _mapRectSelecting = false;
        if (currentPos != _mouseDownPos) {
            // moved more than 40px?
            if (
                ((currentPos.x() - _mouseDownPos.x()) * (currentPos.x() - _mouseDownPos.x()))
                + ((currentPos.y() - _mouseDownPos.y()) * (currentPos.y() - _mouseDownPos.y())) > 40 * 40
            ) {
                double downLat, downLon;
                if (mouse2latlon(_mouseDownPos.x(), _mouseDownPos.y(), downLat, downLon)) {
                    double currLat, currLon;
                    if (mouse2latlon(currentPos.x(), currentPos.y(), currLat, currLon)) {
                        DoublePair mid = NavData::greatCircleFraction(downLat, downLon, currLat, currLon, .5);
                        setMapPosition(mid.first, mid.second,
                                       qMax(NavData::distance(downLat, downLon,
                                                              downLat, currLon),
                                            NavData::distance(downLat, downLon,
                                                              currLat, downLon)) / 4000.);
                    }
                }
            }
        } else
            updateGL();
    } else if (_mouseDownPos == currentPos && event->button() == Qt::LeftButton) {
        // chasing a "click-spot" vertically offset problem on Win/nvidia
        double lat, lon;
        bool onGlobe = mouse2latlon(currentPos.x(), currentPos.y(), lat, lon);
        qDebug() << "GLWidget::mouseReleaseEvent left btn before objectsAt()"
                 << QString(
                      "widget[width=%5, height=%6], event[x=%1,"
                      " y=%2], mapFromGlobal(used value)[x=%10, y=%11], global[x=%8,y=%9]"
                      " onGlobe=%7, globe[lat=%3, lon=%4]"
                    )
                    .arg(event->x()).arg(event->y())
                    .arg(lat).arg(lon)
                    .arg(width()).arg(height())
                    .arg(onGlobe)
                    .arg(QCursor::pos().x())
                    .arg(QCursor::pos().y())
                    .arg(mapFromGlobal(QCursor::pos()).x())
                    .arg(mapFromGlobal(QCursor::pos()).y());

        QList<MapObject*> objects;
        foreach(MapObject* m, objectsAt(currentPos.x(), currentPos.y())) {
            if (dynamic_cast<Waypoint*>(m) != 0) // all but waypoints have a dialog
                continue;

            objects.append(m);
        }
        if (objects.isEmpty()) {
            clientSelection->clearObjects();
            clientSelection->close();
        } else if (objects.size() == 1)
            objects[0]->showDetailsDialog();
        else {
            clientSelection->move(QCursor::pos());
            clientSelection->setObjects(objects);
        }
    } else if (_mouseDownPos == currentPos && event->button() == Qt::RightButton)
        rightClick(currentPos);
    updateGL();
}

void GLWidget::rightClick(const QPoint& pos) {
    qDebug() << "GLWidget::rightClick()";
    QList<MapObject*> objects = objectsAt(pos.x(), pos.y());
    int countRelevant = 0;
    Pilot *pilot = 0;
    Airport *airport = 0;
    foreach(MapObject* m, objects) {
        if(dynamic_cast<Pilot*>(m) != 0) {
            pilot = dynamic_cast<Pilot*>(m);
            countRelevant++;
        }
        if(dynamic_cast<Airport*>(m) != 0) {
            airport = dynamic_cast<Airport*>(m);
            countRelevant++;
            break; // priorise airports
        }
    }
    if (countRelevant == 0) {
        GuiMessages::message("no object under cursor");
    } else if (countRelevant > 1) {
        GuiMessages::message("too many objects under cursor");
    } else if (airport != 0) {
        GuiMessages::message(QString("toggled routes for %1 [%2]").arg(airport->label).
                             arg(airport->showFlightLines? "off": "on"),
                             "routeToggleAirport");
        airport->showFlightLines = !airport->showFlightLines;
        if (AirportDetails::instance(false) != 0)
            AirportDetails::instance()->refresh();
        if (PilotDetails::instance(false) != 0) // can have an effect on the state of
            PilotDetails::instance()->refresh(); // ...PilotDetails::cbPlotRoutes
        createPilotsList();
        updateGL();
    } else if (pilot != 0) {
        // display flight path for pilot
        GuiMessages::message(QString("toggled route for %1 [%2]").arg(pilot->label).
                             arg(pilot->showDepDestLine? "off": "on"),
                             "routeTogglePilot");
        pilot->showDepDestLine = !pilot->showDepDestLine;
        if (PilotDetails::instance(false) != 0)
            PilotDetails::instance()->refresh();
        createPilotsList();
        updateGL();
    }
    qDebug() << "GLWidget::rightClick() -- finished";
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    QToolTip::hideText();
    if (event->buttons().testFlag(Qt::LeftButton)) {
        double lat, lon;
        if (mouse2latlon(currentPos.x(), currentPos.y(), lat, lon))
            setMapPosition(lat, lon, _zoom, false);
        zoomIn(.6);
    } else if (event->button() == Qt::RightButton) {
        double lat, lon;
        if (mouse2latlon(currentPos.x(), currentPos.y(), lat, lon))
            setMapPosition(lat, lon, _zoom, false);
        zoomIn(-.6);
    } else if (event->button() == Qt::MiddleButton)
        zoomTo(2.);
}

void GLWidget::wheelEvent(QWheelEvent* event) {
    QToolTip::hideText();
    //if(event->orientation() == Qt::Vertical) {
    if (qAbs(event->angleDelta().y()) > Settings::wheelMax()) // always recalibrate if bigger values are found
        Settings::setWheelMax(qAbs(event->angleDelta().y()));
    zoomIn((double) event->angleDelta().y() / Settings::wheelMax());
}

bool GLWidget::event(QEvent *event) {
    if(event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QList<MapObject*> objects = objectsAt(helpEvent->pos().x(), helpEvent->pos().y());
        if (objects.isEmpty())
            QToolTip::hideText();
        else {
            QString toolTip;
            for (int i = 0; i < objects.size(); i++) {
                if(i > 0) toolTip += "\n";
                toolTip += objects[i]->toolTip();
            }
            QToolTip::showText(helpEvent->globalPos(), toolTip);
        }
    }
    return QGLWidget::event(event);
}

void GLWidget::zoomIn(double factor) {
    _zoom -= _zoom * qMax(-.6, qMin(.6, .2 * factor * Settings::zoomFactor()));
    resetZoom();
    updateGL();
}

void GLWidget::zoomTo(double zoom) {
    this->_zoom = zoom;
    resetZoom();
    updateGL();
}



/////////////////////////////
// rendering text
/////////////////////////////

void GLWidget::renderLabels() {
    _fontRectangles.clear();
    _allFontRectangles.clear();

    // FIR labels
    QList<MapObject*> objects;
    foreach(Controller *c, Whazzup::instance()->whazzupData().controllers)
        if (c->sector != 0)
            objects.append(c);
    renderLabels(objects, Settings::firFont(), _controllerLabelZoomTreshold,
                 Settings::firFontColor());
    if(_allSectorsDisplayed) {
        qglColor(Settings::firFontColor());
        foreach (const Sector *sector, NavData::instance()->sectors) {
            QPair<double, double> center = sector->getCenter();
            double lat = center.first;
            double lon = center.second;
            renderText(SX(lat, lon), SY(lat, lon),
                       SZ(lat, lon), sector->icao, Settings::firFont());
        }
    }

    // planned route waypoint labels from Flightplan Dialog
    if(PlanFlightDialog::instance(false) != 0) {
        if(PlanFlightDialog::instance()->cbPlot->isChecked() &&
                PlanFlightDialog::instance()->selectedRoute != 0) {
            objects.clear();
            for (int i=1; i < PlanFlightDialog::instance()->
                       selectedRoute->waypoints.size() - 1; i++)
                objects.append(PlanFlightDialog::instance()->
                               selectedRoute->waypoints[i]);
            renderLabels(objects, Settings::waypointsFont(),
                         _usedWaypointsLabelZoomThreshold,
                         Settings::waypointsFontColor());
        }
    }

    // airport labels
    objects.clear();
    // ordered by congestion ascending,
    // big airport's labels will always be drawn first
    QList<Airport*> airportList = NavData::instance()->activeAirports.values();
    for(int i = airportList.size() - 1; i > -1; i--) { // from up to down
        //if(airportList[i] == 0) continue; // if we look carefully, we should be able to get rid of this
        if(airportList[i]->active)
            objects.append(airportList[i]);
    }
    renderLabels(objects, Settings::airportFont(), _activeAirportLabelZoomTreshold,
                 Settings::airportFontColor());

    // pilot labels
    QList<Pilot*> pilots = Whazzup::instance()->whazzupData().pilots.values();
    if(Settings::showPilotsLabels()) {
        objects.clear();
        for(int i = 0; i < pilots.size(); i++)
            if (pilots[i]->flightStatus() == Pilot::DEPARTING
                    || pilots[i]->flightStatus() == Pilot::EN_ROUTE
                    || pilots[i]->flightStatus() == Pilot::ARRIVING)
                objects.append(pilots[i]);
        renderLabels(objects, Settings::pilotFont(), _pilotLabelZoomTreshold,
                 Settings::pilotFontColor());
    }

    // temperatures and spreads
    if (Settings::showSonde()) {
        objects.clear();
        foreach(Station *s, SondeData::instance()->stationList)
                if (!s->mapLabel().isEmpty())
                        objects.append(s);
        renderLabels(objects, Settings::sondeFont(), _sondeLabelZoomTreshold,
                     Settings::windColor().lighter(), Settings::windColor().darker(300));
    }

    // waypoints used in shown routes
    if (Settings::showUsedWaypoints()) {
        // using a QSet 'cause it takes care of unique values
        QSet<MapObject*> waypointObjects;
        foreach(Pilot *p, pilots) {
            if (p->showDepLine() || p->showDestLine()) {
                QList<Waypoint*> waypoints = p->routeWaypoints();
                int next = p->nextPointOnRoute(waypoints);
                if (p->showDepLine())
                    for (int i = 0; i < next; i++)
                        waypointObjects.insert(waypoints[i]);
                if (p->showDestLine())
                    for (int i = next; i < waypoints.size(); i++)
                        waypointObjects.insert(waypoints[i]);
            }
        }
        renderLabels(waypointObjects.values(),
                     Settings::waypointsFont(), _usedWaypointsLabelZoomThreshold,
                     Settings::waypointsFontColor());
    }

    // inactive airports
    if(Settings::showInactiveAirports()) { // + inactive labels
        objects.clear();
        foreach(Airport *airport, NavData::instance()->airports.values())
            if (!airport->active)
                objects.append(airport);
        renderLabels(objects, Settings::inactiveAirportFont(), _inactiveAirportLabelZoomTreshold,
                     Settings::inactiveAirportFontColor());
    }

/*
    // all waypoints (fixes + navaids)
    QSet<MapObject*> tmp_points;
    foreach(Waypoint* wp, Airac::instance()->allPoints)
        tmp_points.insert(wp);

    Airac::instance()->allPoints.subtract(waypointObjects).toList()
    if(Settings::showAllWaypoints())
        renderLabels(tmp_points.subtract(waypointObjects).toList(), Settings::waypointsFont(),
                     allWaypointsLabelZoomTreshold, Settings::waypointsFontColor());
*/
}

void GLWidget::renderLabels(const QList<MapObject *> &objects, const QFont& font,
                            const double zoomTreshold, QColor color, QColor bgColor) {
    if (Settings::simpleLabels()) // cheap function
        renderLabelsSimple(objects, font, zoomTreshold, color, bgColor);
    else // expensive function
        renderLabelsComplex(objects, font, zoomTreshold, color, bgColor);
}

/**
 this one checks if labels overlap etc. - the transformation lat/lon -> x/y is very expensive
*/
void GLWidget::renderLabelsComplex(const QList<MapObject *> &objects, const QFont& font,
                            const double zoomTreshold, QColor color, QColor bgColor) {
    if(_zoom > zoomTreshold || color.alpha() == 0)
        return; // don't draw if too far away or color-alpha == 0

    // fade out
    color.setAlphaF(qMax(0., qMin(1., (zoomTreshold - _zoom) / zoomTreshold * 1.5))); // fade out

    // fade out: shadow
    if (bgColor.isValid())
        bgColor.setAlphaF(qMax(0., qMin(1., (zoomTreshold - _zoom) / zoomTreshold * 1.5))); // fade out

    QFontMetricsF fontMetrics(font, this);
    foreach(MapObject *o, objects) {
        if (_fontRectangles.size() >= Settings::maxLabels())
            break;
        if (!o->drawLabel)
            continue;
        int x, y; if (isPointVisible(o->lat, o->lon, &x, &y)) {
            const QString &text = o->mapLabel();
            QRectF rect = fontMetrics.boundingRect(text);
            int drawX = x - rect.width() / 2; // center horizontally
            int drawY = y - rect.height() - 5; // some px above dot
            rect.moveTo(drawX, drawY);

            QList<QRectF> rects; // possible positions, with preferred ones first
            rects << rect;
            rects << rect.translated(0,  rect.height() / 1.5);
            rects << rect.translated(0, -rect.height() / 1.5);
            rects << rect.translated( rect.width() / 1.5, 0);
            rects << rect.translated(-rect.width() / 1.5, 0);
            rects << rect.translated( rect.width() / 1.5,  rect.height() / 1.5 + 5);
            rects << rect.translated( rect.width() / 1.5, -rect.height() / 1.5);
            rects << rect.translated(-rect.width() / 1.5,  rect.height() / 1.5 + 5);
            rects << rect.translated(-rect.width() / 1.5, -rect.height() / 1.5);

            FontRectangle *drawnFontRect = 0;
            qglColor(color);
            foreach(const QRectF &r, rects) {
                if(shouldDrawLabel(r)) {
                    drawnFontRect = new FontRectangle(r, o);
                    // shadow: changing colors is expensive, could be made better
                    if (bgColor.isValid()) {
                        qglColor(bgColor);
                        renderText(r.left() + 1, (r.top() + r.height()) + 1, text, font);
                        qglColor(color);
                    }

                    // yes, this is slow and it is known: ..
                    // https://bugreports.qt-project.org/browse/QTBUG-844
                    // this is why we have the 'simple labels' option =>
                    // renderLabelsSimple()
                    renderText(r.left(), (r.top() + r.height()), text, font);
                    _fontRectangles.insert(drawnFontRect);
                    _allFontRectangles.insert(drawnFontRect);
                    break;
                }
            }
            if (drawnFontRect == 0) // default position if it was not drawn
                _allFontRectangles.insert(new FontRectangle(rect, o));
        }
    }
}

/**
  this one uses 3D-coordinates to paint and does not check overlap
  which results in tremendously improved framerates
*/
void GLWidget::renderLabelsSimple(const QList<MapObject *> &objects, const QFont& font,
                            const double zoomTreshold, QColor color, QColor bgColor) {
    if (_zoom > zoomTreshold || color.alpha() == 0)
        return; // don't draw if too far away or color-alpha == 0

    // fade out
    color.setAlphaF(qMax(0., qMin(1., (zoomTreshold - _zoom) / zoomTreshold * 1.5))); // fade out

    // fade out: shadow
    if (bgColor.isValid())
        bgColor.setAlphaF(qMax(0., qMin(1., (zoomTreshold - _zoom) / zoomTreshold * 1.5))); // fade out

    qglColor(color);
    foreach(MapObject *o, objects) {
        if (_fontRectangles.size() >= Settings::maxLabels())
            break;
        if (!o->drawLabel)
            continue;
        _fontRectangles.insert(new FontRectangle(QRectF(), 0)); // we use..
                        // this bogus value to stay compatible..
                        // with maxLabels-checking
        // shadow: changing colors is expensive, could be made better
        if (bgColor.isValid()) {
            qglColor(bgColor);
            renderText(SXhigh(o->lat - .08 * _zoom, o->lon + .08 * _zoom),
                       SYhigh(o->lat - .08 * _zoom, o->lon + .08 * _zoom),
                       SZhigh(o->lat - .08 * _zoom, o->lon + .08 * _zoom),
                       o->mapLabel(), font);
            qglColor(color);
        }
        // fast text rendering in the 3D space
        renderText(SXhigh(o->lat, o->lon), SYhigh(o->lat, o->lon), SZhigh(o->lat, o->lon),
                   o->mapLabel(), font);
    }
}

bool GLWidget::shouldDrawLabel(const QRectF &rect) {
    foreach(const FontRectangle *fr, _fontRectangles) {
        QRectF checkRect = fr->rect;
        checkRect.setWidth(checkRect.width()   / 1.6); // make them smaller to allow a tiny bit of intersect
        checkRect.setHeight(checkRect.height() / 1.6);
        checkRect.moveCenter(fr->rect.center());
        if(rect.intersects(checkRect))
            return false;
    }
    return true;
}

QList<MapObject*> GLWidget::objectsAt(int x, int y, double radius) const {
    QList<MapObject*> result;
    foreach(const FontRectangle *fr, _allFontRectangles) // scan text labels
        if(fr->rect.contains(x, y))
            result.append(fr->object);

    double lat, lon;
    if(!mouse2latlon(x, y, lat, lon)) // returns false if not on globe
        return result;

    double radiusDegQuad = Nm2Deg((qFuzzyIsNull(radius)? 30. * _zoom: radius));
    radiusDegQuad *= radiusDegQuad;

    foreach(Airport* a, NavData::instance()->airports.values()) {
        if(a->active) {
            double x = a->lat - lat;
            double y = a->lon - lon;
            if(x*x + y*y < radiusDegQuad) {
                result.removeAll(a);
                result.append(a);
            }
        }
    }

    foreach(Pilot *p, Whazzup::instance()->whazzupData().pilots.values()) {
        double x = p->lat - lat;
        double y = p->lon - lon;
        if(x*x + y*y < radiusDegQuad) {
            result.removeAll(p);
            result.append(p);
        }
    }

    foreach(Controller *c, Whazzup::instance()->whazzupData().controllers.values()) {
        if (c->sector != 0) {
            if (c->sector->containsPoint(QPointF(lat, lon))) {
                result.removeAll(c);
                result.append(c);
            }
        }
    }
    return result;
}

/////////////////////////
// draw-helper functions
/////////////////////////

void GLWidget::drawSelectionRectangle() {
    QPoint current = mapFromGlobal(QCursor::pos());
    double downLat, downLon;
    if (mouse2latlon(_mouseDownPos.x(), _mouseDownPos.y(), downLat, downLon)) {
        double currLat, currLon;
        if (mouse2latlon(current.x(), current.y(), currLat, currLon)) {
            // calculate a rectangle: approximating what the viewport will look after zoom
            // (far from perfect but an okayish approximation...)
            // down...: where the mouse was pressed down (1 edge of the rectangle)
            // curr...: where it is now (the opposite edge)
            double currLonDist = NavData::distance(currLat, downLon, currLat, currLon);
            double downLonDist = NavData::distance(downLat, downLon, downLat, currLon);
            double avgLonDist = (downLonDist + currLonDist) / 2.; // this needs to be the side length
            DoublePair downLatCurrLon = NavData::greatCircleFraction(downLat, downLon,
                                                                     downLat, currLon,
                                                                     avgLonDist / downLonDist);
            DoublePair currLatDownLon = NavData::greatCircleFraction(currLat, currLon,
                                                                     currLat, downLon,
                                                                     avgLonDist / currLonDist);
            QList<QPair<double, double> > points;
            points.append(DoublePair(downLat, downLon));
            points.append(DoublePair(downLatCurrLon.first, downLatCurrLon.second));
            points.append(DoublePair(currLat, currLon));
            points.append(DoublePair(currLatDownLon.first, currLatDownLon.second));
            points.append(DoublePair(downLat, downLon));

            // draw background
            glColor4f(0., 1., 1., .2);
            glBegin(GL_POLYGON);
            NavData::plotPointsOnEarth(points);
            glEnd();
            // draw rectangle
            glLineWidth(2.);
            glColor4f(0., 1., 1., .5);
            glBegin(GL_LINE_LOOP);
            NavData::plotPointsOnEarth(points);
            glEnd();
            // draw great circle course line
            glLineWidth(2.);
            glColor4f(0., 1., 1., .2);
            glBegin(GL_LINE_STRIP);
            NavData::plotPointsOnEarth(QList<QPair<double, double> >() << points[0] << points[2]);
            glEnd();

            // information labels
            const QFont font = QFont(); //Settings::firFont();
            const QFontMetricsF fontMetrics(font, this);

            // show position label
            const QString currText = QString("%1%2 %3%4").
                    arg(currLat > 0? "N": "S").
                    arg(qAbs(currLat), 5, 'f', 2, '0').
                    arg(currLon > 0? "E": "W").
                    arg(qAbs(currLon), 6, 'f', 2, '0');
            int x, y;
            if (isPointVisible(currLat, currLon, &x, &y)) {
                glColor4f(0., 0., 0., .7);
                renderText(
                        x + 21,
                        y + 21,
                        currText, font
                );
                glColor4f(0., 1., 1., 1.);
                renderText(
                        x + 20,
                        y + 20,
                        currText, font
                );
            }

            // show distance label
            const DoublePair middle = NavData::greatCircleFraction(downLat, downLon,
                                                             currLat, currLon,
                                                             .5);
            const QString middleText = QString("%1 NM / TC %2deg").arg(
                NavData::distance(downLat, downLon, currLat, currLon),
                0, 'f', 1
            ).arg(
                NavData::courseTo(downLat, downLon, currLat, currLon),
                3, 'f', 0, '0'
            );
            QRectF rect = fontMetrics.boundingRect(middleText);
            if (isPointVisible(middle.first, middle.second, &x, &y)) {
                glColor4f(0., 0., 0., .7);
                renderText(
                        x - rect.width() / 2. + 1,
                        y + rect.height() / 3. + 1,
                        middleText, font
                );
                glColor4f(0., 1., 1., 1.);
                renderText(
                        x - rect.width() / 2.,
                        y + rect.height() / 3.,
                        middleText, font
                );
            }
        }
    }
}

/**
  just for debugging. Visualization of the axii. red=x/green=y/blue=z
*/
void GLWidget::drawCoordinateAxii() const {
        glPushMatrix(); glLoadIdentity();
        glTranslatef(0, 0, -9);
        glRotated(_xRot, 1, 0, 0); glRotated(_yRot, 0, 1, 0); glRotated(_zRot, 0, 0, 1);
        GLUquadricObj *q = gluNewQuadric(); gluQuadricNormals(q, GLU_SMOOTH);

        glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);

        glColor3f(0, 0, 0); gluSphere(q, 0.02, 64, 32); // center
        glRotatef(90, 0, 1, 0);

        glColor3f(1, 0, 0); gluCylinder(q, 0.02, 0.0, 0.3, 64, 1); // x-axis
        glRotatef(90, 0, -1, 0);
        glRotatef(90, -1, 0, 0);
        glColor3f(0, 1, 0); gluCylinder(q, 0.02, 0.0, 0.3, 64, 1); // y-axis
        glRotatef(90, 1, 0, 0);
        glColor3f(0, 0, 1); gluCylinder(q, 0.02, 0.0, 0.3, 64, 1); // z-axis

    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glPopMatrix();
}


/////////////////////////
// uncategorized
/////////////////////////

void GLWidget::newWhazzupData(bool isNew) {
    qDebug() << "GLWidget::newWhazzupData() isNew =" << isNew;
    if(isNew) {
        // update airports
        NavData::instance()->updateData(Whazzup::instance()->whazzupData());

        _sectorsToDraw = Whazzup::instance()->whazzupData().controllersWithSectors();

        createPilotsList();
        createAirportsList();
        createControllersLists();
        _friends = Whazzup::instance()->whazzupData().friendsLatLon();

        updateGL();
    }
    qDebug() << "GLWidget::newWhazzupData -- finished";
}

void GLWidget::displayAllSectors(bool value) {
    _allSectorsDisplayed = value;
    newWhazzupData(true);
}

void GLWidget::showInactiveAirports(bool value) {
    Settings::setShowInactiveAirports(value);
    newWhazzupData(true);
}

void GLWidget::createFriendHighlighter() {
    _highlighter = new QTimer(this);
    _highlighter->setInterval(100);
    connect(_highlighter, SIGNAL(timeout()), this, SLOT(updateGL()));
    _highlighter->start();
}

void GLWidget::destroyFriendHightlighter() {
    if(_highlighter == 0) return;
    if(_highlighter->isActive()) _highlighter->stop();
    disconnect(_highlighter, SIGNAL(timeout()), this, SLOT(updateGL()));
    delete _highlighter;
    _highlighter = 0;
}


//////////////////////////////////
// Clouds, Lightning and Earth Textures
//////////////////////////////////

void GLWidget::useClouds() {
    parseEarthClouds();
}

void GLWidget::parseEarthClouds() {
    qDebug() << "GLWidget::parseEarthClouds()";
    // how hourglass during CPU-intensive operation
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    GuiMessages::progress("textures", "Preparing textures...");

    QImage earthTexIm, cloudsIm;

    //Check if clouds available
    QString cloudsTexFile =
            Settings::dataDirectory("textures/clouds/clouds.jpg");
    QFileInfo cloudsTexFI(cloudsTexFile);
    if(Settings::showClouds() && cloudsTexFI.exists()) {
        qDebug() << "GLWidget::parseEarthClouds() loading cloud texture";
        GuiMessages::progress("textures", "Preparing textures: loading clouds...");
        cloudsIm = QImage(cloudsTexFile);
    }

    if(Settings::glTextures()) {
        QString earthTexFile = Settings::dataDirectory(
                    QString("textures/%1").arg(Settings::glTextureEarth()));
        qDebug() << "GLWidget::parseEarthClouds() loading earth texture";
        GuiMessages::progress("textures", "Preparing textures: loading earth...");
        earthTexIm.load(earthTexFile);
    } else {
        qDebug() << "GLWidget::parseEarthClouds() finishing using clouds";
        _completedEarthIm = cloudsIm;
    }

    if((!Settings::showClouds() || cloudsIm.isNull()) && Settings::glTextures()) {
        qDebug() << "GLWidget::parseEarthClouds() finishing using earthTex"
                 << "(clouds deactivated or cloudsIm.isNull())";
        _completedEarthIm = earthTexIm;
    }

    if(!cloudsIm.isNull() && !earthTexIm.isNull() && Settings::showClouds()) {
        //transform so same size, take the bigger image
//        int width = cloudsIm.width();
//        int height = cloudsIm.height();

        // this always scales clouds to earth texture size
        GuiMessages::progress("textures", "Preparing textures: scaling clouds...");
        qDebug() << "GLWidget::parseEarthClouds() scaling cloud image from"
                 << cloudsIm.size() << "to" << earthTexIm.size();
        cloudsIm = cloudsIm.scaled(earthTexIm.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//        if(earthTexIm.width() > width) {
//            qDebug() << "GLWidget::parseEarthClouds upscaling cloud image horizontally";
//            cloudsIm = cloudsIm.scaledToWidth(earthTexIm.width(), Qt::SmoothTransformation);
//            width = earthTexIm.width();
//        } else {
//            qDebug() << "GLWidget::parseEarthClouds downscaling cloud image horizontally";
//            earthTexIm = earthTexIm.scaledToWidth(width, Qt::SmoothTransformation);
//        }

//        if(earthTexIm.height() > height) {
//            qDebug() << "GLWidget::parseEarthClouds upscaling cloud image vertically";
//            cloudsIm = cloudsIm.scaledToHeight(earthTexIm.height(),
//                                               Qt::SmoothTransformation);
//            height = earthTexIm.height();
//        } else {
//            qDebug() << "GLWidget::parseEarthClouds downscaling cloud image vertically";
//            earthTexIm = earthTexIm.scaledToHeight(height, Qt::SmoothTransformation);
//        }

        _completedEarthIm = earthTexIm;
        GuiMessages::progress("textures", "Preparing textures: converting texture...");
        qDebug() << "GLWidget::parseEarthClouds() converting image to ARGB32";
        _completedEarthIm =
                _completedEarthIm.convertToFormat(QImage::Format_ARGB32);

        qDebug() << "GLWidget::parseEarthClouds() combining earth+clouds";
        GuiMessages::progress("textures", "Preparing textures: combining textures...");
        QPainter painter(&_completedEarthIm);
        painter.setCompositionMode(QPainter::CompositionMode_Screen);
                // more modes available:
                // QPainter::CompositionMode_Screen
                // QPainter::CompositionMode_ColorDodge
                // QPainter::CompositionMode_Plus
                // QPainter::CompositionMode_Lighten
                // QPainter::CompositionMode_HardLight
                // QPainter::CompositionMode_Exclusion
                // QPainter::CompositionMode_Multiply
        painter.drawImage(0, 0, cloudsIm);
        painter.end();
    }

    if (_completedEarthIm.isNull())
        qWarning() << "Unable to load texture file: "
                   << Settings::dataDirectory(
                          QString("textures/%1").arg(Settings::glTextureEarth()));
    else {
        GLint max_texture_size;  glGetIntegerv(GL_MAX_TEXTURE_SIZE,  &max_texture_size);
        qDebug() << "OpenGL reported MAX_TEXTURE_SIZE as" << max_texture_size;

        // multitexturing units, if we need it once (headers in GL/glext.h, on Windows not available ?!)
        //GLint max_texture_units; glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);
        //qDebug() << "OpenGL reported MAX_TEXTURE_UNITS as" << max_texture_units;
        qDebug() << "Binding parsed texture as" << _completedEarthIm.width()
                 << "x" << _completedEarthIm.height() << "px texture";
        qDebug() << "Generating texture coordinates";
        GuiMessages::progress("textures", "Preparing textures: preparing texture coordinates...");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glGenTextures(1, &earthTex); // bindTexture does this the Qt'ish way already
        qDebug() << "Emptying error buffer";
        glGetError(); // empty the error buffer
        qDebug() << "Binding texture";
        _earthTex = bindTexture(_completedEarthIm, GL_TEXTURE_2D, GL_RGBA,
                               QGLContext::LinearFilteringBindOption); // QGLContext::MipmapBindOption
        if (GLenum glError = glGetError())
            qCritical() << QString("OpenGL returned an error (0x%1)")
                           .arg((int) glError, 4, 16, QChar('0'));
    }
    qDebug() << "GLWidget::parseEarthClouds() finished";
    update();
    qApp->restoreOverrideCursor();
    GuiMessages::remove("textures");
}

void GLWidget::createLights() {
    //const GLfloat earthAmbient[]  = {0, 0, 0, 1};
    const GLfloat earthDiffuse[]  = {1, 1, 1, 1};
    const GLfloat earthSpecular[] = {
        (GLfloat) Settings::specularColor().redF(),
        (GLfloat) Settings::specularColor().greenF(),
        (GLfloat) Settings::specularColor().blueF(),
        (GLfloat) Settings::specularColor().alphaF()
    };
    const GLfloat earthEmission[] = {0, 0, 0, 1};
    const GLfloat earthShininess[] = {(GLfloat) Settings::earthShininess()};
    //glMaterialfv(GL_FRONT, GL_AMBIENT, earthAmbient); // GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
    glMaterialfv(GL_FRONT, GL_DIFFUSE, earthDiffuse); // ...GL_EMISSION, GL_SHININESS, GL_AMBIENT_AND_DIFFUSE,
    glMaterialfv(GL_FRONT, GL_SPECULAR, earthSpecular); // ...GL_COLOR_INDEXES
    glMaterialfv(GL_FRONT, GL_EMISSION, earthEmission);   //
    glMaterialfv(GL_FRONT, GL_SHININESS, earthShininess); //... only DIFFUSE has an own alpha channel!
    glColorMaterial(GL_FRONT, GL_AMBIENT); // GL_EMISSION, GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR, GL_AMBIENT_AND_DIFFUSE
    glEnable(GL_COLOR_MATERIAL);    // controls if glColor will drive the given values in glColorMaterial

    const GLfloat sunAmbient[] = {0., 0., 0., 1.};
    QColor adjustSunDiffuse = Settings::sunLightColor();
    if (Settings::glLights() > 1)
        adjustSunDiffuse = adjustSunDiffuse.darker(100. * (Settings::glLights() - // reduce light intensity by number of lights...
                                                           Settings::glLightsSpread() / 180. * (Settings::glLights() - 1))); // ...and increase again by their distribution
    const GLfloat sunDiffuse[] = {
        (GLfloat) adjustSunDiffuse.redF(),
        (GLfloat) adjustSunDiffuse.greenF(),
        (GLfloat) adjustSunDiffuse.blueF(),
        (GLfloat) adjustSunDiffuse.alphaF()
    };
    //const GLfloat sunSpecular[] = {1, 1, 1, 1}; // we drive this via material values
    for (int light = 0; light < 8; light++) {
        if (light < Settings::glLights()) {
            glLightfv(GL_LIGHT0 + light, GL_AMBIENT, sunAmbient); // GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR, GL_POSITION, GL_SPOT_CUTOFF,
            glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, sunDiffuse); // ...GL_SPOT_DIRECTION, GL_SPOT_EXPONENT, GL_CONSTANT_ATTENUATION,
            //glLightfv(GL_LIGHT0 + light, GL_SPECULAR, sunSpecular);// ...GL_LINEAR_ATTENUATION GL_QUADRATIC_ATTENUATION
            glEnable(GL_LIGHT0 + light);
        } else
            glDisable(GL_LIGHT0 + light);
    }
    const GLfloat modelAmbient[] = {.2, .2, .2, 1.}; // the "background" ambient light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmbient); // GL_LIGHT_MODEL_AMBIENT, GL_LIGHT_MODEL_COLOR_CONTROL,
    //glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // ...GL_LIGHT_MODEL_LOCAL_VIEWER, GL_LIGHT_MODEL_TWO_SIDE

    glShadeModel(GL_SMOOTH); // SMOOTH or FLAT
    glEnable(GL_NORMALIZE);
    _lightsGenerated = true;
}
