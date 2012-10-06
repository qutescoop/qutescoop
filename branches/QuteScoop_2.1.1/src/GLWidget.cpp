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
//#include <GL/glext.h>   Multitexturing

GLWidget::GLWidget(QGLFormat fmt, QWidget *parent) :
        QGLWidget(fmt, parent),
        xRot(0), yRot(0), zRot(0), zoom(2), aspectRatio(1), earthTex(0),
        earthList(0), coastlinesList(0), countriesList(0), gridlinesList(0),
        pilotsList(0), activeAirportsList(0), inactiveAirportsList(0), congestionsList(0), FixesList(0),
        usedWaypointsList(0), sectorPolygonsList(0), sectorPolygonBorderLinesList(0),
        appBorderLinesList(0), windList(0),
        pilotLabelZoomTreshold(.9), activeAirportLabelZoomTreshold(1.2), inactiveAirportLabelZoomTreshold(.15),
        controllerLabelZoomTreshold(2.), allWaypointsLabelZoomTreshold(.1), usedWaypointsLabelZoomThreshold(1.2),
        allSectorsDisplayed(false),
        mapIsMoving(false), mapIsZooming(false), mapIsRectangleSelecting(false)
{
    setAutoFillBackground(false);
    setMouseTracking(true);
    lightsGenerated = false;
    cloudsAvaliable = false;
    highlighter = 0;
    // call default (=1) map position
    Settings::getRememberedMapPosition(&xRot, &yRot, &zRot, &zoom, 1);
    xRot = modPositive(xRot, 360.);
    yRot = modPositive(yRot, 360.);
    zRot = modPositive(zRot, 360.);
    resetZoom();
    emit newPosition();

    clientSelection = new ClientSelectionWidget();


}

GLWidget::~GLWidget() {
    makeCurrent();
    glDeleteLists(earthList, 1); glDeleteLists(gridlinesList, 1);
    glDeleteLists(coastlinesList, 1); glDeleteLists(countriesList, 1); glDeleteLists(FixesList, 1);
    glDeleteLists(usedWaypointsList, 1); glDeleteLists(pilotsList, 1);
    glDeleteLists(activeAirportsList, 1); glDeleteLists(inactiveAirportsList, 1); glDeleteLists(congestionsList, 1);
    glDeleteLists(appBorderLinesList, 1); glDeleteLists(windList, 1);
    glDeleteLists(sectorPolygonsList, 1); glDeleteLists(sectorPolygonBorderLinesList, 1);

    if (earthTex != 0)
        deleteTexture(earthTex);
        //glDeleteTextures(1, &earthTex); // handled Qt'ish by deleteTexture
    gluDeleteQuadric(earthQuad);

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
    xRot = modPositive(270. - lat, 360.);
    zRot = modPositive(     - lon, 360.);
    zoom = newZoom;
    resetZoom();
    if (updateGL)
        this->updateGL();
    emit newPosition();
}

QPair<double, double> GLWidget::currentPosition() const { // current rotation in lat/lon
    return QPair<double, double>(modPositive(-90. - xRot + 180., 360.) - 180.,
                                 modPositive(     - zRot + 180., 360.) - 180.);
}

void GLWidget::handleRotation(QMouseEvent *event) {
    const double zoomFactor = zoom / 10.;
    double dx = ( event->x() - lastPos.x()) * zoomFactor;
    double dy = (-event->y() + lastPos.y()) * zoomFactor;
    xRot = modPositive(xRot + dy + 180., 360.) - 180.;
    zRot = modPositive(zRot + dx + 180., 360.) - 180.;
    updateGL();
    lastPos = event->pos();
    emit newPosition();
}

bool GLWidget::mouseOnGlobe(int x, int y) const {
    double xGl = (2. * x / width()  - 1.) * aspectRatio * zoom / 2;
    double zGl = (2. * y / height() - 1.) * zoom / 2;
    double yGl = sqrt(1 - (xGl*xGl) - (zGl*zGl)); // As the radius of globe is 1
    return !qIsNaN(yGl);
}

bool GLWidget::mouse2latlon(int x, int y, double &lat, double &lon) const {
    // Converts screen mouse coordinates into latitude/longitude of the map.
    // returns false if x/y is not on the globe
    // Basis: Euler angles.
    // 1) mouse coordinates to Cartesian coordinates of the openGL environment [-1...+1]
    double xGl = (2. * x / width()  - 1.) * aspectRatio * zoom / 2;
    double zGl = (2. * y / height() - 1.) * zoom / 2;
    double yGl = sqrt(1 - (xGl*xGl) - (zGl*zGl)); // As the radius of globe is 1
    if(qIsNaN(yGl))
        return false; // mouse is not on globe

    // 2) skew (rotation around the x-axis, where 0° means looking onto the equator)
    double theta = (xRot + 90.) * Pi180;

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

    lon = modPositive(lon - zRot + 180., 360.) - 180.;
    return true;
}

void GLWidget::scrollBy(int moveByX, int moveByY) {
    QPair<double, double> cur = currentPosition();
    setMapPosition(cur.first  - (double) moveByY * zoom * 6., // 6° on zoom=1
                   cur.second + (double) moveByX * zoom * 6., zoom);
    updateGL();
}

void GLWidget::resetZoom() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5 * zoom * aspectRatio, +0.5 * zoom * aspectRatio, // clipping left/right/bottom/top/near/far
            +0.5 * zoom, -0.5 * zoom, 8, 10); // or gluPerspective for perspective viewing
    //gluPerspective(zoom, aspectRatio, 8, 10); // just for reference, if you want to try it
    glMatrixMode(GL_MODELVIEW);
}

bool GLWidget::pointIsVisible(double lat, double lon, int *px, int *py) const {
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
    GuiMessages::message(QString("Remembered position %1").arg(nr));
    Settings::setRememberedMapPosition(xRot, yRot, zRot, zoom, nr);
}

void GLWidget::restorePosition(int nr) {
    Settings::getRememberedMapPosition(&xRot, &yRot, &zRot, &zoom, nr);
    xRot = modPositive(xRot, 360.);
    zRot = modPositive(zRot, 360.);
    resetZoom();
    updateGL();
    emit newPosition();
}

const QPair<double, double> GLWidget::sunZenith(const QDateTime &dateTime) const {
    // dirtily approximating present zenith Lat/Lon (where the sun is directly above).
    // scientific solution: http://openmap.bbn.com/svn/openmap/trunk/src/openmap/com/bbn/openmap/layer/daynight/SunPosition.java
    // [sunPosition()] - that would have been at least 100 lines of code...
    return QPair<double, double>(-23. * qCos((double) dateTime.date().dayOfYear() / (double)dateTime.date().daysInYear() * 2.*M_PI),
                                 -((double) dateTime.time().hour() + (double) dateTime.time().minute() / 60.) * 15. - 180.);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Methods preparing displayLists
//
void GLWidget::createPilotsList() {
    qDebug() << "GLWidget::createPilotsList() ";
    makeCurrent();

    if(pilotsList == 0)
        pilotsList = glGenLists(1);

    glNewList(pilotsList, GL_COMPILE);

    QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().pilots.values();

    // aircraft dots
    if (Settings::pilotDotSize() > 0.) {
        glPointSize(Settings::pilotDotSize());
        glBegin(GL_POINTS);
        qglColor(Settings::pilotDotColor());
        foreach(const Pilot *p, pilots)
            if (!qFuzzyIsNull(p->lat) || !qFuzzyIsNull(p->lon))
                VERTEX(p->lat, p->lon);
        glEnd();
    }

    // timelines / leader lines
    if(Settings::timelineSeconds() > 0 && Settings::timeLineStrength() > 0.) {
        glLineWidth(Settings::timeLineStrength());
        glBegin(GL_LINES);
        qglColor(Settings::timeLineColor());
        foreach(const Pilot *p, pilots) {
            if (p->groundspeed > 30 && (!qFuzzyIsNull(p->lat) || !qFuzzyIsNull(p->lon))) {
                VERTEX(p->lat, p->lon);
                QPair<double, double> pos = p->positionInFuture(Settings::timelineSeconds());
                VERTEX(pos.first, pos.second);
            }
        }
        glEnd();
    }


    // flight paths, also for booked flights
    foreach(Pilot *p, Whazzup::getInstance()->whazzupData().allPilots()) {
        if (p->showDepLine() || p->showDestLine()) {
            QList<Waypoint*> waypoints = p->routeWaypointsWithDepDest();
            int next = p->nextPointOnRoute(waypoints);
            QList<DoublePair> points; // these are the points that really get drawn
            if (p->showDepLine()) // Dep -> plane
                for (int i = 0; i < next; i++)
                    points.append(DoublePair(waypoints[i]->lat, waypoints[i]->lon));
            if (!qFuzzyIsNull(p->lat) && !qFuzzyIsNull(p->lon)) { // plane ok: draw to plane and reset list for DestLine
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
            }
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
    }

    // planned route from Flightplan Dialog (does not really belong to pilots lists, but is convenient here)
    if(PlanFlightDialog::getInstance(false) != 0)
        PlanFlightDialog::getInstance(true)->plotPlannedRoute();

    glEndList();

    // used waypoints (dots)
    if(usedWaypointsList == 0)
            usedWaypointsList = glGenLists(1);

    if(Settings::showUsedWaypoints() && Settings::waypointsDotSize() > 0.) {
        glNewList(usedWaypointsList, GL_COMPILE);
        qglColor(Settings::waypointsDotColor());
        glPointSize(Settings::waypointsDotSize());
        glBegin(GL_POINTS);
        foreach(Pilot *p, Whazzup::getInstance()->whazzupData().allPilots()) {
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
    if (activeAirportsList == 0)
        activeAirportsList = glGenLists(1);
    QList<Airport*> airportList = NavData::getInstance()->airports.values();

    // active airports
    glNewList(activeAirportsList, GL_COMPILE);
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
    if(inactiveAirportsList == 0)
        inactiveAirportsList = glGenLists(1);
    glNewList(inactiveAirportsList, GL_COMPILE);
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
    if(congestionsList == 0)
        congestionsList = glGenLists(1);
    glNewList(congestionsList, GL_COMPILE);
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
            for(int h = 0; h <= 360; h += 10) {
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
    if(sectorPolygonsList == 0)
        sectorPolygonsList = glGenLists(1);

    // make sure all the lists are there to avoid nested glNewList calls
    foreach(const Controller *c, sectorsToDraw) {
        if(c->sector != 0)
            c->sector->getGlPolygon();
    }

    // create a list of lists
    glNewList(sectorPolygonsList, GL_COMPILE);
    foreach(const Controller *c, sectorsToDraw) {
        if(c->sector != 0)
            glCallList(c->sector->getGlPolygon());
    }
    glEndList();



    QList<Airport*> airportList = NavData::getInstance()->airports.values();



    // FIR borders
    if(sectorPolygonBorderLinesList == 0)
        sectorPolygonBorderLinesList = glGenLists(1);

    if(!allSectorsDisplayed && Settings::firBorderLineStrength() > 0.) {
        // first, make sure all lists are there
        foreach(const Controller *c, sectorsToDraw) {
            if(c->sector != 0)
                c->sector->getGlBorderLine();
        }
        glNewList(sectorPolygonBorderLinesList, GL_COMPILE);
        foreach(const Controller *c, sectorsToDraw) {
            if(c->sector != 0)
                glCallList(c->sector->getGlBorderLine());
        }
        glEndList();
    } else if(allSectorsDisplayed && Settings::firBorderLineStrength() > 0.) {
        // display ALL fir borders
        foreach(Sector *s, NavData::getInstance()->sectors.values())
            s->getGlBorderLine();
        glNewList(sectorPolygonBorderLinesList, GL_COMPILE);
        foreach(Sector *s, NavData::getInstance()->sectors.values())
            glCallList(s->getGlBorderLine());
        glEndList();
    }

    // APP border lines
    if(appBorderLinesList == 0)
        appBorderLinesList = glGenLists(1);

    if(Settings::appBorderLineStrength() > 0.) {
        foreach(Airport *a, airportList)
            if(!a->approaches.isEmpty())
                a->getAppBorderDisplayList();

        glNewList(appBorderLinesList, GL_COMPILE);
        foreach(Airport *a, airportList)
            if(!a->approaches.isEmpty())
                glCallList(a->getAppBorderDisplayList());
        glEndList();
    }
    qDebug() << "GLWidget::createControllersLists() -- finished";
}

void GLWidget::createStaticLists(){
    // earth
    qDebug() << "GLWidget::createStaticLists() earth";
    earthQuad = gluNewQuadric();
    gluQuadricDrawStyle(earthQuad, GLU_FILL); // FILL, LINE, SILHOUETTE or POINT
    gluQuadricNormals(earthQuad, GLU_SMOOTH); // NONE, FLAT or SMOOTH
    gluQuadricOrientation(earthQuad, GLU_OUTSIDE); // GLU_INSIDE

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



    earthList = glGenLists(1);
    glNewList(earthList, GL_COMPILE);
    qglColor(Settings::globeColor());
    gluSphere(earthQuad, 1, qRound(360 / Settings::glCirclePointEach()), // draw a globe with radius, slicesX, stacksZ
              qRound(180 / Settings::glCirclePointEach()));
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
    gridlinesList = glGenLists(1);
    glNewList(gridlinesList, GL_COMPILE);
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
    coastlinesList = glGenLists(1);
    glNewList(coastlinesList, GL_COMPILE);
    if (Settings::coastLineStrength() > 0.0) {
        qglColor(Settings::coastLineColor());
        glLineWidth(Settings::coastLineStrength());
        LineReader lineReader(Settings::applicationDataDirectory("data/coastline.dat"));
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
    countriesList = glGenLists(1);
    glNewList(countriesList, GL_COMPILE);
    if (Settings::countryLineStrength() > 0.0) {
        qglColor(Settings::countryLineColor());
        glLineWidth(Settings::countryLineStrength());
        LineReader countries = LineReader(Settings::applicationDataDirectory("data/countries.dat"));
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
    FixesList = glGenLists(1);
    if(Settings::showAllWaypoints()) {
        qDebug() << "GLWidget::createStaticLists() allWaypoints";
        glNewList(FixesList, GL_COMPILE);
        qglColor(Settings::waypointsDotColor());
        glLineWidth(Settings::countryLineStrength());
        double sin30 = .5; double cos30 = .8660254037;
        double tri_c = .01; double tri_a = tri_c * cos30; double tri_b = tri_c * sin30;
        glBegin(GL_TRIANGLES);
        foreach( Waypoint *w, Airac::getInstance()->allPoints) {
            if(w->getTyp() == 1){
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

void GLWidget::createSaticSectorLists(QList<Sector*> sectors){

    //Polygon
    if(staticSectorPolygonsList == 0){
        staticSectorPolygonsList = glGenLists(1);
    }

    // make sure all the lists are there to avoid nested glNewList calls
    foreach(Sector *sector, sectors) {
        if(sector != 0){
            sector->getGlPolygon();}
    }

    // create a list of lists
    glNewList(staticSectorPolygonsList, GL_COMPILE);
    foreach(Sector *sector, sectors) {
        if(sector != 0){
            glCallList(sector->getGlPolygon());}
    }
    glEndList();


    // FIR borders
    if(staticSectorPolygonBorderLinesList == 0){
        staticSectorPolygonBorderLinesList = glGenLists(1);
    }


    if(!allSectorsDisplayed && Settings::firBorderLineStrength() > 0.) {
        // first, make sure all lists are there
        foreach(Sector *sector, sectors) {
            if(sector != 0){
                sector->getGlBorderLine();}
        }
        glNewList(staticSectorPolygonBorderLinesList, GL_COMPILE);
        foreach(Sector *sector, sectors) {
            if(sector != 0){
                glCallList(sector->getGlBorderLine());}
        }
        glEndList();
    }


}

//////////////////////////////////////////
// initializeGL(), paintGL() & resizeGL()
//////////////////////////////////////////

void GLWidget::initializeGL(){
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
    //qDebug() << "GL_SHADING_LANGUAGE_VERSION:" << reinterpret_cast<char const*> (glGetString(GL_SHADING_LANGUAGE_VERSION));
    //qDebug() << "GL_EXTENSIONS:" << reinterpret_cast<char const*> (glGetString(GL_EXTENSIONS));
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
        if(Settings::glBlending()) {
                glEnable(GL_BLEND);
                //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // for texture blending
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // source,dest:
                // ...GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
                // ...GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR,
                // ...GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_SRC_ALPHA_SATURATE

                glEnable(GL_FOG); // fog - fading Earth's borders
                glFogi(GL_FOG_MODE, GL_LINEAR); // GL_EXP2, GL_EXP, GL_LINEAR
                GLfloat fogColor[] = {
                    Settings::backgroundColor().redF(),
                    Settings::backgroundColor().greenF(),
                    Settings::backgroundColor().blueF(),
                    Settings::backgroundColor().alphaF()
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
                    Settings::specularColor().redF(),
                    Settings::specularColor().greenF(),
                    Settings::specularColor().blueF(),
                    Settings::specularColor().alphaF()
                };
                const GLfloat earthEmission[] = {0, 0, 0, 1};
                const GLfloat earthShininess[] = {Settings::earthShininess()};
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
                const GLfloat sunDiffuse[] = {adjustSunDiffuse.redF(), adjustSunDiffuse.greenF(),
                                              adjustSunDiffuse.blueF(), adjustSunDiffuse.alphaF()};
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
                lightsGenerated = true;
        }

        createStaticLists();
        qDebug() << "GLWidget::initializeGL() -- finished";
}

void GLWidget::paintGL() {
    //qint64 started = QDateTime::currentMSecsSinceEpoch(); // for method execution time calculation. See last line of method.
    //qDebug() << "GLWidget::paintGL()";

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        glTranslatef(0, 0, -10);
        glRotated(xRot, 1, 0, 0);
        glRotated(yRot, 0, 1, 0);
        glRotated(zRot, 0, 0, 1);

    if (Settings::glLighting()) {

        //check if lights generated
        if(!lightsGenerated){
            createLights();
        }

        glEnable(GL_LIGHTING);
        // moving sun's position
        QPair<double, double> zenith = sunZenith(Whazzup::getInstance()->whazzupData().whazzupTime.isValid()?
                                                 Whazzup::getInstance()->whazzupData().whazzupTime:
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
    if (Settings::glTextures() && earthTex != 0 && Settings::glLighting()) {
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE
            glBindTexture(GL_TEXTURE_2D, earthTex);
        }
    if (Settings::glTextures() && earthTex != 0 && !Settings::glLighting()){
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE
        glBindTexture(GL_TEXTURE_2D, earthTex);
    }


    glCallList(earthList);
    if (Settings::glLighting())
        glDisable(GL_LIGHTING); // disable lighting after drawing earth...

    if (Settings::glTextures() && earthTex != 0) // disable textures after drawing earth...
        glDisable(GL_TEXTURE_2D);

    glCallList(coastlinesList);
    glCallList(countriesList);
    glCallList(gridlinesList);
    if(Settings::showAllWaypoints() && zoom < allWaypointsLabelZoomTreshold * .7)
        glCallList(FixesList);
    if(Settings::showUsedWaypoints() && zoom < usedWaypointsLabelZoomThreshold * .7)
        glCallList(usedWaypointsList);

    //render Center
    if(Settings::showCTR()){
        glCallList(sectorPolygonsList);
        glCallList(sectorPolygonBorderLinesList);
    }

    //Static Sectors (for editing Sectordata)
    if(renderstaticSectors){
        glCallList(staticSectorPolygonsList);
        glCallList(staticSectorPolygonBorderLinesList);
    }

    QList<Airport*> airportList = NavData::getInstance()->airports.values();
    //render Aproach
    if(Settings::showAPP()){
        glCallList(appBorderLinesList);
        foreach(Airport *a, airportList) {

            if(!a->approaches.isEmpty())
                glCallList(a->getAppDisplayList());
        }
    }

    //render Tower
    if(Settings::showTWR()){
        foreach(Airport *a, airportList) {
            //if(a == 0) continue;
            if(!a->towers.isEmpty())
                glCallList(a->getTwrDisplayList());
        }
    }

    //render Ground/Delivery
    if(Settings::showGND()){
        foreach(Airport *a, airportList) {
            if(!a->grounds.isEmpty())
                glCallList(a->getGndDisplayList());
            if(!a->deliveries.isEmpty())
                glCallList(a->getDelDisplayList());
        }
    }


    if(Settings::showAirportCongestion())
            glCallList(congestionsList);
    glCallList(activeAirportsList);
    if(Settings::showInactiveAirports() && (zoom < inactiveAirportLabelZoomTreshold * .7))
            glCallList(inactiveAirportsList);

    glCallList(pilotsList);


    //Highlight friends
    if(Settings::highlightFriends()){
        if(highlighter == 0) createFriendHighlighter();
        QTime time = QTime::currentTime();
        double range = (time.second()%5);
        range += (time.msec()%500)/1000;
;
        GLfloat red = Settings::highlightColor().redF();
        GLfloat green = Settings::highlightColor().greenF();
        GLfloat blue = Settings::highlightColor().blueF();
        GLfloat alpha = Settings::highlightColor().alphaF();
        double lineWidth = Settings::highlightLineWidth();
        if(!Settings::useHighlightAnimation()) {
            range = 0;
            destroyFriendHightlighter();
        }

        for(int ii = 0; ii < friends.size(); ii++)
        {
            glBegin(GL_LINE_LOOP);
            glLineWidth(lineWidth);
            glColor4f(red, green, blue, alpha);
            GLdouble circle_distort = qCos(friends.value(ii).first * Pi180);
            for(int i = 0; i <= 360; i += 20){
                double x = friends.value(ii).first  + Nm2Deg((100-(range*20))) * circle_distort * qCos(i * Pi180);
                double y = friends.value(ii).second + Nm2Deg((100-(range*20))) * qSin(i * Pi180);
                VERTEX(x, y);
            }
            glEnd();
        }
    }

    //render Wind
    if(Settings::showUpperWind()){
        //Display al +/- 1000ft
        glCallList(WindData::getInstance()->getWindArrows((Settings::upperWindAlt()-1)));
        glCallList(WindData::getInstance()->getWindArrows(Settings::upperWindAlt()));
        glCallList(WindData::getInstance()->getWindArrows((Settings::upperWindAlt()+1)));
    }

    //render labels
    renderLabels();

    if (mapIsRectangleSelecting)
            drawSelectionRectangle();

    /*
    // some preparations to draw small textures on the globe (plane symbols, wind data...).
    QPixmap planePm(":/icons/images/arrowup16.png");
    GLuint planeTex = bindTexture(planePm, GL_TEXTURE_2D,
                                  GL_RGBA, QGLContext::LinearFilteringBindOption); // QGLContext::MipmapBindOption
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, planeTex);
    glColor3f(1., 1., 1.);
    for (double lat = -90.; lat <= 90.; lat += 45.) {
        for (double lon = -180.; lon <= 180.; lon += 45.) {
            glPushMatrix();
            glTranslatef(SX(lat, lon), SY(lat, lon), SZ(lat, lon));
            glRotatef(0, 1, 0, 0);
            glRotatef(90, 0, 1, 0);
            glRotatef(90, 0, 0, 1);

            glBegin(GL_QUADS);
            glTexCoord2i(0, 1);
            glVertex2f(-.05,  .15);
            glTexCoord2i(1, 1);
cloudsIm.width()            glVertex2f( .05,  .15);
            glTexCoord2i(1, 0);
            glVertex2f( .05, -.15);
            glTexCoord2f(.4, 0);
            glVertex2f(-.05, -.15);
            glEnd();
            glPopMatrix();
        }
    }
    glDisable(GL_TEXTURE_2D);
    */
    //drawCoordinateAxii(); // use this to see where the axii are (x = red, y = green, z = blue)


    glFlush(); // http://www.opengl.org/sdk/docs/man/xhtml/glFlush.xml

    // just for performance measurement:
    //qDebug() << "GLWidget::paintGL() -- finished in" << QDateTime::currentMSecsSinceEpoch() - started << "ms";
}

void GLWidget::resizeGL(int width, int height) {
    aspectRatio = (double)width / (double)height;
    glViewport(0, 0, width, height);
    resetZoom();
}

////////////////////////////////////////////////////////////
// SLOTS: mouse, key... and general user-map interaction
//
void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons().testFlag(Qt::RightButton) || // check before left button if useSelectionRectangle=off
            (!Settings::useSelectionRectangle() && event->buttons().testFlag(Qt::LeftButton))) { // rotate
        mapIsMoving = true;
        handleRotation(event);
    } else if (event->buttons().testFlag(Qt::MiddleButton)) { // zoom
        mapIsZooming = true;
        zoomIn((event->x() - lastPos.x() - event->y() + lastPos.y()) / 100. * Settings::zoomFactor());
        lastPos = event->pos();
    } else if (event->buttons().testFlag(Qt::LeftButton)) { // selection rectangle
        mapIsRectangleSelecting = true;
        updateGL();
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    QToolTip::hideText();
    if (mapIsMoving || mapIsZooming || mapIsRectangleSelecting) {
        mapIsMoving = false;
        mapIsZooming = false;
        mapIsRectangleSelecting = false;
        updateGL();
    }
    if (!mapIsRectangleSelecting)
        lastPos = mouseDownPos = event->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    QToolTip::hideText();
    if (mapIsMoving)
        mapIsMoving = false;
    else if (mapIsZooming)
        mapIsZooming = false;
    else if (mapIsRectangleSelecting) {
        mapIsRectangleSelecting = false;
        if (event->pos() != mouseDownPos) {
            //Checking if courser moved more than 10pix in x and y direction
            if(((event->x()- mouseDownPos.x())*(event->x()- mouseDownPos.x())) > 4000 && ((event->y() - mouseDownPos.y())*(event->y()-mouseDownPos.y())) >4000 )
            {
                double downLat, downLon;
                if (mouse2latlon(mouseDownPos.x(), mouseDownPos.y(), downLat, downLon)) {
                    double currLat, currLon;
                    if (mouse2latlon(event->x(), event->y(), currLat, currLon)) {
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
    } else if (mouseDownPos == event->pos() && event->button() == Qt::LeftButton) {
        QList<MapObject*> objects;
        foreach(MapObject* m, objectsAt(event->x(), event->y()))
            if (dynamic_cast<Waypoint*>(m) == 0) // all but waypoints have a dialog
                objects.append(m);
        if (objects.isEmpty()) {
            clientSelection->clearClients();
            clientSelection->close();
        } else if (objects.size() == 1)
            objects[0]->showDetailsDialog();
        else {
            clientSelection->move(event->globalPos());
            clientSelection->setObjects(objects);
        }
    } else if (mouseDownPos == event->pos() && event->button() == Qt::RightButton)
        rightClick(event->pos());
    updateGL();
}

void GLWidget::rightClick(const QPoint& pos) {
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
    if(countRelevant == 0) {
        GuiMessages::message("no object under cursor");
        return;
    }
    if(countRelevant > 1) {
        GuiMessages::message("too many objects under cursor");
        return; // ambiguous search result
    }
    if(airport != 0) {
        GuiMessages::message(QString("toggled routes for %1 [%2]").arg(airport->label).
                             arg(airport->showFlightLines? "off": "on"),
                             "routeToggleAirport");
        airport->showFlightLines = !airport->showFlightLines;
        if (AirportDetails::getInstance(false) != 0)
            AirportDetails::getInstance(true)->refresh();
        if (PilotDetails::getInstance(false) != 0) // can have an effect on the state of
            PilotDetails::getInstance(true)->refresh(); // ...PilotDetails::cbPlotRoutes
        createPilotsList();
        updateGL();
        return;
    }
    if(pilot != 0) {
        // display flight path for pilot
        GuiMessages::message(QString("toggled route for %1 [%2]").arg(pilot->label).
                             arg(pilot->showDepDestLine? "off": "on"),
                             "routeTogglePilot");
        pilot->showDepDestLine = !pilot->showDepDestLine;
        if (PilotDetails::getInstance(false) != 0)
            PilotDetails::getInstance(true)->refresh();
        createPilotsList();
        updateGL();
        return;
    }
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    QToolTip::hideText();
    if (event->buttons().testFlag(Qt::LeftButton)) {
        double lat, lon;
        if (mouse2latlon(event->x(), event->y(), lat, lon))
            setMapPosition(lat, lon, zoom, false);
        zoomIn(.6);
    } else if (event->button() == Qt::RightButton) {
        double lat, lon;
        if (mouse2latlon(event->x(), event->y(), lat, lon))
            setMapPosition(lat, lon, zoom, false);
        zoomIn(-.6);
    } else if (event->button() == Qt::MiddleButton)
        zoomTo(2.);
}

void GLWidget::wheelEvent(QWheelEvent* event) {
    QToolTip::hideText();
    //if(event->orientation() == Qt::Vertical) {
    if (qAbs(event->delta()) > Settings::wheelMax()) // always recalibrate if bigger values are found
        Settings::setWheelMax(qAbs(event->delta()));
    zoomIn((double) event->delta() / Settings::wheelMax());
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
    zoom -= zoom * qMax(-.6, qMin(.6, .2 * factor * Settings::zoomFactor()));
    resetZoom();
    updateGL();
}

void GLWidget::zoomTo(double zoom) {
    this->zoom = zoom;
    resetZoom();
    updateGL();
}



/////////////////////////////
// rendering text
/////////////////////////////

void GLWidget::renderLabels() {
    fontRectangles.clear();
    allFontRectangles.clear();

    // FIR labels
    QList<MapObject*> objects;
    foreach(Controller *c, Whazzup::getInstance()->whazzupData().controllers) // draw all CTR+FSS labels, also if sector unknown
        if (!c->getCenter().isNull())
            objects.append(c);
    renderLabels(objects, Settings::firFont(), controllerLabelZoomTreshold,
                 Settings::firFontColor());
    if(allSectorsDisplayed) {
        qglColor(Settings::firFontColor());
        foreach (const Sector *sector, NavData::getInstance()->sectors)
            renderText(SX(sector->lat, sector->lon), SY(sector->lat, sector->lon),
                       SZ(sector->lat, sector->lon), sector->icao, Settings::firFont());
    }

    // planned route waypoint labels from Flightplan Dialog
    if(PlanFlightDialog::getInstance(false) != 0) {
        if(PlanFlightDialog::getInstance(true)->cbPlot->isChecked() &&
           PlanFlightDialog::getInstance(true)->selectedRoute != 0) {
            objects.clear();
            for (int i=1; i < PlanFlightDialog::getInstance(true)->
                       selectedRoute->waypoints.size() - 1; i++)
                objects.append(PlanFlightDialog::getInstance(true)->
                               selectedRoute->waypoints[i]);
            renderLabels(objects, Settings::waypointsFont(), usedWaypointsLabelZoomThreshold,
                         Settings::waypointsFontColor());
        }
    }

    // airport labels
    objects.clear();
    QList<Airport*> airportList = NavData::getInstance()->activeAirports.values(); //ordered by congestion ascending,
                                                                        //big airport's labels will always be drawn first
    for(int i = airportList.size() - 1; i > -1; i--) { // from up to down
        //if(airportList[i] == 0) continue; // if we look carefully, we should be able to get rid of this
        if(airportList[i]->active)
            objects.append(airportList[i]);
    }
    renderLabels(objects, Settings::airportFont(), activeAirportLabelZoomTreshold,
                 Settings::airportFontColor());

    // pilot labels
    objects.clear();
    QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().pilots.values();
    for(int i = 0; i < pilots.size(); i++)
        objects.append(pilots[i]);
    if(Settings::showPilotsLabels()){
        renderLabels(objects, Settings::pilotFont(), pilotLabelZoomTreshold,
                 Settings::pilotFontColor());
    }

    // waypoints used in shown routes
    QSet<MapObject*> waypointObjects; // using a QSet 'cause it takes care of unique values
    if (Settings::showUsedWaypoints()) {
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
        renderLabels(waypointObjects.toList(), Settings::waypointsFont(), usedWaypointsLabelZoomThreshold,
                     Settings::waypointsFontColor());
    }

    // inactive airports
    if(Settings::showInactiveAirports()) { // + inactive labels
        objects.clear();
        foreach(Airport *airport, NavData::getInstance()->airports.values())
            if (!airport->active)
                objects.append(airport);
        renderLabels(objects, Settings::inactiveAirportFont(), inactiveAirportLabelZoomTreshold,
                     Settings::inactiveAirportFontColor());
    }

    /*// all waypoints (fixes + navaids)
    QSet<MapObject*> tmp_points;
    foreach(Waypoint* wp, Airac::getInstance()->allPoints)
        tmp_points.insert(wp);

    /*Airac::getInstance()->allPoints.subtract(waypointObjects).toList()
    if(Settings::showAllWaypoints())
        renderLabels(tmp_points.subtract(waypointObjects).toList(), Settings::waypointsFont(),
                     allWaypointsLabelZoomTreshold, Settings::waypointsFontColor());*/
}

void GLWidget::renderLabels(const QList<MapObject*>& objects, const QFont& font,
                            double zoomTreshold, QColor color) {
    if (Settings::simpleLabels()) // cheap function
        renderLabelsSimple(objects, font, zoomTreshold, color);
    else // expensive function
        renderLabelsComplex(objects, font, zoomTreshold, color);
}

// this one checks if labels overlap etc. the transformation lat/lon -> x,y is very expensive
void GLWidget::renderLabelsComplex(const QList<MapObject*>& objects, const QFont& font,
                            double zoomTreshold, QColor color) {
    if(zoom > zoomTreshold || color.alpha() == 0)
        return; // don't draw if too far away or color-alpha == 0
    color.setAlphaF(qMax(0., qMin(1., (zoomTreshold - zoom) / zoomTreshold * 1.5))); // fade out

    QFontMetricsF fontMetrics(font, this);
    foreach(MapObject *o, objects) {
        if (fontRectangles.size() >= Settings::maxLabels())
            break;
        if (!o->drawLabel)
            continue;
        int x, y; if (pointIsVisible(o->lat, o->lon, &x, &y)) {
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
            foreach(const QRectF &r, rects) {
                if(shouldDrawLabel(r)) {
                    drawnFontRect = new FontRectangle(r, o);
                    qglColor(color);

                    // yes, this is slow and it is known: ..
                    // https://bugreports.qt-project.org/browse/QTBUG-844
                    renderText(r.left(), (r.top() + r.height()), text, font);
                    fontRectangles.insert(drawnFontRect);
                    allFontRectangles.insert(drawnFontRect);
                    break;
                }
            }
            if (drawnFontRect == 0) // default position if it was not drawn
                allFontRectangles.insert(new FontRectangle(rect, o));
        }
    }
}

// this one uses 3D-coordinates to paint and does not check overlap
// which results in tremendously improved framerates
void GLWidget::renderLabelsSimple(const QList<MapObject*>& objects, const QFont& font,
                            double zoomTreshold, QColor color) {
    if (zoom > zoomTreshold || color.alpha() == 0)
        return; // don't draw if too far away or color-alpha == 0
    color.setAlphaF(qMax(0., qMin(1., (zoomTreshold - zoom) / zoomTreshold * 1.5))); // fade out

    foreach(MapObject *o, objects) {
        if (fontRectangles.size() >= Settings::maxLabels())
            break;
        fontRectangles.insert(new FontRectangle(QRectF(), 0)); // we use..
                        // this bogus value to stay compatible..
                        // with maxLabels-checking
        if (!o->drawLabel)
            continue;
        qglColor(color);
        renderText(SXhigh(o->lat, o->lon), SYhigh(o->lat, o->lon), SZhigh(o->lat, o->lon),
                   o->mapLabel(), font);
    }
}

bool GLWidget::shouldDrawLabel(const QRectF &rect) {
    foreach(const FontRectangle *fr, fontRectangles) {
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
    foreach(const FontRectangle *fr, allFontRectangles) // scan text labels
        if(fr->rect.contains(x, y))
            result.append(fr->object);

    double lat, lon; if(!mouse2latlon(x, y, lat, lon)) // returns false if not on globe
        return result;

    double radiusDegQuad = Nm2Deg((qFuzzyIsNull(radius)? 30. * zoom: radius));
    radiusDegQuad *= radiusDegQuad;

    foreach(Airport* a, NavData::getInstance()->airports.values()) {
        if(a->active) {
            double x = a->lat - lat;
            double y = a->lon - lon;
            if(x*x + y*y < radiusDegQuad) {
                result.removeAll(a);
                result.append(a);
            }
        }
    }
    QList<MapObject*> observers;
    foreach(Controller *c, Whazzup::getInstance()->whazzupData().controllers) {
        double x = c->lat - lat;
        double y = c->lon - lon;
        if(x*x + y*y < radiusDegQuad) {
            if(c->isObserver()) {
                observers.removeAll(c);
                observers.append(c);
            } else {
                result.removeAll(c);
                result.append(c);
            }
        }
    }
    foreach(Pilot *p, Whazzup::getInstance()->whazzupData().pilots.values()) {
        double x = p->lat - lat;
        double y = p->lon - lon;
        if(x*x + y*y < radiusDegQuad) {
            result.removeAll(p);
            result.append(p);
        }
    }
    return result + observers;
}

/////////////////////////
// draw-helper functions
/////////////////////////

void GLWidget::drawSelectionRectangle() const {
    QPoint current = mapFromGlobal(QCursor::pos());
    double downLat, downLon;
    if (mouse2latlon(mouseDownPos.x(), mouseDownPos.y(), downLat, downLon)) {
        double currLat, currLon;
        if (mouse2latlon(current.x(), current.y(), currLat, currLon)) {
            // calculate a rectangle: approximating what the viewport will look after zoom (far from perfect)
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

            glColor4f(0., 1., 1., .5);
            glBegin(GL_POLYGON);
            NavData::plotPointsOnEarth(points);
            glEnd();
            glLineWidth(2.);
            glColor4f(0., 1., 1., 1.);
            glBegin(GL_LINE_LOOP);
            NavData::plotPointsOnEarth(points);
            glEnd();
        }
    }
}

void GLWidget::drawCoordinateAxii() const { // just for debugging. Visualization of the axii. red=x/green=y/blue=z
        glPushMatrix(); glLoadIdentity();
        glTranslatef(0, 0, -9);
        glRotated(xRot, 1, 0, 0); glRotated(yRot, 0, 1, 0); glRotated(zRot, 0, 0, 1);
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
        NavData::getInstance()->updateData(Whazzup::getInstance()->whazzupData());

        sectorsToDraw = Whazzup::getInstance()->whazzupData().activeSectors();
        createPilotsList();
        createAirportsList();
        createControllersLists();
        friends = Whazzup::getInstance()->whazzupData().friendsLatLon;

        updateGL();
    }
    qDebug() << "GLWidget::newWhazzupData -- finished";
}

void GLWidget::displayAllSectors(bool value) {
    allSectorsDisplayed = value;
    newWhazzupData(true);
}

void GLWidget::showInactiveAirports(bool value) {
    Settings::setShowInactiveAirports(value);
    newWhazzupData(true);
}

void GLWidget::savePosition()
{
    Settings::setRememberedMapPosition(xRot, yRot, zRot, zoom, 1);
}

void GLWidget::createFriendHighlighter(){
    highlighter = new QTimer(this);
    highlighter->setInterval(100);
    connect(highlighter, SIGNAL(timeout()), this, SLOT(updateGL()));
    highlighter->start();
}

void GLWidget::destroyFriendHightlighter(){
    if(highlighter == 0) return;
    if(highlighter->isActive()) highlighter->stop();
    disconnect(highlighter, SIGNAL(timeout()), this, SLOT(updateGL()));
    delete highlighter;
    highlighter = 0;
}


//////////////////////////////////
// Clouds, Lightning and Earth Textures
//////////////////////////////////

void GLWidget::useClouds(){
    parseEarthClouds();
}

void GLWidget::parseEarthClouds()
{
    qDebug() << "GLWidget::parseEarthClouds -- start parsing";

    QImage earthTexIm;
    QImage cloudsIm;
    //completedEarthTexIm

    //Check if clouds available
    if(cloudsAvaliable && Settings::showClouds()){
    QString cloudsTexFile = Settings::applicationDataDirectory(QString("textures/clouds/clouds.jpg"));
    cloudsIm = QImage(cloudsTexFile);
    }

    if(Settings::glTextures()) {
        QString earthTexFile = Settings::applicationDataDirectory(
                    QString("textures/%1").arg(Settings::glTextureEarth()));
        earthTexIm.load(earthTexFile);
    } else {
        completedEarthTexIm = cloudsIm;
        qDebug() << "GLWidget::parseEarthClouds -- finished using clouds";
    }

    if(cloudsIm.isNull() && Settings::glTextures())
    {
        completedEarthTexIm = earthTexIm;
        qDebug() << "GLWidget::parseEarthClouds -- finished using earthTex (no cloud tex found)";
    }

    if(!Settings::showClouds() && Settings::glTextures()){
        completedEarthTexIm = earthTexIm;
        qDebug() << "GLWidget::parseEarthClouds -- finished using earthTex";
    }

    if(!cloudsIm.isNull() && !earthTexIm.isNull() && Settings::showClouds())
    {
        //transform so same size, take the bigger image
        int width = cloudsIm.width();
        int height = cloudsIm.height();

        if(earthTexIm.width() > width){
            cloudsIm = cloudsIm.scaledToWidth(earthTexIm.width(), Qt::SmoothTransformation);
            width = earthTexIm.width();
        }
        else{
            earthTexIm = earthTexIm.scaledToWidth(width, Qt::SmoothTransformation);

        }

        if(earthTexIm.height() > height){
            cloudsIm = cloudsIm.scaledToHeight(earthTexIm.height(), Qt::SmoothTransformation);
            height = earthTexIm.height();
        }
        else {
            earthTexIm = earthTexIm.scaledToHeight(height, Qt::SmoothTransformation);
        }

        completedEarthTexIm = earthTexIm;
        completedEarthTexIm = completedEarthTexIm.convertToFormat(QImage::Format_ARGB32);



        //read every pixel and add clouds to earth
        for( int line = 0; line < completedEarthTexIm.height(); line++)
        {
            QRgb* cloudPixel = reinterpret_cast<QRgb*>(
                        cloudsIm.scanLine(line));
            QRgb* pixel = reinterpret_cast<QRgb*>(
                        completedEarthTexIm.scanLine(line));

            for(int pos = 0; pos < completedEarthTexIm.width() ; pos++)
            {
                int cRed = qRed(cloudPixel[pos]);
                int cGreen = qGreen(cloudPixel[pos]);
                int cBlue =  qBlue(cloudPixel[pos]);

                int red = qRed(pixel[pos]);
                int green = qGreen(pixel[pos]);
                int blue = qBlue(pixel[pos]);
                int alpha = qAlpha(pixel[pos]);

                red += cRed;
                green += cGreen;
                blue += cBlue;

                if(red > 255) red = 255;
                if(green > 255) green = 255;
                if(blue > 255) blue = 255;

                pixel[pos] = qRgba( red, green, blue, alpha);
            }
        }
        qDebug() << "GLWidget::parseEarthClouds -- finished parsing using clouds and earthTex";
    }


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
    update();
}

void GLWidget::createLights()
{
    //const GLfloat earthAmbient[]  = {0, 0, 0, 1};
    const GLfloat earthDiffuse[]  = {1, 1, 1, 1};
    const GLfloat earthSpecular[] = {Settings::specularColor().redF(), Settings::specularColor().greenF(),
                                                                     Settings::specularColor().blueF(), Settings::specularColor().alphaF()};
    const GLfloat earthEmission[] = {0, 0, 0, 1};
    const GLfloat earthShininess[] = {Settings::earthShininess()};
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
    const GLfloat sunDiffuse[] = {adjustSunDiffuse.redF(), adjustSunDiffuse.greenF(),
                                  adjustSunDiffuse.blueF(), adjustSunDiffuse.alphaF()};
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
    lightsGenerated = true;
}



