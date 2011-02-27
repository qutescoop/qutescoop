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
#include "ListClientsDialog.h"
#include "Window.h"

GLWidget::GLWidget(QGLFormat fmt, QWidget *parent) :
        QGLWidget(fmt, parent),
        xRot(0), yRot(0), zRot(0), zoom(2), aspectRatio(1), earthTex(0),
        earthList(0), coastlinesList(0), countriesList(0), gridlinesList(0),
        pilotsList(0), airportsList(0), airportsInactiveList(0), congestionsList(0), fixesList(0),
        sectorPolygonsList(0), sectorPolygonBorderLinesList(0), airportControllersList(0), appBorderLinesList(0),
        pilotLabelZoomTreshold(0.9), airportLabelZoomTreshold(1.2),
        inactiveAirportDotZoomTreshold(0.15), inactiveAirportLabelZoomTreshold(0.3),
        controllerLabelZoomTreshold(2), fixZoomTreshold(0.05),
        plotFlightPlannedRoute(false),
        allSectorsDisplayed(false),
        mapIsMoving(false),
        shutDownAnim_t(0)
{
    setAutoFillBackground(false);
    setMouseTracking(true);
    // call default (=1) map position
    Settings::getRememberedMapPosition(&xRot, &yRot, &zRot, &zoom, 1);
    xRot = modPositive(xRot, 360.);
    yRot = modPositive(yRot, 360.);
    zRot = modPositive(zRot, 360.);
    resetZoom();
    emit newPosition();
}

GLWidget::~GLWidget() {
    makeCurrent();
    glDeleteLists(earthList, 1); glDeleteLists(gridlinesList, 1);
    glDeleteLists(coastlinesList, 1); glDeleteLists(countriesList, 1); glDeleteLists(fixesList, 1);
    glDeleteLists(pilotsList, 1);
    glDeleteLists(airportsList, 1); glDeleteLists(airportsInactiveList, 1); glDeleteLists(congestionsList, 1);
    glDeleteLists(airportControllersList, 1); glDeleteLists(appBorderLinesList, 1);
    glDeleteLists(sectorPolygonsList, 1); glDeleteLists(sectorPolygonBorderLinesList, 1);

    if (earthTex != 0)
        deleteTexture(earthTex);
        //glDeleteTextures(1, &earthTex); // handled Qt'ish by deleteTexture
    gluDeleteQuadric(earthQuad);

    QList<Airport*> airportList = NavData::getInstance()->airports().values();
    for (int i = 0; i < airportList.size(); i++) {
        delete airportList[i];
    }
    QList<Sector*> sectorList = NavData::getInstance()->sectors().values();
    for (int i = 0; i < sectorList.size(); i++) {
        delete sectorList[i];
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Methods handling position: world lat/lon <-> scene x/y/z <-> mouse x/y and related.
//
// scene -> unrotated world (looking onto N0/E0):
//      (1,0,0)->(0,90), (0,1,0)->(0,180), (0,0,1)->(-90,0) [Southpole].
// The scene is then rotated by xRot/yRot/zRot. When looking onto N0/E0, -90°/0°/0°
// This looks a bit anarchic, but it fits the automatically created texture coordinates.
// call drawCoordinateAxii() inside paintGL() to se where the axii are.
void GLWidget::setMapPosition(double lat, double lon, double newZoom) {
    xRot = modPositive(270. - lat, 360.);
    zRot = modPositive(     - lon, 360.);
    zoom = newZoom;
    resetZoom();
    updateGL();
    emit newPosition();
}

QPair<double, double> GLWidget::currentPosition() { // current rotation in lat/lon
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

bool GLWidget::mouse2latlon(int x, int y, double& lat, double& lon) const {
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
    double x0 = -zGl * sin(theta) + yGl * cos(theta);
    double z0 = +zGl * cos(theta) + yGl * sin(theta);
    double y0 = xGl;

    // 4) now to lat/lon
    lat = qAtan(-z0 / qSqrt(1 - (z0*z0))) * 180 / M_PI;
    lon = qAtan(-x0 / y0) * 180 / Pi - 90;

    // 5) atan might have lost the sign
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
    emit hasGuiMessage(QString("Remembered position %1").arg(nr));
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

const QPair<double, double> GLWidget::sunZenith(const QDateTime &dateTime) {
    // dirtily approximating present zenith Lat/Lon (where the sun is directly above).
    // scientific solution: http://openmap.bbn.com/svn/openmap/trunk/src/openmap/com/bbn/openmap/layer/daynight/SunPosition.java
    // [sunPosition()] - that would have been at least 100 lines of code...
    return QPair<double, double>(-23. * cos((double) dateTime.date().dayOfYear() / (double)dateTime.date().daysInYear() * 2.*M_PI),
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

    QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().getPilots();

    // aircraft dots
    glPointSize(Settings::pilotDotSize());
    glBegin(GL_POINTS);
    qglColor(Settings::pilotDotColor());
    for (int i = 0; i < pilots.size(); i++) {
        const Pilot *p = pilots[i];
        if (p->lat != 0 && p->lon != 0)
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

    // flight paths, also for booked flights
    pilots = Whazzup::getInstance()->whazzupData().getAllPilots();
    for (int i = 0; i < pilots.size(); i++) {
        Pilot *p = pilots[i];
        p->plotFlightPath();
    }

    // planned route from Flightplan Dialog
    if(plotFlightPlannedRoute)
        PlanFlightDialog::getInstance()->plotPlannedRoute();

    glEndList();
    qDebug() << "GLWidget::createPilotsList() -- finished";
}

void GLWidget::createAirportsList() {
    qDebug() << "GLWidget::createAirportsList() ";
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
        qglColor(Settings::airportCongestionBorderLineColor());
        glLineWidth(Settings::airportCongestionBorderLineStrength());
        for(int i = 0; i < airportList.size(); i++) {
            if(airportList[i] == 0) continue;
            if(!airportList[i]->isActive()) continue;
            int congested = airportList[i]->numFilteredArrivals + airportList[i]->numFilteredDepartures;
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
    qDebug() << "GLWidget::createAirportsList() -- finished";
}

void GLWidget::prepareDisplayLists() {
    qDebug() << "GLWidget::prepareDisplayLists() ";

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
    qDebug() << "GLWidget::prepareDisplayLists() -- finished";
}

void GLWidget::createObjects(){
    // earth
    qDebug() << "GLWidget::createObjects() earth";
    earthQuad = gluNewQuadric();
    gluQuadricDrawStyle(earthQuad, GLU_FILL); // FILL, LINE, SILHOUETTE or POINT
    gluQuadricNormals(earthQuad, GLU_SMOOTH); // NONE, FLAT or SMOOTH
    gluQuadricOrientation(earthQuad, GLU_OUTSIDE); // GLU_INSIDE
    if (Settings::glTextures()) {
        QString earthTexFile = Settings::applicationDataDirectory("textures/earth.png");
        QImage earthTexIm = QImage(earthTexFile);
        if (earthTexIm.isNull())
            qWarning() << "Unable to load texture file" << earthTexFile;
        else {
            GLint max_texture_size; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
            qDebug() << "OpenGL reported MAX_TEXTURE_SIZE as" << max_texture_size << "// trying to bind" <<
                    earthTexFile << "as" << earthTexIm.width() << "x" << earthTexIm.height() << "px texture";
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            //glGenTextures(1, &earthTex); // bindTexture does this the Qt'ish way already
            glGetError(); // empty the error buffer
            earthTex = bindTexture(earthTexIm, GL_TEXTURE_2D,
                                   GL_RGBA, QGLContext::LinearFilteringBindOption); // QGLContext::MipmapBindOption
            if (GLenum glError = glGetError())
                qCritical() << QString("OpenGL returned an error (0x%1)").arg((int) glError, 4, 16, QChar('0'));
            gluQuadricTexture(earthQuad, GL_TRUE); // prepare texture coordinates
        }
    }
    earthList = glGenLists(1);
    glNewList(earthList, GL_COMPILE);
    qglColor(Settings::globeColor());
    gluSphere(earthQuad, 1, qRound(360 / Settings::glCirclePointEach()), // draw a globe with radius, slicesX, stacksZ
              qRound(180 / Settings::glCirclePointEach()));
    glEndList();

    // grid
    qDebug() << "GLWidget::createObjects() gridLines";
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
    qDebug() << "GLWidget::createObjects() coastLines";
    coastlinesList = glGenLists(1);
    glNewList(coastlinesList, GL_COMPILE);
    if (Settings::coastLineStrength() > 0.0) {
        qglColor(Settings::coastLineColor());
        glLineWidth(Settings::coastLineStrength());
        LineReader lineReader(Settings::applicationDataDirectory("data/coastline.dat"));
        QList<QPair<double, double> > line = lineReader.readLine();
        while (!line.isEmpty()) {
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < line.size(); i++)
                VERTEX(line[i].first, line[i].second);
            glEnd();
            line = lineReader.readLine();
        }
    }
    glEndList();

    // countries
    qDebug() << "GLWidget::createObjects() countries";
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

    // fixes
    qDebug() << "GLWidget::createObjects() fixes";
    if(!NavData::getInstance()->getAirac().isEmpty() && Settings::showFixes()) {
        fixesList = glGenLists(1);
        glNewList(fixesList, GL_COMPILE);
        qglColor(Settings::countryLineColor());
        glLineWidth(Settings::countryLineStrength());
        const Airac& airac = NavData::getInstance()->getAirac();
        const QList<Waypoint*>& fixes = airac.getAllWaypoints();
        double sin30 = 0.5; double cos30 = 0.8660254037;
        double tri_c = 0.01; double tri_a = tri_c * cos30; double tri_b = tri_c * sin30;
        glBegin(GL_TRIANGLES);
        foreach(Waypoint *wp, fixes) {
            double circle_distort = cos(wp->lat * Pi180);
            double tri_b_c = tri_b * circle_distort;
            VERTEX(wp->lat - tri_b_c, wp->lon - tri_a);
            VERTEX(wp->lat - tri_b_c, wp->lon + tri_a);
            VERTEX(wp->lat + tri_c * circle_distort, wp->lon);
        }
        glEnd();
        glEndList();
    }
}

//////////////////////////////////////////
// initializeGL(), paintGL() & resizeGL()
//

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
    qDebug() << "GL_VENDOR:  "   << reinterpret_cast<char const*> (glGetString(GL_VENDOR));
    qDebug() << "GL_RENDERER:" << reinterpret_cast<char const*> (glGetString(GL_RENDERER));
    qDebug() << "GL_VERSION: "  << reinterpret_cast<char const*> (glGetString(GL_VERSION));
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
	} else {
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}

	glDisable(GL_DEPTH_TEST); // this helps against sectors and coastlines that are "farer" away than the earth superficie
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

        const GLfloat sunAmbient[] = {0, 0, 0, 1};
        QColor adjustSunDiffuse =  // reduce light intensity by number of lights...
                Settings::sunLightColor().darker(100 * Settings::glLights());
        if (Settings::glLights() > 1)
            adjustSunDiffuse.lighter(100 / Settings::glLightsSpread() * 180); // ...and increase again by their distribution
        const GLfloat sunDiffuse[] = {adjustSunDiffuse.redF(), adjustSunDiffuse.greenF(), adjustSunDiffuse.blueF(), adjustSunDiffuse.alphaF()};
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
        const GLfloat modelAmbient[] = {0.2, 0.2, 0.2, 1.0}; // the "background" ambient light
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmbient); // GL_LIGHT_MODEL_AMBIENT, GL_LIGHT_MODEL_COLOR_CONTROL,
        //glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // ...GL_LIGHT_MODEL_LOCAL_VIEWER, GL_LIGHT_MODEL_TWO_SIDE

		glShadeModel(GL_SMOOTH); // SMOOTH or FLAT
	}
	createObjects();

	qDebug() << "GLWidget::initializeGL() -- finished";
}

void GLWidget::paintGL() {
    //qint64 started = QDateTime::currentMSecsSinceEpoch(); // for method execution time calculation. See last line of method.
    //qDebug() << "GLWidget::paintGL()";

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if (shutDownAnim_t != 0) { // we are animatimg...
		quint16 elapsed = (QDateTime::currentMSecsSinceEpoch() - shutDownAnim_t); // 1 elapsed = 1ms
		if (elapsed > 6000)
			qApp->exit(0);
		else
			QTimer::singleShot(20, this, SLOT(updateGL())); // aim for 50 fps
		// move
		GLfloat x = 0.006f * (GLfloat) elapsed * (qCos(elapsed / 1400.0f) - 1.0f)
						* qSin(elapsed / 800.0f);
		GLfloat y = 0.02f  * (GLfloat) elapsed * (qCos(elapsed / 4500.0f) - 0.97f);
		glTranslatef(x, y, 0.0);
		// rotate
		xRot += elapsed / 160.0f;
		yRot += elapsed / 250.0f;
		zRot += elapsed / 130.0f;
		double zoomTo = 2.0f + (float) elapsed * (float) elapsed / 150000.0f;
		zoom = zoom + (zoomTo - zoom) / 10.0;
		resetZoom();
		// morph space color
		qglClearColor(QColor(
				Settings::backgroundColor().red()   + (150 - Settings::backgroundColor().red())   * elapsed/6000.,
				Settings::backgroundColor().green() + (200 - Settings::backgroundColor().green()) * elapsed/6000.,
				Settings::backgroundColor().blue()  + (255 - Settings::backgroundColor().blue())  * elapsed/6000.));
	}
	glTranslatef(0.0, 0.0, -10.0);
	glRotated(xRot, 1.0, 0.0, 0.0);
	glRotated(yRot, 0.0, 1.0, 0.0);
	glRotated(zRot, 0.0, 0.0, 1.0);

    if (Settings::glLighting()) {
        glEnable(GL_LIGHTING);
        // moving sun's position
        QPair<double, double> zenith = sunZenith(Whazzup::getInstance()->whazzupData().timestamp().isValid()?
                                                 Whazzup::getInstance()->whazzupData().timestamp():
                                                 QDateTime::currentDateTimeUtc());
        GLfloat sunVertex0[] = {SX(zenith.first, zenith.second), SY(zenith.first, zenith.second),
                               SZ(zenith.first, zenith.second), 0}; // sun has parallel light -> dist=0
        glLightfv(GL_LIGHT0, GL_POSITION, sunVertex0); // light 0 always has the real (center) position
        if (Settings::glLights() > 1) {
            for (int light = 1; light < Settings::glLights(); light++) { // setting the other light's position
                double fraction = 2*M_PI / (Settings::glLights() - 1) * light;
                double spreadLat = zenith.first  + qSin(fraction) * Settings::glLightsSpread();
                double spreadLon = zenith.second + qCos(fraction) * Settings::glLightsSpread();
                GLfloat sunVertex[] = {SX(spreadLat, spreadLon), SY(spreadLat, spreadLon), SZ(spreadLat, spreadLon), 0};
                glLightfv(GL_LIGHT0 + light, GL_POSITION, sunVertex); // GL_LIGHTn is conveniently GL_LIGHT0 + n
            }
        }
    }
    if (Settings::glTextures() && earthTex != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, earthTex);
    }
    glCallList(earthList);
    if (Settings::glTextures() && earthTex != 0)
        glDisable(GL_TEXTURE_2D);
    if (Settings::glLighting())
        glDisable(GL_LIGHTING); // light only earth, not overlay
    glCallList(coastlinesList);
    glCallList(countriesList);
    glCallList(gridlinesList);
    if(zoom < fixZoomTreshold && Settings::showFixes())
        glCallList(fixesList);

	glCallList(sectorPolygonsList);
	glCallList(sectorPolygonBorderLinesList);
	glCallList(airportControllersList);
	glCallList(appBorderLinesList);
	if(Settings::showAirportCongestion())
		glCallList(congestionsList);
	glCallList(airportsList);
	if(Settings::showInactiveAirports() && zoom < inactiveAirportDotZoomTreshold)
		glCallList(airportsInactiveList);

	glCallList(pilotsList);

	renderLabels();

    //drawCoordinateAxii(); // use this to see where the axii are (x = red, y = green, z = blue)
    glFlush(); // seems to be advisable as I understand it. http://www.opengl.org/sdk/docs/man/xhtml/glFlush.xml

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
    if((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::RightButton)) {
        handleRotation(event);
        mapIsMoving = true;
    }
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
    if(mapIsMoving == true) {
        emit newPosition();
        mapIsMoving = false;
    }
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
            emit hasGuiMessage("Too many objects under cursor");
            return; // area too crowded
        }
    }
    if(countRelevant == 0) {
        emit hasGuiMessage("No object under cursor");
        return;
    }
    if(countRelevant != 1) return; // ambiguous search result

    if(airport != 0) {
        emit hasGuiMessage(QString("Toggled routes for %1").arg(airport->label));
        airport->setDisplayFlightLines(!airport->showFlightLines);
        createPilotsList();
        updateGL();
        return;
    }

    if(pilot != 0) {
        // display flight path for pilot
        emit hasGuiMessage(QString("Toggled route for %1").arg(pilot->label));
        pilot->toggleDisplayPath();
        createPilotsList();
        updateGL();
        return;
    }
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    QToolTip::hideText();
    lastPos = mouseDownPos = QPoint(); // fake to disallow mapClicked()
    double lat, lon;
    if (mouse2latlon(event->x(), event->y(), lat, lon))
        setMapPosition(lat, lon, zoom);
}

void GLWidget::wheelEvent(QWheelEvent* event) {
    QToolTip::hideText();
    //if(event->orientation() == Qt::Vertical) {
    if (abs(event->delta()) > Settings::wheelMax()) { // always recalibrate if bigger values are found
        Settings::setWheelMax(abs(event->delta()));
    }
    zoomIn((double) event->delta() / Settings::wheelMax());
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

/////////////////////////////
// rendering text
//

void GLWidget::renderLabels() {
    fontRectangles.clear();
    allFontRectangles.clear();

    // FIR labels
    QList<MapObject*> objects;
    for(int i = 0; i < sectorsToDraw.size(); i++) objects.append(sectorsToDraw[i]);
    renderLabels(objects, Settings::firFont(), controllerLabelZoomTreshold, Settings::firFontColor());

    // Airport labels
    objects.clear();
    QList<Airport*> airportList = NavData::getInstance()->activeAirports().values(); //ordered by congestion ascending, big airport's labels will always be drawn first
    for(int i = airportList.size() - 1; i > -1; i--) { // from up to down
        if(airportList[i] == 0) continue; // if we look carefully, we should be able to get rid of this
        if(airportList[i]->isActive()) objects.append(airportList[i]);
    }
    renderLabels(objects, Settings::airportFont(), airportLabelZoomTreshold, Settings::airportFontColor());

    // Pilot labels
    objects.clear();
    QList<Pilot*> pilots = Whazzup::getInstance()->whazzupData().getPilots();
    for(int i = 0; i < pilots.size(); i++) objects.append(pilots[i]);
    renderLabels(objects, Settings::pilotFont(), pilotLabelZoomTreshold, Settings::pilotFontColor());

    // Inactive airports
    if(Settings::showInactiveAirports()) { // + inactive labels
        objects.clear();
        foreach(Airport *airport, NavData::getInstance()->airports().values()) {
            if (airport == 0) continue; // if we look carefully, we should be able to get rid of this
            if (!airport->isActive()) objects.append(airport);
        }
        renderLabels(objects, Settings::inactiveAirportFont(), inactiveAirportLabelZoomTreshold, Settings::inactiveAirportFontColor());
    }

    // Fixes
    if(Settings::showFixes()) {
        const Airac& airac = NavData::getInstance()->getAirac();
        const QList<Waypoint*>& fixes = airac.getAllWaypoints();
        objects.clear();
        for(int i = 0; i < fixes.size(); i++) {
            if(fixes[i] == 0) continue; // if we look carefully, we should be able to get rid of this
            objects.append(fixes[i]);
        }
        renderLabels(objects, Settings::inactiveAirportFont(), fixZoomTreshold, Settings::countryLineColor());
    }
}

void GLWidget::renderLabels(const QList<MapObject*>& objects, const QFont& font, double zoomTreshold, QColor color) {
    if(zoom > zoomTreshold)
        return; // don't draw if too far away

    int maxLabels = Settings::maxLabels();
    color.setAlphaF(qMax(0.0, qMin(1.0, (zoomTreshold - zoom) / zoomTreshold * 1.5)));

    QFontMetricsF fontMetrics(font, this);
    for (int i = 0; i < objects.size() && fontRectangles.size() < maxLabels; i++) {
        MapObject *o = objects[i];
        if(o == 0) continue;

        int x, y;
        if(pointIsVisible(o->lat, o->lon, &x, &y)) {
            QString text = o->mapLabel();
            QRectF rect = fontMetrics.boundingRect(text);
            int drawX = x - rect.width() / 2; // center horizontally
            int drawY = y - rect.height() - 5; // some px above dot
            rect.moveTo(drawX, drawY);

            FontRectangle fontRect = FontRectangle(rect, o);
            allFontRectangles.append(fontRect);
            if(shouldDrawLabel(fontRect)) {
                qglColor(color);
                renderText(drawX, (drawY + rect.height()), text, font);
                fontRectangles.append(fontRect);
            }
        }
    }
}

bool GLWidget::shouldDrawLabel(const FontRectangle& rect) {
    int shrinkCheckRectByFactor = 2; // be less conservative in edge overlap-checking
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

QList<MapObject*> GLWidget::objectsAt(int x, int y, double radius) const {
    QList<MapObject*> result;

    // scan text labels
    for(int i = 0; i < allFontRectangles.size(); i++) {
        if(allFontRectangles[i].rect().contains(x, y))
            result.append(allFontRectangles[i].object());
    }

    double lat, lon; if(!mouse2latlon(x, y, lat, lon)) // returns false if not on globe
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

/////////////////////////
// uncategorized
////////////////

void GLWidget::shutDownAnimation() {
	if (shutDownAnim_t != 0)
		qApp->exit(0);
	shutDownAnim_t = QDateTime::currentMSecsSinceEpoch();
	QTimer::singleShot(10, this, SLOT(updateGL()));
}

void GLWidget::newWhazzupData(bool isNew) {
    qDebug() << "GLWidget::newWhazzupData() isNew =" << isNew;
    if(isNew) {
        // update airports
        NavData::getInstance()->updateData(Whazzup::getInstance()->whazzupData());

        sectorsToDraw = Whazzup::getInstance()->whazzupData().activeSectors();
        createPilotsList();
        createAirportsList();
        prepareDisplayLists();

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

void GLWidget::drawCoordinateAxii() { // just for debugging. Visualization of the axii. red=x/green=y/blue=z
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