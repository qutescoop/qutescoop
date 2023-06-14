#include "GLWidget.h"

#include "Controller.h"
#include "dialogs/AirportDetails.h"
#include "dialogs/PlanFlightDialog.h"
#include "dialogs/PilotDetails.h"
#include "helpers.h"
#include "GuiMessage.h"
#include "LineReader.h"
#include "NavData.h"
#include "Pilot.h"
#include "Settings.h"
#include "Waypoint.h"
#include "Whazzup.h"

//#include <GL/glext.h>   // Multitexturing - not platform-independant
#include <algorithm>

GLWidget::GLWidget(QGLFormat fmt, QWidget* parent)
    : QGLWidget(fmt, parent),
      _mapMoving(false), _mapZooming(false), _mapRectSelecting(false),
      _lightsGenerated(false),
      _earthTex(0), _immediateRouteTex(0),
      _earthList(0), _coastlinesList(0), _countriesList(0), _gridlinesList(0),
      _pilotsList(0), _activeAirportsList(0), _inactiveAirportsList(0),
      _usedWaypointsList(0), _sectorPolygonsList(0), _sectorPolygonBorderLinesList(0),
      _congestionsList(0),
      _staticSectorPolygonsList(0), _staticSectorPolygonBorderLinesList(0),
      _hoveredSectorPolygonsList(0), _hoveredSectorPolygonBorderLinesList(0),
      _pilotLabelZoomTreshold(1.5),
      _activeAirportLabelZoomTreshold(2.), _inactiveAirportLabelZoomTreshold(.08),
      _controllerLabelZoomTreshold(2.5),
      _usedWaypointsLabelZoomThreshold(.7),
      _xRot(0), _yRot(0), _zRot(0), _zoom(2), _aspectRatio(1),
      _highlighter(0) {
    setAutoFillBackground(false);
    setMouseTracking(true);

    // call default (=9) map position (without triggering a GuiMessage)
    restorePosition(9, true);

    clientSelection = new ClientSelectionWidget();
}

GLWidget::~GLWidget() {
    glDeleteLists(_earthList, 1); glDeleteLists(_gridlinesList, 1);
    glDeleteLists(_coastlinesList, 1); glDeleteLists(_countriesList, 1);
    glDeleteLists(_usedWaypointsList, 1); glDeleteLists(_pilotsList, 1);
    glDeleteLists(_activeAirportsList, 1); glDeleteLists(_inactiveAirportsList, 1);
    glDeleteLists(_congestionsList, 1);
    glDeleteLists(_sectorPolygonsList, 1); glDeleteLists(_sectorPolygonBorderLinesList, 1);
    glDeleteLists(_staticSectorPolygonsList, 1);
    glDeleteLists(_staticSectorPolygonBorderLinesList, 1);
    glDeleteLists(_hoveredSectorPolygonsList, 1);
    glDeleteLists(_hoveredSectorPolygonBorderLinesList, 1);

    if (_earthTex != 0) {
        deleteTexture(_earthTex);
        //glDeleteTextures(1, &earthTex); // handled Qt'ish by deleteTexture
    }
    if (_immediateRouteTex != 0) {
        deleteTexture(_immediateRouteTex);
    }

    gluDeleteQuadric(_earthQuad);

    delete clientSelection;
}

void GLWidget::setMapPosition(double lat, double lon, double newZoom) {
    _xRot = Helpers::modPositive(270. - lat, 360.);
    _zRot = Helpers::modPositive(-lon, 360.);
    _zoom = newZoom;
    resetZoom();
    update();
}

/**
 * current lat/lon
 **/
QPair<double, double> GLWidget::currentPosition() const {
    return QPair<double, double>(
        Helpers::modPositive(-90. - _xRot + 180., 360.) - 180.,
        Helpers::modPositive(-_zRot + 180., 360.) - 180.
    );
}

void GLWidget::invalidatePilots() {
    m_isPilotsListDirty = true;
    m_isPilotMapObjectsDirty = true;
    m_isUsedWaypointMapObjectsDirty = true;
    update();
}

void GLWidget::invalidateAirports() {
    m_isAirportsListDirty = true;
    m_isAirportsMapObjectsDirty = true;
    m_isUsedWaypointMapObjectsDirty = true;
    update();
}

void GLWidget::invalidateControllers() {
    m_isControllerListsDirty = true;
    m_isControllerMapObjectsDirty = true;
    update();
}

void GLWidget::setStaticSectors(QList<Sector*> sectors) {
    m_staticSectors = sectors;
    m_isStaticSectorListsDirty = true;
    update();
}

/**
 * rotate according to mouse movement
 **/
void GLWidget::handleRotation(QMouseEvent*) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    const double zoomFactor = _zoom / 10.;
    double dx = (currentPos.x() - _lastPos.x()) * zoomFactor;
    double dy = (-currentPos.y() + _lastPos.y()) * zoomFactor;
    _xRot = Helpers::modPositive(_xRot + 180., 360.) - 180.;
    _xRot = qMax(-180., qMin(0., _xRot + dy));
    _zRot = Helpers::modPositive(_zRot + dx + 180., 360.) - 180.;
    _lastPos = currentPos;

    m_fontRectangles.clear();
    update();
}

/**
 * Converts screen mouse coordinates into latitude/longitude of the map.
 * Calculation based on Euler angles.
 * @returns false if x/y is not on the globe
 **/
bool GLWidget::local2latLon(int x, int y, double &lat, double &lon) const {
    // 1) mouse coordinates to Cartesian coordinates of the openGL environment [-1...+1]
    double xGl = (2. * x / width() - 1.) * _aspectRatio * _zoom / 2;
    double zGl = (2. * y / height() - 1.) * _zoom / 2;
    double yGl = sqrt(1 - (xGl * xGl) - (zGl * zGl)); // As the radius of globe is 1
    if (qIsNaN(yGl)) {
        return false; // mouse is not on globe
    }
    // 2) skew (rotation around the x-axis, where 0° means looking onto the equator)
    double theta = (_xRot + 90.) * Pi180;

    // 3) new cartesian coordinates, taking skew into account
    double x0 = -zGl* qSin(theta) + yGl * qCos(theta);
    double z0 = +zGl* qCos(theta) + yGl * qSin(theta);
    double y0 = xGl;

    // 4) now to lat/lon
    lat = qAtan(-z0 / qSqrt(1 - (z0 * z0))) * 180 / M_PI;
    lon = qAtan(-x0 / y0) * 180 / M_PI - 90;

    // 5) qAtan might have lost the sign
    if (xGl >= 0) {
        lon += 180;
    }

    lon = Helpers::modPositive(lon - _zRot + 180., 360.) - 180.;
    return true;
}

void GLWidget::scrollBy(int moveByX, int moveByY) {
    QPair<double, double> cur = currentPosition();
    setMapPosition(
        cur.first - (double) moveByY * _zoom * 6., // 6° on zoom=1
        cur.second + (double) moveByX * _zoom * 6., _zoom
    );
    update();
}

void GLWidget::resetZoom() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // clipping left/right/bottom/top/near/far
    glOrtho(
        -0.5 * _zoom * _aspectRatio,
        +0.5 * _zoom * _aspectRatio,
        +0.5 * _zoom,
        -0.5 * _zoom,
        8,
        10
    );
    // or gluPerspective for perspective viewing
    // gluPerspective(_zoom, _aspectRatio, 8, 10); // just for reference, if you want to try it
    glMatrixMode(GL_MODELVIEW);

    m_fontRectangles.clear();
}

/**
 * Check if a point is visible to the current viewport
 **/
bool GLWidget::latLon2local(double lat, double lon, int* px, int* py) const {
    GLfloat buffer[6];
    glFeedbackBuffer(6, GL_3D, buffer); // create a feedback buffer
    glRenderMode(GL_FEEDBACK); // set to feedback mode
    glBegin(GL_POINTS); // send a point to GL
    VERTEX(lat, lon);
    glEnd();
    if (glRenderMode(GL_RENDER) > 0) { // if the feedback buffer size is zero, the point was clipped
        if (px != 0) {
            *px = (int) buffer[1];
        }
        if (py != 0) {
            *py = height() - (int) buffer[2];
        }
        return true;
    }
    return false;
}

void GLWidget::rememberPosition(int nr) {
    GuiMessages::message(QString("Remembered map position %1").arg(nr));
    Settings::setRememberedMapPosition(_xRot, _yRot, _zRot, _zoom, nr);
}

void GLWidget::restorePosition(int nr, bool isSilent) {
    if (!isSilent) {
        GuiMessages::message(QString("Recalled map position %1").arg(nr));
    }
    Settings::rememberedMapPosition(&_xRot, &_yRot, &_zRot, &_zoom, nr);
    _xRot = Helpers::modPositive(_xRot, 360.);
    _zRot = Helpers::modPositive(_zRot, 360.);
    resetZoom();
    update();
}

const QPair<double, double> GLWidget::sunZenith(const QDateTime &dateTime) const {
    // dirtily approximating present zenith Lat/Lon (where the sun is directly above).
    // scientific solution:
    // http://openmap.bbn.com/svn/openmap/trunk/src/openmap/com/bbn/openmap/layer/daynight/SunPosition.java
    // [sunPosition()] - that would have been at least 100 lines of code...
    return QPair<double, double>(
        -23. * qCos(
            (double) dateTime.date().dayOfYear() /
            (double) dateTime.date().daysInYear() * 2. * M_PI
        ),
        -((double) dateTime.time().hour() +
        (double) dateTime.time().minute() / 60.) * 15. - 180.
    );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Methods preparing displayLists
//
void GLWidget::createPilotsList() {
    qDebug();

    if (_pilotsList == 0) {
        _pilotsList = glGenLists(1);
    }

    glNewList(_pilotsList, GL_COMPILE);

    QList<Pilot*> pilots = Whazzup::instance()->whazzupData().pilots.values();

    // leader lines
    if (Settings::timelineSeconds() > 0 && !qFuzzyIsNull(Settings::timeLineStrength())) {
        glLineWidth(Settings::timeLineStrength());
        glBegin(GL_LINES);
        qglColor(Settings::leaderLineColor());
        foreach (const Pilot* p, pilots) {
            if (p->groundspeed < 30) {
                continue;
            }

            if (qFuzzyIsNull(p->lat) && qFuzzyIsNull(p->lon)) {
                continue;
            }

            VERTEX(p->lat, p->lon);
            QPair<double, double> pos = p->positionInFuture(Settings::timelineSeconds());
            VERTEX(pos.first, pos.second);
        }
        glEnd();
    }

    // flight paths, also for booked flights
    if (m_isUsedWaypointMapObjectsDirty) {
        m_usedWaypointMapObjects.clear();

        foreach (Pilot* p, Whazzup::instance()->whazzupData().allPilots()) {
            if (qFuzzyIsNull(p->lat) && qFuzzyIsNull(p->lon)) {
                continue;
            }

            if (!p->showDepLine() && !p->showDestLine()) {
                continue;
            }

            QList<Waypoint*> waypoints = p->routeWaypointsWithDepDest();
            int next = p->nextPointOnRoute(waypoints);

            QList<DoublePair> points; // these are the points that really get drawn

            // Dep -> plane
            if (p->showDepLine() && !qFuzzyIsNull(Settings::depLineStrength()) && !Settings::onlyShowImmediateRoutePart()) {
                for (int i = 0; i < next; i++) {
                    if (!m_usedWaypointMapObjects.contains(waypoints[i])) {
                        m_usedWaypointMapObjects.append(waypoints[i]);
                    }
                    points.append(DoublePair(waypoints[i]->lat, waypoints[i]->lon));
                }

                // draw to plane
                points.append(DoublePair(p->lat, p->lon));
                if (Settings::depLineDashed()) {
                    glLineStipple(3, 0xAAAA);
                }
                qglColor(Settings::depLineColor());
                glLineWidth(Settings::depLineStrength());
                glBegin(GL_LINE_STRIP);
                NavData::plotGreatCirclePoints(points);
                points.clear();
                glEnd();
                if (Settings::depLineDashed()) {
                    glLineStipple(1, 0xFFFF);
                }
            }

            points.append(DoublePair(p->lat, p->lon));

            // plane -> Dest
            if (p->showDestLine() && next < waypoints.size()) {
                // immediate
                auto destImmediateNm = p->groundspeed * (Settings::destImmediateDurationMin() / 60.);

                auto lastPoint = DoublePair(p->lat, p->lon);
                double distanceFromPlane = 0;
                int i = next;
                if (!qFuzzyIsNull(Settings::destImmediateLineStrength())) {
                    for (; i < waypoints.size(); i++) {
                        double distance = NavData::distance(lastPoint.first, lastPoint.second, waypoints[i]->lat, waypoints[i]->lon);
                        if (distanceFromPlane + distance < destImmediateNm) {
                            if (!m_usedWaypointMapObjects.contains(waypoints[i])) {
                                m_usedWaypointMapObjects.append(waypoints[i]);
                            }
                            const auto _p = DoublePair(waypoints[i]->lat, waypoints[i]->lon);
                            if (!points.contains(_p)) { // very cautious for duplicates here
                                points.append(_p);
                            }
                            distanceFromPlane += distance;
                            lastPoint = DoublePair(waypoints[i]->lat, waypoints[i]->lon);
                            continue;
                        }

                        if (!points.contains(lastPoint)) {
                            points.append(lastPoint);
                        }
                        const float neededFraction = (destImmediateNm - distanceFromPlane) / qMax(distance, 1.);
                        const auto absoluteLast = NavData::greatCircleFraction(lastPoint.first, lastPoint.second, waypoints[i]->lat, waypoints[i]->lon, neededFraction);
                        if (!points.contains(absoluteLast)) {
                            points.append(absoluteLast);
                        }
                        break;
                    }

                    glPushAttrib(GL_ENABLE_BIT);
                    if (Settings::onlyShowImmediateRoutePart()) {
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                        glEnable(GL_TEXTURE_1D);
                        glBindTexture(GL_TEXTURE_1D, _immediateRouteTex);
                    }
                    qglColor(Settings::destImmediateLineColor());
                    glLineWidth(Settings::destImmediateLineStrength());
                    glBegin(GL_LINE_STRIP);
                    NavData::plotGreatCirclePoints(points);
                    glEnd();
                    glPopAttrib();
                }

                // rest
                if (!qFuzzyIsNull(Settings::destLineStrength()) && !Settings::onlyShowImmediateRoutePart()) {
                    while (points.size() > 1) {
                        points.takeFirst();
                    }
                    for (; i < waypoints.size(); i++) {
                        if (!m_usedWaypointMapObjects.contains(waypoints[i])) {
                            m_usedWaypointMapObjects.append(waypoints[i]);
                        }
                        points.append(DoublePair(waypoints[i]->lat, waypoints[i]->lon));
                    }
                    qglColor(Settings::destLineColor());
                    if (Settings::destLineDashed()) {
                        glLineStipple(3, 0xAAAA);
                    }
                    glLineWidth(Settings::destLineStrength());
                    glBegin(GL_LINE_STRIP);
                    NavData::plotGreatCirclePoints(points);
                    glEnd();

                    if (Settings::destLineDashed()) {
                        glLineStipple(1, 0xFFFF);
                    }
                }
            }
        }
        m_isUsedWaypointMapObjectsDirty = false;
    }

    // aircraft dots
    if (!qFuzzyIsNull(Settings::pilotDotSize())) {
        glPointSize(Settings::pilotDotSize());
        qglColor(Settings::pilotDotColor());
        glBegin(GL_POINTS);
        foreach (const Pilot* p, pilots) {
            if (qFuzzyIsNull(p->lat) && qFuzzyIsNull(p->lon)) {
                continue;
            }
            if (!p->isFriend()) {
                VERTEX(p->lat, p->lon);
            }
        }
        glEnd();

        // friends
        qglColor(Settings::friendsPilotDotColor());
        glPointSize(Settings::pilotDotSize() * 1.3);
        glBegin(GL_POINTS);
        foreach (const Pilot* p, pilots) {
            if (qFuzzyIsNull(p->lat) && qFuzzyIsNull(p->lon)) {
                continue;
            }

            if (p->isFriend()) {
                VERTEX(p->lat, p->lon);
            }
        }
        glEnd();
    }

    // planned route from Flightplan Dialog (does not really belong to pilots lists, but is convenient here)
    // @todo
    if (PlanFlightDialog::instance(false) != 0) {
        PlanFlightDialog::instance()->plotPlannedRoute();
    }

    glEndList();

    // waypoints used in routes (dots)
    if (_usedWaypointsList == 0) {
        _usedWaypointsList = glGenLists(1);
    }

    if (
        Settings::showUsedWaypoints()
        && !qFuzzyIsNull(Settings::waypointsDotSize())
    ) {
        glNewList(_usedWaypointsList, GL_COMPILE);
        qglColor(Settings::waypointsDotColor());
        glPointSize(Settings::waypointsDotSize());
        glBegin(GL_POINTS);
        foreach (const auto wp, m_usedWaypointMapObjects) {
            VERTEX(wp->lat, wp->lon);
        }
        glEnd();
        glEndList();
    }

    qDebug() << "-- finished";
}

void GLWidget::createAirportsList() {
    qDebug();
    if (_activeAirportsList == 0) {
        _activeAirportsList = glGenLists(1);
    }
    QList<Airport*> airportList = NavData::instance()->airports.values();

    // inactive airports
    if (_inactiveAirportsList == 0) {
        _inactiveAirportsList = glGenLists(1);
    }
    glNewList(_inactiveAirportsList, GL_COMPILE);
    if (Settings::showInactiveAirports() && !qFuzzyIsNull(Settings::inactiveAirportDotSize())) {
        glPointSize(Settings::inactiveAirportDotSize());
        qglColor(Settings::inactiveAirportDotColor());
        glBegin(GL_POINTS);
        foreach (const Airport* a, airportList) {
            if (!a->active) {
                VERTEX(a->lat, a->lon);
            }
        }
        glEnd();
    }
    glEndList();

    // active airports
    glNewList(_activeAirportsList, GL_COMPILE);
    if (!qFuzzyIsNull(Settings::airportDotSize())) {
        glPointSize(Settings::airportDotSize());
        qglColor(Settings::airportDotColor());
        glBegin(GL_POINTS);
        foreach (const Airport* a, airportList) {
            if (a->active) {
                VERTEX(a->lat, a->lon);
            }
        }
        glEnd();

        // friends
        glPointSize(Settings::airportDotSize() * 1.3);
        qglColor(Settings::friendsAirportDotColor());
        glBegin(GL_POINTS);
        foreach (const Airport* a, airportList) {
            if (a->active) {
                foreach (const auto c, a->allControllers()) {
                    if (c->isFriend()) {
                        VERTEX(a->lat, a->lon);
                        break;
                    }
                }
            }
        }
        glEnd();
    }
    glEndList();

    // airport congestion based on filtered traffic
    if (_congestionsList == 0) {
        _congestionsList = glGenLists(1);
    }
    glNewList(_congestionsList, GL_COMPILE);
    if (Settings::showAirportCongestion()) {
        glPushAttrib(GL_ENABLE_BIT);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, _immediateRouteTex);
        foreach (const Airport* a, airportList) {
            Q_ASSERT(a != 0);
            if (!a->active) {
                continue;
            }
            int congestion = a->congestion();
            if (congestion < Settings::airportCongestionMovementsMin()) {
                continue;
            }
            const GLdouble circle_distort = qCos(a->lat * Pi180);
            if (qFuzzyIsNull(circle_distort)) {
                continue;
            }

            const float fraction = qMin<float>(
                1.,
                Helpers::fraction(
                    Settings::airportCongestionMovementsMin(),
                    Settings::airportCongestionMovementsMax(),
                    a->congestion()
                )
            );
            const GLfloat distanceNm = Helpers::lerp(
                Settings::airportCongestionRadiusMin(),
                Settings::airportCongestionRadiusMax(),
                fraction
            );
            QList<QPair<double, double> > points;
            for (int h = 0; h <= 360; h += 6) {
                double x = a->lat + Nm2Deg(distanceNm) * qCos(h * Pi180);
                double y = a->lon + Nm2Deg(distanceNm) / circle_distort * qSin(h * Pi180);
                points.append(QPair<double, double>(x, y));
            }
            qglColor(
                Helpers::mixColor(
                    Settings::airportCongestionColorMin(),
                    Settings::airportCongestionColorMax(),
                    fraction
                )
            );
            if (Settings::showAirportCongestionGlow()) {
                glBegin(GL_TRIANGLE_FAN);
                glTexCoord1f(0.);
                VERTEX(a->lat, a->lon);
                for (int h = 0; h < points.size(); h++) {
                    glTexCoord1f(1.);
                    VERTEX(points[h].first, points[h].second);
                }
                glEnd();
            }
            if (Settings::showAirportCongestionRing()) {
                glLineWidth(
                    Helpers::lerp(
                        Settings::airportCongestionBorderLineStrengthMin(),
                        Settings::airportCongestionBorderLineStrengthMax(),
                        fraction
                    )
                );
                glBegin(GL_LINE_LOOP);
                for (int h = 0; h < points.size(); h++) {
                    glTexCoord1f(0.);
                    VERTEX(points[h].first, points[h].second);
                }
                glEnd();

                // or as a Torus:
                if (false) {
                    glBegin(GL_TRIANGLE_STRIP);
                    for (int h = 0; h <= 360; h += 6) {
                        glTexCoord1f(-1.);
                        VERTEX(
                            a->lat + Nm2Deg(distanceNm) * qCos(h * Pi180),
                            a->lon + Nm2Deg(distanceNm) / circle_distort * qSin(h * Pi180)
                        );
                        glTexCoord1f(1.);
                        VERTEX(
                            a->lat + Nm2Deg(distanceNm * 2) * qCos(h * Pi180),
                            a->lon + Nm2Deg(distanceNm * 2) / circle_distort * qSin(h * Pi180)
                        );
                    }
                    glEnd();
                }
            }
        }
        glPopAttrib();
    }
    glEndList();
    qDebug() << "-- finished";
}

void GLWidget::createControllerLists() {
    qDebug();

    // FIR polygons
    if (_sectorPolygonsList == 0) {
        _sectorPolygonsList = glGenLists(1);
    }

    auto _sectorsToDraw = Whazzup::instance()->whazzupData().controllersWithSectors();

    // make sure all the lists are there to avoid nested glNewList calls
    foreach (const Controller* c, _sectorsToDraw) {
        if (c->sector != 0) {
            c->sector->glPolygon();
        }
    }

    // create a list of lists
    glNewList(_sectorPolygonsList, GL_COMPILE);
    foreach (const Controller* c, _sectorsToDraw) {
        if (c->sector != 0) {
            glCallList(c->sector->glPolygon());
        }
    }
    glEndList();

    // FIR borders
    if (_sectorPolygonBorderLinesList == 0) {
        _sectorPolygonBorderLinesList = glGenLists(1);
    }

    if (!qFuzzyIsNull(Settings::firBorderLineStrength())) {
        // first, make sure all lists are there
        foreach (const Controller* c, _sectorsToDraw) {
            if (c->sector != 0) {
                c->sector->glBorderLine();
            }
        }
        glNewList(_sectorPolygonBorderLinesList, GL_COMPILE);
        foreach (const Controller* c, _sectorsToDraw) {
            if (c->sector != 0) {
                glCallList(c->sector->glBorderLine());
            }
        }
        glEndList();
    }
    qDebug() << "-- finished";
}


void GLWidget::createHoveredControllersLists(QSet<Controller*> controllers) {
    // make sure all the lists are there to avoid nested glNewList calls
    foreach (Controller* c, controllers) {
        if (c->sector != 0) {
            c->sector->glPolygonHighlighted();
        } else if (c->isAppDep()) {
            foreach (const auto _a, c->airports()) {
                _a->appDisplayList();
            }
        } else if (c->isTwr()) {
            foreach (const auto _a, c->airports()) {
                _a->twrDisplayList();
            }
        } else if (c->isGnd()) {
            foreach (const auto _a, c->airports()) {
                _a->gndDisplayList();
            }
        } else if (c->isDel()) {
            foreach (const auto _a, c->airports()) {
                _a->delDisplayList();
            }
        }
    }

    // create a list of lists
    if (_hoveredSectorPolygonsList == 0) {
        _hoveredSectorPolygonsList = glGenLists(1);
    }
    glNewList(_hoveredSectorPolygonsList, GL_COMPILE);
    foreach (Controller* c, controllers) {
        if (c->sector != 0) {
            glCallList(c->sector->glPolygonHighlighted());
        } else if (c->isAppDep()) {
            foreach (const auto _a, c->airports()) {
                glCallList(_a->appDisplayList());
            }
        } else if (c->isTwr()) {
            foreach (const auto _a, c->airports()) {
                glCallList(_a->twrDisplayList());
            }
        } else if (c->isGnd()) {
            foreach (const auto _a, c->airports()) {
                glCallList(_a->gndDisplayList());
            }
        } else if (c->isDel()) {
            foreach (const auto _a, c->airports()) {
                glCallList(_a->delDisplayList());
            }
        }
    }
    glEndList();


    // FIR borders
    if (_hoveredSectorPolygonBorderLinesList == 0) {
        _hoveredSectorPolygonBorderLinesList = glGenLists(1);
    }

    if (!qFuzzyIsNull(Settings::firHighlightedBorderLineStrength())) {
        // first, make sure all lists are there
        foreach (Controller* c, controllers) {
            if (c->sector != 0) {
                c->sector->glBorderLineHighlighted();
            }
        }
        glNewList(_hoveredSectorPolygonBorderLinesList, GL_COMPILE);
        foreach (Controller* c, controllers) {
            if (c->sector != 0) {
                glCallList(c->sector->glBorderLineHighlighted());
            }
        }
        glEndList();
    }
}

void GLWidget::createStaticLists() {
    // earth
    qDebug() << "Generating quadric texture coordinates";
    _earthQuad = gluNewQuadric();
    gluQuadricTexture(_earthQuad, GL_TRUE); // prepare texture coordinates
    gluQuadricDrawStyle(_earthQuad, GLU_FILL); // FILL, LINE, SILHOUETTE or POINT
    gluQuadricNormals(_earthQuad, GLU_SMOOTH); // NONE, FLAT or SMOOTH
    gluQuadricOrientation(_earthQuad, GLU_OUTSIDE); // GLU_INSIDE

    parseTexture();

    if (_immediateRouteTex == 0) {
        glGenTextures(1, &_immediateRouteTex);
        glBindTexture(GL_TEXTURE_1D, _immediateRouteTex);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        const char components = 4;
        GLubyte buf[32 * components];
        for (size_t i = 0; i < sizeof(buf); i += components) {
            GLfloat fraction = i / (GLfloat) (sizeof(buf) - components);
            GLfloat result = qCos(fraction * M_PI / 2.); // ease out sine
            const GLubyte grey = 255 * result;
            buf[i + 0] = 255; // rand() % 255; // for testing with a rainbow
            buf[i + 1] = 255; // rand() % 255;
            buf[i + 2] = 255; // rand() % 255;
            buf[i + 3] = grey;
        }
        glTexImage1D(GL_TEXTURE_1D, 0, components, sizeof(buf) / components, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    }

    _earthList = glGenLists(1);
    glNewList(_earthList, GL_COMPILE);
    qglColor(Settings::globeColor());
    gluSphere(
        _earthQuad, 1, qRound(360. / Settings::glCirclePointEach()), // draw a globe with..
        qRound(180. / Settings::glCirclePointEach())
    ); // ..radius, slicesX, stacksZ
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
    qDebug() << "gridLines";
    _gridlinesList = glGenLists(1);
    glNewList(_gridlinesList, GL_COMPILE);
    if (!qFuzzyIsNull(Settings::gridLineStrength())) {
        // meridians
        qglColor(Settings::gridLineColor());
        glLineWidth(Settings::gridLineStrength());
        for (int lon = 0; lon < 180; lon += Settings::earthGridEach()) {
            glBegin(GL_LINE_LOOP);
            for (int lat = 0; lat < 360; lat += Settings::glCirclePointEach()) {
                VERTEX(lat, lon);
            }
            glEnd();
        }
        // parallels
        for (int lat = -90 + Settings::earthGridEach(); lat < 90; lat += Settings::earthGridEach()) {
            glBegin(GL_LINE_LOOP);
            for (
                int lon = -180; lon < 180;
                lon += qCeil(Settings::glCirclePointEach() / qCos(lat * Pi180))
            ) {
                VERTEX(lat, lon);
            }
            glEnd();
        }
    }
    glEndList();

    // coastlines
    qDebug() << "coastLines";
    _coastlinesList = glGenLists(1);
    glNewList(_coastlinesList, GL_COMPILE);
    if (!qFuzzyIsNull(Settings::coastLineStrength())) {
        qglColor(Settings::coastLineColor());
        glLineWidth(Settings::coastLineStrength());
        LineReader lineReader(Settings::dataDirectory("data/coastline.dat"));
        QList<QPair<double, double> > line = lineReader.readLine();
        while (!line.isEmpty()) {
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i < line.size(); i++) {
                VERTEX(line[i].first, line[i].second);
            }
            glEnd();
            line = lineReader.readLine();
        }
    }
    glEndList();

    // countries
    qDebug() << "countries";
    _countriesList = glGenLists(1);
    glNewList(_countriesList, GL_COMPILE);
    if (!qFuzzyIsNull(Settings::countryLineStrength())) {
        qglColor(Settings::countryLineColor());
        glLineWidth(Settings::countryLineStrength());
        LineReader countries = LineReader(Settings::dataDirectory("data/countries.dat"));
        QList<QPair<double, double> > line = countries.readLine();
        while (!line.isEmpty()) {
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i < line.size(); i++) {
                VERTEX(line[i].first, line[i].second);
            }
            glEnd();
            line = countries.readLine();
        }
    }
    glEndList();
}

void GLWidget::createStaticSectorLists() {
    //Polygon
    if (_staticSectorPolygonsList == 0) {
        _staticSectorPolygonsList = glGenLists(1);
    }

    // make sure all the lists are there to avoid nested glNewList calls
    foreach (Sector* sector, m_staticSectors) {
        if (sector != 0) {
            sector->glPolygon();
        }
    }

    // create a list of lists
    glNewList(_staticSectorPolygonsList, GL_COMPILE);
    foreach (Sector* sector, m_staticSectors) {
        if (sector != 0) {
            glCallList(sector->glPolygon());
        }
    }
    glEndList();


    // FIR borders
    if (_staticSectorPolygonBorderLinesList == 0) {
        _staticSectorPolygonBorderLinesList = glGenLists(1);
    }

    if (!qFuzzyIsNull(Settings::firBorderLineStrength())) {
        // first, make sure all lists are there
        foreach (Sector* sector, m_staticSectors) {
            if (sector != 0) {
                sector->glBorderLine();
            }
        }

        glNewList(_staticSectorPolygonBorderLinesList, GL_COMPILE);
        foreach (Sector* sector, m_staticSectors) {
            if (sector != 0) {
                glCallList(sector->glBorderLine());
            }
        }
        glEndList();
    }
}

/**
 * Sets up OpenGL environment and prepares static artefacts for quick access later.
 * Call drawCoordinateAxii() inside paintGL() to see where the axii are.
 **/
void GLWidget::initializeGL() {
    qDebug() << "OpenGL support: " << context()->format().hasOpenGL()
             << "; 1.1:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_1_1)
             << "; 1.2:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_1_2)
             << "; 1.3:" << format().openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_1_3)     // multitexturing
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

    if (format().sampleBuffers() && format().samples() > 1) {
        glEnable(GL_MULTISAMPLE);
        qDebug() << "MSAA: Multi-sample anti-aliasing enabled using" << format().samples() << "sample buffers";
    } else {
        qWarning() << "MSAA: Multi-sample anti-aliasing has NOT been enabled. Things won't look so nice.";
    }

    qglClearColor(Settings::backgroundColor());

    if (Settings::glStippleLines()) {
        glEnable(GL_LINE_STIPPLE);
    } else {
        glDisable(GL_LINE_STIPPLE);
    }
    if (Settings::displaySmoothDots()) {
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST); // GL_FASTEST, GL_NICEST, GL_DONT_CARE
    } else {
        glDisable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST); // GL_FASTEST, GL_NICEST, GL_DONT_CARE
    }
    if (Settings::displaySmoothLines()) {
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // GL_FASTEST, GL_NICEST, GL_DONT_CARE
    } else {
        glDisable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST); // GL_FASTEST, GL_NICEST, GL_DONT_CARE
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
        glFogf(GL_FOG_START, (GLfloat) 9.8);
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
     * AMBIENT - light that comes from all directions equally and is scattered in all directions equally by the
     * polygons
     *  in your scene. This isn't quite true of the real world - but it's a good first approximation for light that
     * comes
     *  pretty much uniformly from the sky and arrives onto a surface by bouncing off so many other surfaces that it
     * might
     *  as well be uniform.
     * DIFFUSE - light that comes from a particular point source (like the Sun) and hits surfaces with an intensity
     *  that depends on whether they face towards the light or away from it. However, once the light radiates from
     * the
     *  surface, it does so equally in all directions. It is diffuse lighting that best defines the shape of 3D
     * objects.
     * SPECULAR - as with diffuse lighting, the light comes from a point souce, but with specular lighting, it is
     * reflected
     *  more in the manner of a mirror where most of the light bounces off in a particular direction defined by the
     * surface
     *  shape. Specular lighting is what produces the shiney highlights and helps us to distinguish between flat,
     * dull
     *  surfaces such as plaster and shiney surfaces like polished plastics and metals.
     * EMISSION - in this case, the light is actually emitted by the polygon - equally in all directions.
     *                 */

    if (Settings::glLighting()) {
        //const GLfloat earthAmbient[]  = {0, 0, 0, 1};
        const GLfloat earthDiffuse[] = { 1, 1, 1, 1 };
        const GLfloat earthSpecular[] = {
            (GLfloat) Settings::specularColor().redF(),
            (GLfloat) Settings::specularColor().greenF(),
            (GLfloat) Settings::specularColor().blueF(),
            (GLfloat) Settings::specularColor().alphaF()
        };
        const GLfloat earthEmission[] = { 0, 0, 0, 1 };
        const GLfloat earthShininess[] = { (GLfloat) Settings::earthShininess() };
        //glMaterialfv(GL_FRONT, GL_AMBIENT, earthAmbient); // GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
        glMaterialfv(GL_FRONT, GL_DIFFUSE, earthDiffuse); // ...GL_EMISSION, GL_SHININESS,
                                                          // GL_AMBIENT_AND_DIFFUSE,
        glMaterialfv(GL_FRONT, GL_SPECULAR, earthSpecular); // ...GL_COLOR_INDEXES
        glMaterialfv(GL_FRONT, GL_EMISSION, earthEmission); //
        glMaterialfv(GL_FRONT, GL_SHININESS, earthShininess); //... only DIFFUSE has an own alpha channel!
        glColorMaterial(GL_FRONT, GL_AMBIENT); // GL_EMISSION, GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
                                               // GL_AMBIENT_AND_DIFFUSE
        glEnable(GL_COLOR_MATERIAL); // controls if glColor will drive the given values in glColorMaterial

        const GLfloat sunAmbient[] = { 0., 0., 0., 1. };
        QColor adjustSunDiffuse = Settings::sunLightColor();
        if (Settings::glLights() > 1) {
            adjustSunDiffuse = adjustSunDiffuse.darker(
                100. * (Settings::glLights() - // reduce light intensity
                                               // by number of lights...
                Settings::glLightsSpread() / 180. *
                (Settings::glLights() - 1))
            ); // ...and increase again
        }
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
                glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, sunDiffuse); // ...GL_SPOT_DIRECTION,
                                                                      // GL_SPOT_EXPONENT,
                // GL_CONSTANT_ATTENUATION,
                //glLightfv(GL_LIGHT0 + light, GL_SPECULAR, sunSpecular);// ...GL_LINEAR_ATTENUATION
                // GL_QUADRATIC_ATTENUATION
                glEnable(GL_LIGHT0 + light);
            } else {
                glDisable(GL_LIGHT0 + light);
            }
        }
        const GLfloat modelAmbient[] = { (GLfloat) .2, (GLfloat) .2, (GLfloat) .2, (GLfloat) 1. }; // the
                                                                                                   // "background"
                                                                                                   // ambient
                                                                                                   // light
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmbient); // GL_LIGHT_MODEL_AMBIENT,
                                                              // GL_LIGHT_MODEL_COLOR_CONTROL,
        //glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // ...GL_LIGHT_MODEL_LOCAL_VIEWER,
        // GL_LIGHT_MODEL_TWO_SIDE

        glShadeModel(GL_SMOOTH); // SMOOTH or FLAT
        glEnable(GL_NORMALIZE);
        _lightsGenerated = true;
    }

    createStaticLists();
    qDebug() << "-- finished";
}

/**
 * gets called whenever a screen refresh is needed. If you want to schedule a repaint,
 * call update().
 */
void GLWidget::paintGL() {
    qint64 started = QDateTime::currentMSecsSinceEpoch(); // for method execution time calculation.

    // create lists (if necessary)
    if (m_isPilotsListDirty) {
        createPilotsList();
        m_isPilotsListDirty = false;
    }
    if (m_isControllerListsDirty) {
        createControllerLists();
        m_isControllerListsDirty = false;
    }
    if (m_isAirportsListDirty) {
        createAirportsList();
        m_isAirportsListDirty = false;
    }
    if (m_isStaticSectorListsDirty) {
        createStaticSectorLists();
        m_isStaticSectorListsDirty = false;
    }
    createHoveredControllersLists(m_hoveredControllers);

    // blank out the screen (buffered, of course)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0, 0, -10);
    glRotated(_xRot, 1, 0, 0);
    glRotated(_yRot, 0, 1, 0);
    glRotated(_zRot, 0, 0, 1);

    if (Settings::glLighting()) {
        if (!_lightsGenerated) {
            createLights();
        }

        glEnable(GL_LIGHTING);
        // moving sun's position
        QPair<double, double> zenith = sunZenith(
            Whazzup::instance()->whazzupData().whazzupTime.isValid()?
            Whazzup::instance()->whazzupData().whazzupTime:
            QDateTime::currentDateTimeUtc()
        );
        GLfloat sunVertex0[] = { SX(zenith.first, zenith.second), SY(zenith.first, zenith.second),
                                 SZ(zenith.first, zenith.second), 0 }; // sun has parallel light -> dist=0
        glLightfv(GL_LIGHT0, GL_POSITION, sunVertex0); // light 0 always has the real (center) position
        if (Settings::glLights() > 1) {
            for (int light = 1; light < Settings::glLights(); light++) { // setting the other lights'
                                                                         // positions
                double fraction = 2 * M_PI / (Settings::glLights() - 1) * light;
                double spreadLat = zenith.first + qSin(fraction) * Settings::glLightsSpread();
                double spreadLon = zenith.second + qCos(fraction) * Settings::glLightsSpread();
                GLfloat sunVertex[] = { SX(spreadLat, spreadLon), SY(spreadLat, spreadLon), SZ(spreadLat, spreadLon), 0 };
                glLightfv(GL_LIGHT0 + light, GL_POSITION, sunVertex); // GL_LIGHTn is conveniently
                                                                      // GL_LIGHT0 + n
            }
        }
    }
    if (Settings::glTextures() && _earthTex != 0 && Settings::glLighting()) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE, GL_DECAL, GL_BLEND,
                                                                     // GL_REPLACE
        glBindTexture(GL_TEXTURE_2D, _earthTex);
    }
    if (Settings::glTextures() && _earthTex != 0 && !Settings::glLighting()) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE, GL_DECAL, GL_BLEND,
                                                                    // GL_REPLACE
        glBindTexture(GL_TEXTURE_2D, _earthTex);
    }


    glCallList(_earthList);
    if (Settings::glLighting()) {
        glDisable(GL_LIGHTING); // disable lighting after drawing earth...
    }
    if (Settings::glTextures() && _earthTex != 0) { // disable textures after drawing earth...
        glDisable(GL_TEXTURE_2D);
    }

    glCallList(_coastlinesList);
    glCallList(_countriesList);
    glCallList(_gridlinesList);

    if (Settings::showAirportCongestion()) {
        glCallList(_congestionsList);
    }

    if (Settings::showUsedWaypoints() && _zoom < _usedWaypointsLabelZoomThreshold * .1) {
        glCallList(_usedWaypointsList);
    }

    // render sectors
    if (Settings::showCTR()) {
        glCallList(_sectorPolygonsList);
        glCallList(_sectorPolygonBorderLinesList);
    }

    // render hovered sectors
    if (m_hoveredControllers.size() > 0) {
        glCallList(_hoveredSectorPolygonsList);
        glCallList(_hoveredSectorPolygonBorderLinesList);
    }

    // render sectors independently from Whazzup
    if (m_staticSectors.size() > 0) {
        glCallList(_staticSectorPolygonsList);
        glCallList(_staticSectorPolygonBorderLinesList);
    }

    QList<Airport*> airportList = NavData::instance()->airports.values();
    // render Approach
    if (Settings::showAPP()) {
        foreach (Airport* a, airportList) {
            if (!a->appDeps.isEmpty()) {
                glCallList(a->appDisplayList());
            }
        }
    }

    // render Tower
    if (Settings::showTWR()) {
        foreach (Airport* a, airportList) {
            if (!a->twrs.isEmpty()) {
                glCallList(a->twrDisplayList());
            }
        }
    }

    // render Ground/Delivery
    if (Settings::showGND()) {
        foreach (Airport* a, airportList) {
            if (!a->dels.isEmpty()) {
                glCallList(a->delDisplayList());
            }
            if (!a->gnds.isEmpty()) {
                glCallList(a->gndDisplayList());
            }
        }
    }

    glCallList(_activeAirportsList);
    if (Settings::showInactiveAirports() && (_zoom < _inactiveAirportLabelZoomTreshold * .7)) {
        glCallList(_inactiveAirportsList);
    }

    // render pilots
    glCallList(_pilotsList);


    // highlight friends
    if (Settings::highlightFriends()) {
        if (_highlighter == 0) {
            createFriendHighlighter();
        }
        QTime time = QTime::currentTime();
        double range = (time.second() % 5);
        range += (time.msec() % 500) / 1000;

        double lineWidth = Settings::highlightLineWidth();
        if (!Settings::useHighlightAnimation()) {
            range = 0;
            destroyFriendHighlighter();
        }

        foreach (const auto &_friend, m_friendPositions) {
            if (qFuzzyIsNull(_friend.first) && qFuzzyIsNull(_friend.second)) {
                continue;
            }

            glLineWidth(lineWidth);
            qglColor(Settings::friendsHighlightColor());
            glBegin(GL_LINE_LOOP);
            GLdouble circle_distort = qCos(_friend.first * Pi180);
            for (int i = 0; i <= 360; i += 10) {
                double x = _friend.first + Nm2Deg((80 - (range * 20))) * circle_distort * qCos(i * Pi180);
                double y = _friend.second + Nm2Deg((80 - (range * 20))) * qSin(i * Pi180);
                VERTEX(x, y);
            }
            glEnd();
        }
    }

    // render labels
    renderLabels();

    // selection rectangle
    if (_mapRectSelecting) {
        drawSelectionRectangle();
    }

    // some preparations to draw textures (symbols, ...).
    // drawTestTextures();

    // drawCoordinateAxii(); // debug: see axii (x = red, y = green, z = blue)

    if (Settings::showFps()) {
        static bool frameToggle = false;
        frameToggle = !frameToggle;
        const float fps = 1000. / (QDateTime::currentMSecsSinceEpoch() - started);
        qglColor(Settings::firFontColor());
        renderText(0, height() - 2, QString("%1 fps %2").arg(fps, 0, 'f', 0).arg(frameToggle? '*': ' '), Settings::firFont());
    }
    glFlush(); // http://www.opengl.org/sdk/docs/man/xhtml/glFlush.xml
}

void GLWidget::resizeGL(int width, int height) {
    _aspectRatio = (double) width / (double) height;
    glViewport(0, 0, width, height);
    resetZoom();
}

////////////////////////////////////////////////////////////
// SLOTS: mouse, key... and general user-map interaction
//
void GLWidget::mouseMoveEvent(QMouseEvent* event) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    auto newHoveredObjects = objectsAt(currentPos.x(), currentPos.y());
    if (newHoveredObjects != m_hoveredObjects) {
        // remove from fontRectangles when not hovered an more
        foreach (const auto &o, m_hoveredObjects) {
            if (!newHoveredObjects.contains(o)) {
                foreach (const auto &fr, m_fontRectangles) {
                    if (fr.object == o) {
                        m_fontRectangles.remove(fr);
                        break;
                    }
                }
            }
        }
        m_hoveredObjects = newHoveredObjects;
        bool hasPrimaryFunction = false;
        foreach (const auto &o, m_hoveredObjects) {
            if (o->hasPrimaryAction()) {
                hasPrimaryFunction = true;
                break;
            }
        }
        setCursor(hasPrimaryFunction? Qt::PointingHandCursor: Qt::ArrowCursor);
        update();
    }

    if (
        event->buttons().testFlag(Qt::RightButton) // check before left button if useSelectionRectangle=off
        || (!Settings::useSelectionRectangle() && event->buttons().testFlag(Qt::LeftButton))
    ) { // rotate
        _mapMoving = true;
        handleRotation(event);
    } else if (event->buttons().testFlag(Qt::MiddleButton)) { // zoom
        _mapZooming = true;
        zoomIn((currentPos.x() - _lastPos.x() - currentPos.y() + _lastPos.y()) / 100. * Settings::zoomFactor());
        _lastPos = currentPos;
    } else if (event->buttons().testFlag(Qt::LeftButton)) { // selection rectangle
        _mapRectSelecting = true;
        update();
    }

    double lat, lon;
    if (local2latLon(currentPos.x(), currentPos.y(), lat, lon)) {
        QSet<Controller*> _newHoveredControllers;
        foreach (Controller* c, Whazzup::instance()->whazzupData().controllers.values()) {
            if (c->sector != 0 && c->sector->containsPoint(QPointF(lat, lon))) {
                _newHoveredControllers.insert(c);
            } else { // APP, TWR, GND, DEL
                int maxDist_nm = -1;
                if (c->isAppDep()) {
                    maxDist_nm = Airport::symbologyAppRadius_nm;
                } else if (c->isTwr()) {
                    maxDist_nm = Airport::symbologyTwrRadius_nm;
                } else if (c->isGnd()) {
                    maxDist_nm = Airport::symbologyGndRadius_nm;
                } else if (c->isDel()) {
                    maxDist_nm = Airport::symbologyDelRadius_nm;
                }
                foreach (const auto _a, c->airports()) {
                    if (NavData::distance(_a->lat, _a->lon, lat, lon) < maxDist_nm) {
                        _newHoveredControllers.insert(c);
                    }
                }
            }
        }
        if (_newHoveredControllers != m_hoveredControllers) {
            m_hoveredControllers = _newHoveredControllers;
            update();
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
        update();
    }
    if (!_mapRectSelecting) {
        _lastPos = _mouseDownPos = currentPos;
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent* event) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    QToolTip::hideText();
    if (_mapMoving) {
        _mapMoving = false;
    } else if (_mapZooming) {
        _mapZooming = false;
    } else if (_mapRectSelecting) {
        _mapRectSelecting = false;
        if (currentPos != _mouseDownPos) {
            // moved more than 40px?
            if (
                ((currentPos.x() - _mouseDownPos.x()) * (currentPos.x() - _mouseDownPos.x()))
                + ((currentPos.y() - _mouseDownPos.y()) * (currentPos.y() - _mouseDownPos.y())) > 40 * 40
            ) {
                double downLat, downLon;
                if (local2latLon(_mouseDownPos.x(), _mouseDownPos.y(), downLat, downLon)) {
                    double currLat, currLon;
                    if (local2latLon(currentPos.x(), currentPos.y(), currLat, currLon)) {
                        DoublePair mid = NavData::greatCircleFraction(downLat, downLon, currLat, currLon, .5);
                        setMapPosition(
                            mid.first, mid.second,
                            qMax(
                                NavData::distance(
                                    downLat, downLon,
                                    downLat, currLon
                                ),
                                NavData::distance(
                                    downLat, downLon,
                                    currLat, downLon
                                )
                            ) / 4000.
                        );
                    }
                }
            }
        } else {
            update();
        }
    } else if (_mouseDownPos == currentPos && event->button() == Qt::LeftButton) {
        QList<MapObject*> objects;
        foreach (MapObject* m, objectsAt(currentPos.x(), currentPos.y())) {
            if (!m->hasPrimaryAction()) {
                continue;
            }

            objects.append(m);
        }
        if (objects.isEmpty()) {
            clientSelection->clearObjects();
            clientSelection->close();
        } else if (objects.size() == 1) {
            if (objects[0]->hasPrimaryAction()) {
                objects[0]->primaryAction();
            }
        } else {
            clientSelection->move(QCursor::pos());
            clientSelection->setObjects(objects);
        }
    } else if (_mouseDownPos == currentPos && event->button() == Qt::RightButton) {
        rightClick(currentPos);
    }
    update();
}

void GLWidget::rightClick(const QPoint& pos) {
    auto objects = objectsAt(pos.x(), pos.y());
    int countRelevant = 0;
    Pilot* pilot = 0;
    Airport* airport = 0;
    foreach (MapObject* m, objects) {
        if (dynamic_cast<Pilot*>(m) != 0) {
            pilot = dynamic_cast<Pilot*>(m);
            countRelevant++;
        }
        if (dynamic_cast<Airport*>(m) != 0) {
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
        GuiMessages::message(
            QString("toggled routes for %1 [%2]").arg(airport->id, airport->showRoutes? "off": "on"),
            "routeToggleAirport"
        );
        airport->showRoutes = !airport->showRoutes;
        if (AirportDetails::instance(false) != 0) {
            AirportDetails::instance()->refresh();
        }
        if (PilotDetails::instance(false) != 0) { // can have an effect on the state of
            PilotDetails::instance()->refresh(); // ...PilotDetails::cbPlotRoutes
        }
        invalidatePilots();
    } else if (pilot != 0) {
        // display flight path for pilot
        GuiMessages::message(
            QString("toggled route for %1 [%2]").arg(pilot->callsign, pilot->showDepDestLine? "off": "on"),
            "routeTogglePilot"
        );
        pilot->showDepDestLine = !pilot->showDepDestLine;
        if (PilotDetails::instance(false) != 0) {
            PilotDetails::instance()->refresh();
        }
        invalidatePilots();
    }
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    // Nvidia mouse coordinates workaround (https://github.com/qutescoop/qutescoop/issues/46)
    QPoint currentPos = mapFromGlobal(QCursor::pos());

    QToolTip::hideText();
    if (event->buttons().testFlag(Qt::LeftButton)) {
        double lat, lon;
        if (local2latLon(currentPos.x(), currentPos.y(), lat, lon)) {
            setMapPosition(lat, lon, _zoom);
        }
        zoomIn(.6);
    } else if (event->button() == Qt::RightButton) {
        double lat, lon;
        if (local2latLon(currentPos.x(), currentPos.y(), lat, lon)) {
            setMapPosition(lat, lon, _zoom);
        }
        zoomIn(-.6);
    } else if (event->button() == Qt::MiddleButton) {
        zoomTo(2.);
    }
}

void GLWidget::wheelEvent(QWheelEvent* event) {
    QToolTip::hideText();
    //if(event->orientation() == Qt::Vertical) {
    if (qAbs(event->angleDelta().y()) > Settings::wheelMax()) { // always recalibrate if bigger values are found
        Settings::setWheelMax(qAbs(event->angleDelta().y()));
    }
    zoomIn((double) event->angleDelta().y() / Settings::wheelMax());
}

bool GLWidget::event(QEvent* event) {
    // we are experimenting to not use tooltips by default currently
    if (Settings::showToolTips() && event->type() == QEvent::ToolTip) {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        if (m_hoveredObjects.isEmpty()) {
            QToolTip::hideText();
        } else {
            QStringList toolTip;
            foreach (const auto o, m_hoveredObjects) {
                toolTip << o->toolTip();
            }
            QToolTip::showText(helpEvent->globalPos(), toolTip.join("\n"));
        }
    }
    return QGLWidget::event(event);
}

void GLWidget::zoomIn(double factor) {
    _zoom -= _zoom * qMax(-.6, qMin(.6, .2 * factor * Settings::zoomFactor()));
    resetZoom();
    update();
}

void GLWidget::zoomTo(double zoom) {
    this->_zoom = zoom;
    resetZoom();
    update();
}



/////////////////////////////
// rendering text
/////////////////////////////

void GLWidget::renderLabels() {
    /**
     * Gather MapObjects
     */

    // sector controller labels
    if (m_isControllerMapObjectsDirty) {
        qDebug() << "building controllerMapObjects";
        m_controllerMapObjects.clear();

        foreach (Controller* c, Whazzup::instance()->whazzupData().controllers) {
            if (c->sector != 0) {
                m_controllerMapObjects.append(c);
            }
        }
        m_isControllerMapObjectsDirty = false;
    }

    // planned route waypoint labels from Flightplan Dialog
    QList<MapObject*> planFlightWaypointMapObjects;
    if (PlanFlightDialog::instance(false) != 0) {
        if (
            PlanFlightDialog::instance()->cbPlot->isChecked()
            && PlanFlightDialog::instance()->selectedRoute != 0
        ) {
            for (int i = 1; i < PlanFlightDialog::instance()->selectedRoute->waypoints.size() - 1; i++) {
                auto _wp = PlanFlightDialog::instance()->selectedRoute->waypoints[i];
                if (!planFlightWaypointMapObjects.contains(_wp)) {
                    planFlightWaypointMapObjects.append(_wp);
                }
            }
        }
    }

    // airport labels
    if (m_isAirportsMapObjectsDirty) {
        qDebug() << "building airportMapObjects";

        m_activeAirportMapObjects.clear();
        const QList<Airport*> activeAirportsSorted = NavData::instance()->activeAirports.values();
        for (int i = activeAirportsSorted.size() - 1; i > -1; i--) { // by traffic
            if (activeAirportsSorted[i]->active) {
                m_activeAirportMapObjects.append(activeAirportsSorted[i]);
            }
        }

        m_inactiveAirportMapObjects.clear();
        if (Settings::showInactiveAirports()) {
            foreach (const auto a, NavData::instance()->airports) {
                if (!a->active) {
                    m_inactiveAirportMapObjects.append(a);
                }
            }
        }

        m_isAirportsMapObjectsDirty = false;
    }

    // pilot labels
    if (m_isPilotMapObjectsDirty) {
        qDebug() << "building pilotMapObjects";
        m_pilotMapObjects.clear();

        const QList<Pilot*> pilots = Whazzup::instance()->whazzupData().allPilots();
        if (Settings::showPilotsLabels()) {
            foreach (Pilot* p, pilots) {
                if (qFuzzyIsNull(p->lat) && qFuzzyIsNull(p->lon)) {
                    continue;
                }
                if (
                    p->flightStatus() == Pilot::DEPARTING
                    || p->flightStatus() == Pilot::EN_ROUTE
                    || p->flightStatus() == Pilot::ARRIVING
                ) {
                    if (p->isFriend()) {
                        m_pilotMapObjects.prepend(p);
                    } else {
                        m_pilotMapObjects.append(p);
                    }
                }
            }
        }
        m_isPilotMapObjectsDirty = false;
    }

    // labels of waypoints used in shown routes
    // - we build that list in createPilotsList() now

    /**
     * remove vanished MapObjects from m_fontRectangles to keep positions as sticky as possible
     */
    const QList<MapObject*> allMapObjects =
        m_controllerMapObjects // big hover area makes it difficult to hover other labels
        + planFlightWaypointMapObjects
        + m_activeAirportMapObjects
        + m_pilotMapObjects
        + m_usedWaypointMapObjects
        + m_inactiveAirportMapObjects // that might hit performance
    ;

    // update our list of stable positions
    foreach (const auto &fr, m_fontRectangles) {
        if (!allMapObjects.contains(fr.object)) {
            m_fontRectangles.remove(fr);
        }
    }

    /**
     * Render labels
     */

    // sector controller labels
    renderLabels(
        m_controllerMapObjects,
        _controllerLabelZoomTreshold,
        Settings::firFont(),
        Settings::firFontColor(),
        Settings::firFontSecondary(),
        Settings::firFontSecondaryColor(),
        false,
        99
    );

    // static sectors - render directly, no questions asked
    qglColor(Settings::friendsHighlightColor()); // just something different from normal sectors
    foreach (const Sector* sector, m_staticSectors) {
        QPair<double, double> center = sector->getCenter();
        if (center.first > -180.) {
            double lat = center.first;
            double lon = center.second;
            renderText(
                SX(lat, lon),
                SY(lat, lon),
                SZ(lat, lon),
                sector->icao,
                Settings::firFont()
            );
        }
    }

    // planned route waypoint labels from Flightplan Dialog
    renderLabels(
        planFlightWaypointMapObjects,
        _usedWaypointsLabelZoomThreshold,
        Settings::waypointsFont(),
        Settings::waypointsFontColor(),
        Settings::waypointsFont(),
        Settings::waypointsFontColor()
    );

    // airport labels
    renderLabels(
        m_activeAirportMapObjects,
        _activeAirportLabelZoomTreshold,
        Settings::airportFont(),
        Settings::airportFontColor(),
        Settings::airportFontSecondary(),
        Settings::airportFontSecondaryColor()
    );

    // pilot labels
    renderLabels(
        m_pilotMapObjects,
        _pilotLabelZoomTreshold,
        Settings::pilotFont(),
        Settings::pilotFontColor(),
        Settings::pilotFontSecondary(),
        Settings::pilotFontSecondaryColor()
    );

    // inactive airports
    if (Settings::showInactiveAirports()) {
        renderLabels(
            m_inactiveAirportMapObjects,
            _inactiveAirportLabelZoomTreshold,
            Settings::inactiveAirportFont(),
            Settings::inactiveAirportFontColor(),
            Settings::inactiveAirportFont(),
            Settings::inactiveAirportFontColor()
        );
    }

    // waypoints used in shown routes
    if (Settings::showUsedWaypoints()) {
        renderLabels(
            m_usedWaypointMapObjects,
            _usedWaypointsLabelZoomThreshold,
            Settings::waypointsFont(),
            Settings::waypointsFontColor(),
            Settings::waypointsFont(),
            Settings::waypointsFontColor(),
            false,
            0 // don't try secondary positions
        );
    }

    // last: draw hovered objects
    foreach (const auto renderLabelCommand, m_prioritizedLabels) {
        renderLabels(
            renderLabelCommand.objects,
            renderLabelCommand.zoomTreshold,
            renderLabelCommand.font,
            renderLabelCommand.color,
            renderLabelCommand.secondaryFont,
            renderLabelCommand.secondaryColor,
            renderLabelCommand.isFastBail,
            renderLabelCommand.tryNOtherPositions,
            true
        );
    }
    m_prioritizedLabels.clear();
}

void GLWidget::renderLabels(
    const QList<MapObject*>& objects,
    const double zoomTreshold,
    const QFont& font,
    const QColor _color,
    const QFont& secondaryFont,
    const QColor _secondaryColor,
    const bool isFastBail, // performance optimization: bail on first MapObject that's not drawn - this will not work if
    // stable positions are desired
    const unsigned short tryNOtherPositions,
    const bool isHoverRenderPass
) {
    if (_zoom > zoomTreshold || _color.alpha() == 0) {
        return;
    }

    QFontMetricsF fontMetrics(font, this);
    QFontMetricsF fontMetricsSecondary(secondaryFont, this);

    foreach (MapObject* o, objects) {
        const bool isHovered = m_hoveredObjects.contains(o);

        // fade out
        auto color(_color);
        auto secondaryColor(_secondaryColor);
        if (!isHovered) {
            color.setAlphaF(qMax(0., qMin(color.alphaF(), (zoomTreshold - _zoom) / zoomTreshold * 1.5)));
            secondaryColor.setAlphaF(qMax(0., qMin(secondaryColor.alphaF(), (zoomTreshold - _zoom) / zoomTreshold * 1.5)));
        }

        // if it is hovered, save for the hoverRenderPass later
        if (isHovered && !isHoverRenderPass) {
            m_prioritizedLabels.prepend(
                {
                    { o },
                    zoomTreshold,
                    font,
                    color,
                    secondaryFont,
                    secondaryColor,
                    isFastBail,
                    tryNOtherPositions
                }
            );
            continue;
        }

        FontRectangle useRect;
        // look if we have previously drawn that label
        foreach (const auto &fr, m_fontRectangles) {
            if (fr.object == o) {
                useRect = fr;
                m_fontRectangles.remove(fr);
                break;
            }
        }

        if (
            !isHovered
            && useRect.object == 0
            && m_fontRectangles.size() + m_prioritizedLabels.size() >= Settings::maxLabels()
        ) {
            if (isFastBail) {
                return;
            }
            continue;
        }

        if (!o->drawLabel) {
            continue;
        }

        int x, y;
        if (!latLon2local(o->lat, o->lon, &x, &y)) {
            continue;
        }

        const uint lineMargin = 3;

        const QString &firstLine = isHovered? o->mapLabelHovered(): o->mapLabel();

        // tightBoundingRect() might be slow on Windows according to docs
        QRectF firstLineRect = fontMetrics.tightBoundingRect(firstLine);
        float firstLineOffset = -firstLineRect.top();
        firstLineRect.moveTop(0);
        QRectF rect(firstLineRect); // complete boundingRect
        rect.setHeight(rect.height() + lineMargin);

        const auto &secondaryLines = isHovered? o->mapLabelSecondaryLinesHovered(): o->mapLabelSecondaryLines();

        auto thisColor = isHovered? Helpers::highLightColor(color): color;
        auto thisSecondaryColor = isHovered? Helpers::highLightColor(secondaryColor): secondaryColor;

        QList<QRectF> secondaryRects;
        float secondaryLinesOffset = 0.;
        for (int iLine = 0; iLine < secondaryLines.size(); iLine++) {
            auto _rect = fontMetricsSecondary.tightBoundingRect(secondaryLines[iLine]);
            secondaryLinesOffset = -_rect.top();
            _rect.moveTop(rect.bottom());
            secondaryRects.insert(iLine, _rect);
            // update complete boundingRect
            rect.setHeight(rect.height() + _rect.height() + lineMargin);
            rect.setWidth(qMax(rect.width(), _rect.width()));
        }

        // remove last lineMargin
        rect.setHeight(rect.height() - lineMargin);

        const uint distanceFromPos = 8;
        int drawY = y - rect.height() - distanceFromPos;
        int drawX = x - rect.width() / 2; // center horizontally
        rect.moveTo(drawX, drawY);

        if (useRect.object == 0) {
            const QVector<QRectF> rects { // possible positions, with preferred ones first
                // above
                rect,
                // right, below, left
                rect.translated(rect.width() / 2 + distanceFromPos, rect.height() / 2 + distanceFromPos),
                rect.translated(0, rect.height() + 2 * distanceFromPos),
                rect.translated(-rect.width() / 2 - distanceFromPos, rect.height() / 2 + distanceFromPos),
                // diagonal (for sectors)
                rect.translated(rect.width() / 2 + distanceFromPos, 0),
                rect.translated(rect.width() / 2 + distanceFromPos, rect.height() + 2 * distanceFromPos),
                rect.translated(-rect.width() / 2 - distanceFromPos, rect.height() + 2 * distanceFromPos),
                rect.translated(-rect.width() / 2 - distanceFromPos, 0),
                // right, below, left (far, for sectors)
                rect.translated(rect.width() + distanceFromPos, rect.height() / 2 + distanceFromPos),
                rect.translated(0, rect.height() * 2 + 2 * distanceFromPos),
                rect.translated(-rect.width() - distanceFromPos, rect.height() / 2 + distanceFromPos),
            };

            for (unsigned short i = 0; i <= tryNOtherPositions && i < rects.count(); i++) {
                if (shouldDrawLabel(rects[i])) {
                    useRect.rect = rects[i];
                    useRect.object = o;
                    // we have not drawn that object before
                    break;
                }
            }
        } else {
            useRect.rect.setHeight(rect.height());
            useRect.rect.setLeft(useRect.rect.left() + (useRect.rect.width() - rect.width()) / 2);
            useRect.rect.setWidth(rect.width());
        }

        if (useRect.object == 0) {
            continue;
        }

        bool isFriend = false;
        // pilots, controllers
        Client* cl = dynamic_cast <Client*> (o);
        if (cl != 0) {
            // Pilots and Controllers
            isFriend = cl->isFriend();
        } else {
            // airports (having a controller that is in the friends list)
            Airport* a = dynamic_cast <Airport*> (o);
            if (a != 0) {
                foreach (const auto c, a->allControllers()) {
                    if (c->isFriend()) {
                        // only if that is the primary airport
                        if (c->airports(false).contains(a)) {
                            isFriend = true;
                            break;
                        }
                    }
                }
            }
        }

        if (Settings::labelAlwaysBackdropped() || isHovered || isFriend) {
            // draw backdrop
            QList<QPair<double, double> > rectPointsLatLon{ { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };
            const auto xMargin = 4;
            const auto rightMargin = 1;
            const auto yMargin = 5;

            if (
                local2latLon(
                    useRect.rect.left() - xMargin, useRect.rect.top() - yMargin,
                    rectPointsLatLon[0].first, rectPointsLatLon[0].second
                )
                && local2latLon(
                    useRect.rect.right() + xMargin + rightMargin, useRect.rect.top() - yMargin,
                    rectPointsLatLon[1].first, rectPointsLatLon[1].second
                )
                && local2latLon(
                    useRect.rect.right() + xMargin + rightMargin, useRect.rect.bottom() + yMargin,
                    rectPointsLatLon[2].first, rectPointsLatLon[2].second
                )
                && local2latLon(
                    useRect.rect.left() - xMargin, useRect.rect.bottom() + yMargin,
                    rectPointsLatLon[3].first, rectPointsLatLon[3].second
                )
            ) {
                if (Settings::labelAlwaysBackdropped() || isHovered) {
                    auto bgColor = thisColor.lightnessF() < .5? Settings::labelHoveredBgColor(): Settings::labelHoveredBgDarkColor();
                    if (!isHovered) {
                        bgColor.setAlphaF(qMax(0., qMin(bgColor.alphaF(), (zoomTreshold - _zoom) / zoomTreshold * 1.5)));
                    }

                    qglColor(bgColor);
                    glBegin(GL_POLYGON);
                    foreach (const auto p, rectPointsLatLon) {
                        VERTEX(p.first, p.second);
                    }
                    glEnd();

                    // draw text shadow
                    if (isHovered) {
                        const auto shadowColor = Helpers::shadowColorForBg(bgColor);
                        qglColor(shadowColor);
                        renderText(
                            useRect.rect.left() + (useRect.rect.width() - firstLineRect.width()) / 2 + 1,
                            useRect.rect.top() + firstLineRect.top() + firstLineOffset + 1,
                            firstLine,
                            font
                        );

                        const auto shadowSecondaryColor = Helpers::shadowColorForBg(bgColor);
                        qglColor(shadowSecondaryColor);
                        for (int iLine = 0; iLine < secondaryLines.size(); iLine++) {
                            renderText(
                                useRect.rect.left() + (useRect.rect.width() - secondaryRects[iLine].width()) / 2 + 1,
                                useRect.rect.top() + secondaryRects[iLine].top() + secondaryLinesOffset + 1,
                                secondaryLines[iLine],
                                secondaryFont
                            );
                        }
                    }
                }

                if (isFriend) {
                    QColor friendsRectColor;
                    Airport* a = dynamic_cast <Airport*> (o);
                    if (a != 0) {
                        friendsRectColor = Settings::friendsAirportLabelRectColor();
                    } else {
                        Controller* c = dynamic_cast <Controller*> (o);
                        if (c != 0) {
                            friendsRectColor = Settings::friendsSectorLabelRectColor();
                        } else {
                            Pilot* c = dynamic_cast <Pilot*> (o);
                            if (c != 0) {
                                friendsRectColor = Settings::friendsPilotLabelRectColor();
                            } else {
                                Q_ASSERT(true);
                            }
                        }
                    }

                    if (!isHovered) {
                        friendsRectColor.setAlphaF(qMax(0., qMin(friendsRectColor.alphaF(), (zoomTreshold - _zoom) / zoomTreshold * 1.5)));
                    }

                    qglColor(friendsRectColor);
                    glLineWidth(.1);
                    glBegin(GL_LINE_LOOP);
                    foreach (const auto p, rectPointsLatLon) {
                        VERTEX(p.first, p.second);
                    }
                    glEnd();
                }
            }
        }

        qglColor(thisColor);
        renderText(
            useRect.rect.left() + (useRect.rect.width() - firstLineRect.width()) / 2,
            useRect.rect.top() + firstLineRect.top() + firstLineOffset,
            firstLine,
            font
        );
        qglColor(thisSecondaryColor);
        for (int iLine = 0; iLine < secondaryLines.size(); iLine++) {
            renderText(
                useRect.rect.left() + (useRect.rect.width() - secondaryRects[iLine].width()) / 2,
                useRect.rect.top() + secondaryRects[iLine].top() + secondaryLinesOffset,
                secondaryLines[iLine],
                secondaryFont
            );
        }

        m_fontRectangles.insert(useRect);
    }
}

bool GLWidget::shouldDrawLabel(const QRectF &rect) {
    foreach (const FontRectangle &fr, m_fontRectangles) {
        if (rect.intersects(fr.rect)) {
            return false;
        }
    }
    return true;
}

QList<GLWidget::FontRectangle> GLWidget::fontRectanglesPrioritized() const {
    auto priorityFor = [](const FontRectangle& fr) ->char {
            if (qobject_cast<Airport*>(fr.object) != nullptr) {
                return 30;
            }
            if (qobject_cast<Controller*>(fr.object) != nullptr) {
                return 20;
            }
            if (qobject_cast<Pilot*>(fr.object) != nullptr) {
                return 10;
            }
            return 0;
        }
    ;

    QMultiMap<char, GLWidget::FontRectangle> sorted;
    foreach (const auto &fr, m_fontRectangles) {
        sorted.insert(priorityFor(fr), fr);
    }

    return sorted.values();
}

void GLWidget::drawTestTextures() {
    static QTimer* testTimer;
    static float i = 0.;
    if (testTimer == 0) {
        testTimer = new QTimer();
        connect(
            testTimer, &QTimer::timeout, this, [&] {
                i = fmod(i + .1, 30.); update();
            }
        );
        testTimer->setInterval(30); testTimer->start();
    }

    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, _earthTex); // with GL texture
    for (float lat = 90. - i; lat >= -90.; lat -= 30.) {
        for (float lon = -165.; lon < 180.; lon += 30.) {
            drawBillboardWorldSize(lat, lon, QSizeF(.4, .4) * qCos(lat * Pi180));
        }
    }

    const static QPixmap planePm(":/startup/logo"); // with QImage
    bindTexture(planePm, GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption); // that's cached by Qt
    for (float lat = -90. + i; lat <= 90.; lat += 30.) {
        for (float lon = -180.; lon < 180.; lon += 30.) {
            drawBillboardScreenSize(lat, lon, planePm.size().scaled(64, 64, Qt::KeepAspectRatio));
        }
    }
    glPopAttrib();
}

void GLWidget::drawBillboardScreenSize(GLfloat lat, GLfloat lon, const QSize &size) {
    const GLfloat sizeFactor = .5 * _zoom / height();
    drawBillboard(lat, lon, sizeFactor * size.width(), sizeFactor * size.height());
}

// size in world coordinates
void GLWidget::drawBillboardWorldSize(GLfloat lat, GLfloat lon, const QSizeF &size) {
    drawBillboard(lat, lon, size.width() / 2., size.height() / 2.);
}

void GLWidget::drawBillboard(GLfloat lat, GLfloat lon, GLfloat halfWidth, GLfloat halfHeight, GLfloat alpha) {
    glPushMatrix();
    orthoMatrix(lat, lon);

    glColor4f(1., 1., 1., alpha);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 1);
    glVertex2f(-halfWidth, halfHeight);
    glTexCoord2i(1, 1);
    glVertex2f(halfWidth, halfHeight);
    glTexCoord2i(1, 0);
    glVertex2f(halfWidth, -halfHeight);
    glTexCoord2f(0, 0);
    glVertex2f(-halfWidth, -halfHeight);
    glEnd();

    if (false) { // debug
        drawCoordinateAxiiCurrentMatrix();

        glColor3f(1., 1., 1.);
        renderText(0, 0, 0, QString("%1/%2").arg(round(lat)).arg(round(lon)), QFont());
    }

    glPopMatrix();
}

void GLWidget::orthoMatrix(GLfloat lat, GLfloat lon) {
    GLfloat theta = std::atan2(SY(lat, lon), SX(lat, lon));
    GLfloat phi = std::asin(SZ(lat, lon));

    glRotatef(90, 1, 0, 0);
    glRotatef(theta / Pi180 + 90, 0, 1, 0);
    glRotatef(-phi / Pi180, 1, 0, 0);

    glTranslatef(0, 0, 1);
}

QList<MapObject*> GLWidget::objectsAt(int x, int y, double radiusSimple) const {
    QList<MapObject*> result;
    foreach (const auto &fr, fontRectanglesPrioritized()) { // scan text labels
        if (fr.rect.adjusted(-10, -10, 10, 10).contains(x, y)) {
            result.append(fr.object);
        }
    }

    double lat, lon;
    if (!local2latLon(x, y, lat, lon)) { // returns false if not on globe
        return result;
    }

    double radiusDegQuad = Nm2Deg((qFuzzyIsNull(radiusSimple)? 30. * _zoom: radiusSimple));
    radiusDegQuad *= radiusDegQuad;

    foreach (Airport* a, NavData::instance()->airports.values()) {
        if (a->active) {
            double x = a->lat - lat;
            double y = a->lon - lon;
            if (x * x + y * y < radiusDegQuad) {
                if (!result.contains(a)) {
                    result.append(a);
                }
            }
        }
    }

    // this adds sectors and airports when hovered over/near them (disabled due to clutter)
//    foreach(Controller* c, Whazzup::instance()->whazzupData().controllers.values()) {
//        if(c->sector != 0 && c->sector->containsPoint(QPointF(lat, lon))) { // controllers with sectors
//            result.insert(c);
//        } else { // APP, TWR, GND, DEL
//            int maxDist_nm = -1;
//            if(c->isAppDep()) {
//                maxDist_nm = Airport::symbologyAppRadius_nm;
//            } else if(c->isTwr()) {
//                maxDist_nm = Airport::symbologyTwrRadius_nm;
//            } else if(c->isGnd()) {
//                maxDist_nm = Airport::symbologyGndRadius_nm;
//            } else if(c->isDel() || c->isAtis()) { // add ATIS to clientSelection
//                maxDist_nm = Airport::symbologyDelRadius_nm;
//            }
//            foreach(auto* _a, c->airports()) {
//                if(NavData::distance(_a->lat, _a->lon, lat, lon) < maxDist_nm) {
//                    result.insert(c);
//                    result.insert(_a);
//                }
//            }
//        }
//    }

    foreach (Pilot* p, Whazzup::instance()->whazzupData().pilots.values()) {
        double x = p->lat - lat;
        double y = p->lon - lon;
        if (x * x + y * y < radiusDegQuad) {
            if (!result.contains(p)) {
                result.append(p);
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
    if (local2latLon(_mouseDownPos.x(), _mouseDownPos.y(), downLat, downLon)) {
        double currLat, currLon;
        if (local2latLon(current.x(), current.y(), currLat, currLon)) {
            // calculate a rectangle: approximating what the viewport will look after zoom
            // (far from perfect but an okayish approximation...)
            // down...: where the mouse was pressed down (1 edge of the rectangle)
            // curr...: where it is now (the opposite edge)
            double currLonDist = NavData::distance(currLat, downLon, currLat, currLon);
            double downLonDist = NavData::distance(downLat, downLon, downLat, currLon);
            double avgLonDist = (downLonDist + currLonDist) / 2.; // this needs to be the side length
            DoublePair downLatCurrLon = NavData::greatCircleFraction(
                downLat, downLon,
                downLat, currLon,
                avgLonDist / downLonDist
            );
            DoublePair currLatDownLon = NavData::greatCircleFraction(
                currLat, currLon,
                currLat, downLon,
                avgLonDist / currLonDist
            );
            QList<QPair<double, double> > points;
            points.append(DoublePair(downLat, downLon));
            points.append(DoublePair(downLatCurrLon.first, downLatCurrLon.second));
            points.append(DoublePair(currLat, currLon));
            points.append(DoublePair(currLatDownLon.first, currLatDownLon.second));
            points.append(DoublePair(downLat, downLon));

            // draw background
            glColor4f((GLfloat) 0., (GLfloat) 1., (GLfloat) 1., (GLfloat) .2);
            glBegin(GL_POLYGON);
            NavData::plotGreatCirclePoints(points);
            glEnd();
            // draw rectangle
            glLineWidth(2.);
            glColor4f((GLfloat) 0., (GLfloat) 1., (GLfloat) 1., (GLfloat) .5);
            glBegin(GL_LINE_LOOP);
            NavData::plotGreatCirclePoints(points);
            glEnd();
            // draw great circle course line
            glLineWidth(2.);
            glColor4f((GLfloat) 0., (GLfloat) 1., (GLfloat) 1., (GLfloat) .2);
            glBegin(GL_LINE_STRIP);
            NavData::plotGreatCirclePoints(QList<QPair<double, double> >() << points[0] << points[2]);
            glEnd();

            // information labels
            const QFont font = Settings::firFont();
            const QFontMetricsF fontMetrics(font, this);

            // show position label
            const QString currText = NavData::toEurocontrol(currLat, currLon);
            int x, y;
            if (latLon2local(currLat, currLon, &x, &y)) {
                glColor4f((GLfloat) 0., (GLfloat) 0., (GLfloat) 0., (GLfloat) .7);
                renderText(
                    x + 21,
                    y + 21,
                    currText, font
                );
                glColor4f((GLfloat) 0., (GLfloat) 1., (GLfloat) 1., (GLfloat) 1.);
                renderText(
                    x + 20,
                    y + 20,
                    currText, font
                );
            }

            // show distance label
            const DoublePair middle = NavData::greatCircleFraction(downLat, downLon, currLat, currLon, .5);
            const QString middleText = QString("%1 NM / TC %2 deg").arg(
                NavData::distance(downLat, downLon, currLat, currLon),
                0, 'f', 1
            ).arg(
                NavData::courseTo(downLat, downLon, currLat, currLon),
                3, 'f', 0, '0'
            );
            QRectF rect = fontMetrics.boundingRect(middleText);
            if (latLon2local(middle.first, middle.second, &x, &y)) {
                glColor4f((GLfloat) 0., (GLfloat) 0., (GLfloat) 0., (GLfloat) .7);
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
 * for debugging. Shows axii at the Earth center. x = red, y = green, z = blue
 */
void GLWidget::drawCoordinateAxii() const {
    glPushMatrix();

    glLoadIdentity();
    glTranslatef(0, 0, -9);
    glRotated(_xRot, 1, 0, 0);
    glRotated(_yRot, 0, 1, 0);
    glRotated(_zRot, 0, 0, 1);

    drawCoordinateAxiiCurrentMatrix();

    glPopMatrix();
}

/**
 * for debugging. Draws the axii. x = red, y = green, z = blue
 */
void GLWidget::drawCoordinateAxiiCurrentMatrix() const {
    GLUquadricObj* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);

    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);

    glDisable(GL_TEXTURE_2D);
    glColor3f(0, 0, 0); gluSphere(q, 0.02, 64, 32); // center
    glRotatef(90, 0, 1, 0);

    glColor3f(1, 0, 0); gluCylinder(q, 0.02, 0.0, 0.3, 64, 1); // x-axis
    glRotatef(90, 0, -1, 0);
    glRotatef(90, -1, 0, 0);
    glColor3f(0, 1, 0); gluCylinder(q, 0.02, 0.0, 0.3, 64, 1); // y-axis
    glRotatef(90, 1, 0, 0);
    glColor3f(0, 0, 1); gluCylinder(q, 0.02, 0.0, 0.3, 64, 1); // z-axis

    glPopAttrib();
    glPopMatrix();
}


/////////////////////////
// uncategorized
/////////////////////////

void GLWidget::newWhazzupData(bool isNew) {
    qDebug() << "isNew =" << isNew;
    if (isNew) {
        // update airports
        NavData::instance()->updateData(Whazzup::instance()->whazzupData());

        m_hoveredObjects.clear();
        m_fontRectangles.clear();
        invalidatePilots();
        invalidateControllers();
        invalidateAirports();
        m_friendPositions = Whazzup::instance()->whazzupData().friendsLatLon();
    }
    qDebug() << "-- finished";
}

void GLWidget::createFriendHighlighter() {
    _highlighter = new QTimer(this);
    _highlighter->setInterval(100);
    connect(_highlighter, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    _highlighter->start();
}

void GLWidget::destroyFriendHighlighter() {
    if (_highlighter == 0) {
        return;
    }
    if (_highlighter->isActive()) {
        _highlighter->stop();
    }
    disconnect(_highlighter, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    delete _highlighter;
    _highlighter = 0;
}


//////////////////////////////////
// Clouds, Lightning and Earth Textures
//////////////////////////////////

void GLWidget::parseTexture() {
    qDebug();
    GuiMessages::progress("textures", "Preparing textures...");

    QImage earthTexIm;

    if (Settings::glTextures()) {
        QString earthTexFile = Settings::dataDirectory(QString("textures/%1").arg(Settings::glTextureEarth()));
        qDebug() << "loading earth texture";
        GuiMessages::progress("textures", "Preparing textures: loading earth...");
        earthTexIm.load(earthTexFile);
    }

    if (earthTexIm.isNull()) {
        qWarning() << "Unable to load texture file: "
                   << Settings::dataDirectory(QString("textures/%1").arg(Settings::glTextureEarth()));
    } else {
        GLint max_texture_size;  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
        qDebug() << "OpenGL reported MAX_TEXTURE_SIZE as" << max_texture_size;

        // multitexturing units, if we need it once (headers in GL/glext.h, on Windows not available ?!)
        //GLint max_texture_units; glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);
        //qDebug() << "OpenGL reported MAX_TEXTURE_UNITS as" << max_texture_units;
        qDebug() << "Binding parsed texture as" << earthTexIm.width()
                 << "x" << earthTexIm.height() << "px texture";
        qDebug() << "Generating texture coordinates";
        GuiMessages::progress("textures", "Preparing textures: preparing texture coordinates...");
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glGenTextures(1, &earthTex); // bindTexture does this the Qt'ish way already
        qDebug() << "Emptying error buffer";
        glGetError(); // empty the error buffer
        qDebug() << "Binding texture";
        _earthTex = bindTexture(
            earthTexIm,
            GL_TEXTURE_2D,
            GL_RGBA,
            QGLContext::LinearFilteringBindOption
        ); // QGLContext::MipmapBindOption
        if (GLenum glError = glGetError()) {
            qCritical() << QString("OpenGL returned an error (0x%1)")
                .arg((int) glError, 4, 16, QChar('0'));
        }
    }

    qDebug() << "-- finished";
    update();
    GuiMessages::remove("textures");
}

void GLWidget::createLights() {
    //const GLfloat earthAmbient[]  = {0, 0, 0, 1};
    const GLfloat earthDiffuse[] = { 1, 1, 1, 1 };
    const GLfloat earthSpecular[] = {
        (GLfloat) Settings::specularColor().redF(),
        (GLfloat) Settings::specularColor().greenF(),
        (GLfloat) Settings::specularColor().blueF(),
        (GLfloat) Settings::specularColor().alphaF()
    };
    const GLfloat earthEmission[] = { 0, 0, 0, 1 };
    const GLfloat earthShininess[] = { (GLfloat) Settings::earthShininess() };
    //glMaterialfv(GL_FRONT, GL_AMBIENT, earthAmbient); // GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
    glMaterialfv(GL_FRONT, GL_DIFFUSE, earthDiffuse); // ...GL_EMISSION, GL_SHININESS, GL_AMBIENT_AND_DIFFUSE,
    glMaterialfv(GL_FRONT, GL_SPECULAR, earthSpecular); // ...GL_COLOR_INDEXES
    glMaterialfv(GL_FRONT, GL_EMISSION, earthEmission); //
    glMaterialfv(GL_FRONT, GL_SHININESS, earthShininess); //... only DIFFUSE has an own alpha channel!
    glColorMaterial(GL_FRONT, GL_AMBIENT); // GL_EMISSION, GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
                                           // GL_AMBIENT_AND_DIFFUSE
    glEnable(GL_COLOR_MATERIAL); // controls if glColor will drive the given values in glColorMaterial

    const GLfloat sunAmbient[] = { 0., 0., 0., 1. };
    QColor adjustSunDiffuse = Settings::sunLightColor();
    if (Settings::glLights() > 1) {
        adjustSunDiffuse = adjustSunDiffuse.darker(
            100. * (Settings::glLights() - // reduce light intensity by number of
                                           // lights...
            Settings::glLightsSpread() / 180. * (Settings::glLights() - 1))
        ); // ...and
           // increase
           // again
           // by
           // their
           // distribution
    }
    const GLfloat sunDiffuse[] = {
        (GLfloat) adjustSunDiffuse.redF(),
        (GLfloat) adjustSunDiffuse.greenF(),
        (GLfloat) adjustSunDiffuse.blueF(),
        (GLfloat) adjustSunDiffuse.alphaF()
    };
    //const GLfloat sunSpecular[] = {1, 1, 1, 1}; // we drive this via material values
    for (int light = 0; light < 8; light++) {
        if (light < Settings::glLights()) {
            glLightfv(GL_LIGHT0 + light, GL_AMBIENT, sunAmbient); // GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,
                                                                  // GL_POSITION,
                                                                  // GL_SPOT_CUTOFF,
            glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, sunDiffuse); // ...GL_SPOT_DIRECTION, GL_SPOT_EXPONENT,
                                                                  // GL_CONSTANT_ATTENUATION,
            //glLightfv(GL_LIGHT0 + light, GL_SPECULAR, sunSpecular);// ...GL_LINEAR_ATTENUATION
            // GL_QUADRATIC_ATTENUATION
            glEnable(GL_LIGHT0 + light);
        } else {
            glDisable(GL_LIGHT0 + light);
        }
    }
    const GLfloat modelAmbient[] = { (GLfloat) .2, (GLfloat) .2, (GLfloat) .2, (GLfloat) 1. }; // the "background"
                                                                                               // ambient light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmbient); // GL_LIGHT_MODEL_AMBIENT, GL_LIGHT_MODEL_COLOR_CONTROL,
    //glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // ...GL_LIGHT_MODEL_LOCAL_VIEWER,
    // GL_LIGHT_MODEL_TWO_SIDE

    glShadeModel(GL_SMOOTH); // SMOOTH or FLAT
    glEnable(GL_NORMALIZE);
    _lightsGenerated = true;
}
