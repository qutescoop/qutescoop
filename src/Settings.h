/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "_pch.h"

class Settings {
    public:
        static QString fileName();

        // export/import
        static void exportToFile(QString fileName);
        static void importFromFile(QString fileName);

        // data directory
        static QString dataDirectory(const QString& composeFilePath = QString(""));

        // saved settings
        static void saveState(const QByteArray& state);
        static QByteArray savedState();
        static void saveGeometry(const QByteArray& state);
        static QByteArray savedGeometry();
        static void saveSize(const QSize& size);
        static QSize savedSize();
        static void savePosition(const QPoint& pos);
        static QPoint savedPosition();

        static bool shootScreenshots();
        static void setShootScreenshots(bool value);
        static int screenshotMethod();
        static void setScreenshotMethod(int value);
        static QString screenshotFormat();
        static void setScreenshotFormat(const QString &value);

        static int downloadInterval();
        static void setDownloadInterval(int value);
        static bool downloadOnStartup();
        static void setDownloadOnStartup(bool value);
        static bool downloadPeriodically();
        static void setDownloadPeriodically(bool value);

        static bool useSupFile();
        static void setUseSupFile(bool value);

        static int downloadNetwork();
        static void setDownloadNetwork(int i);
        static QString downloadNetworkName();
        static QString userDownloadLocation();
        static void setUserDownloadLocation(const QString& location);
        static QString statusLocation();
        static void setStatusLocation(const QString& location);
        static QString bookingsLocation();
        static void setBookingsLocation(const QString& value);
        static bool downloadBookings();
        static void setDownloadBookings(bool value);
        static bool bookingsPeriodically();
        static void setBookingsPeriodically(bool value);
        static int bookingsInterval();
        static void setBookingsInterval(int value);

        static bool useProxy();
        static void setUseProxy(bool value);
        static QString proxyServer();
        static void setProxyServer(const QString& server);
        static int proxyPort();
        static void setProxyPort(int value);
        static QString proxyUser();
        static void setProxyUser(QString user);
        static QString proxyPassword();
        static void setProxyPassword(QString password);

        static bool filterTraffic();
        static void setFilterTraffic(bool v);
        static int filterDistance();
        static void setFilterDistance(int v);
        static double filterArriving();
        static void setFilterArriving(double v);

        static bool showAirportCongestion();
        static void setAirportCongestion(bool v);
        static int airportCongestionMinimum();
        static void setAirportCongestionMinimum(int v);
        static QColor airportCongestionBorderLineColor();
        static void setAirportCongestionBorderLineColor(const QColor& color);
        static double airportCongestionBorderLineStrength();
        static void setAirportCongestionBorderLineStrength(double value);

        static int timelineSeconds();
        static void setTimelineSeconds(int value);

        static bool displaySmoothLines();
        static void setDisplaySmoothLines(bool value);
        static bool displaySmoothDots();
        static void setDisplaySmoothDots(bool value);

        static QString navdataDirectory();
        static void setNavdataDirectory(const QString& directory);
        static bool useNavdata();
        static void setUseNavdata(bool value);

        static bool showAllWaypoints();
        static void setShowAllWaypoints(bool value);

        static int metarDownloadInterval();
        static void setMetarDownloadInterval(int minutes);

        static bool showCTR();
        static void setShowCTR(bool value);
        static bool showAPP();
        static void setShowAPP(bool value);
        static bool showTWR();
        static void setShowTWR(bool value);
        static bool showGND();
        static void setShowGND(bool value);
        static bool showAllSectors();
        static void setShowAllSectors(bool value);
        static bool showSonde();
        static void setShowSonde(bool value);
        static int sondeAlt_1k();
        static void setSondeAlt_1k(int value);
        static QColor windColor();
        static void setWindColor(const QColor& value);
        static int windArrowSize();
        static void setWindArrowSize(int value);
        static bool showRouteFix();
        static void setShowRouteFix(bool value);
        static bool showPilotsLabels();
        static void setShowPilotsLabels(bool value);
        static bool showClouds();
        static void setShowClouds(bool value);
        static bool highlightFriends();
        static void setHighlightFriends(bool value);
        static bool glStippleLines();
        static void setGlStippleLines(bool value);
        static bool glBlending();
        static void setGlBlending(bool value);
        static bool glLighting();
        static void setEnableLighting(bool value);
        static int glLights();
        static void setGlLights(int value);

        static int glLightsSpread();
        static void setGlLightsSpread(int value);

        static int glCirclePointEach();
        static void setGlCirclePointEach(int value);

        static bool glTextures();
        static void setGlTextures(bool value);

        static QString glTextureEarth();
        static void setGlTextureEarth(QString value);

        static QColor sunLightColor();
        static void setSunLightColor(const QColor& color);

        static QColor specularColor();
        static void setSpecularColor(const QColor& color);

        static double earthShininess();
        static void setEarthShininess(double strength);

        static QString stylesheet();
        static void setStylesheet(const QString& value);

        static int earthGridEach();
        static void setEarthGridEach(int value);

        static QColor backgroundColor();
        static void setBackgroundColor(const QColor& color);

        static QColor globeColor();
        static void setGlobeColor(const QColor& color);

        static QColor gridLineColor();
        static void setGridLineColor(const QColor& color);

        static double gridLineStrength();
        static void setGridLineStrength(double strength);

        static QColor coastLineColor();
        static void setCoastLineColor(const QColor& color);

        static double coastLineStrength();
        static void setCoastLineStrength(double strength);

        static QColor countryLineColor();
        static void setCountryLineColor(const QColor& color);

        static double countryLineStrength();
        static void setCountryLineStrength(double strength);

        static QColor firBorderLineColor();
        static void setFirBorderLineColor(const QColor& color);

        static double firBorderLineStrength();
        static void setFirBorderLineStrength(double strength);

        static QColor firFontColor();
        static void setFirFontColor(const QColor& color);

        static QColor firFillColor();
        static void setFirFillColor(const QColor& color);

        static QFont firFont();
        static void setFirFont(const QFont& font);

        static QColor airportFontColor();
        static void setAirportFontColor(const QColor& color);

        static QColor airportDotColor();
        static void setAirportDotColor(const QColor& color);

        static double airportDotSize();
        static void setAirportDotSize(double value);

        static QFont airportFont();
        static void setAirportFont(const QFont& font);

        static bool showInactiveAirports();
        static void setShowInactiveAirports(const bool& value);

        static QColor inactiveAirportFontColor();
        static void setInactiveAirportFontColor(const QColor& color);

        static QColor inactiveAirportDotColor();
        static void setInactiveAirportDotColor(const QColor& color);

        static double inactiveAirportDotSize();
        static void setInactiveAirportDotSize(double value);

        static QFont inactiveAirportFont();
        static void setInactiveAirportFont(const QFont& font);

        static QColor appBorderLineColor();
        static void setAppBorderLineColor(const QColor& color);

        static double appBorderLineStrength();
        static void setAppBorderLineStrength(double value);

        static QColor appCenterColor();
        static void setAppCenterColor(const QColor& color);

        static QColor appMarginColor();
        static void setAppMarginColor(const QColor& color);

        static QColor twrMarginColor();
        static void setTwrMarginColor(const QColor& color);

        static QColor twrCenterColor();
        static void setTwrCenterColor(const QColor& color);

        static QColor gndBorderLineColor();
        static void setGndBorderLineColor(const QColor& color);

        static double gndBorderLineStrength();
        static void setGndBorderLineStrength(double value);

        static QColor gndFillColor();
        static void setGndFillColor(const QColor& color);

        static QColor pilotFontColor();
        static void setPilotFontColor(const QColor& color);

        static QFont pilotFont();
        static void setPilotFont(const QFont& font);

        static QColor pilotDotColor();
        static void setPilotDotColor(const QColor& color);

        static double pilotDotSize();
        static void setPilotDotSize(double value);

        static QColor timeLineColor();
        static void setTimeLineColor(const QColor& color);

        static bool showUsedWaypoints();
        static void setShowUsedWaypoints(bool value);

        static QColor waypointsFontColor();
        static void setWaypointsFontColor(const QColor& color);

        static QColor waypointsDotColor();
        static void setWaypointsDotColor(const QColor& color);

        static double waypointsDotSize();
        static void setWaypointsDotSize(double value);

        static QFont waypointsFont();
        static void setWaypointsFont(const QFont& font);

        static QColor depLineColor();
        static void setDepLineColor(const QColor& color);

        static QColor destLineColor();
        static void setDestLineColor(const QColor& color);

        static double timeLineStrength();
        static void setTimeLineStrength(double value);

        static double depLineStrength();
        static void setDepLineStrength(double value);

        static double destLineStrength();
        static void setDestLineStrength(double value);

        static bool depLineDashed();
        static void setDepLineDashed(bool value);

        static bool destLineDashed();
        static void setDestLineDashed(bool value);

        static QColor highlightColor();
        static void setHighlightColor(QColor& color);

        static double highlightLineWidth();
        static void setHighlightLineWidth(double value);

        static bool useHighlightAnimation();
        static void setUseHighlightAnimation(bool value);

        static bool checkForUpdates();
        static void setCheckForUpdates(bool value);

        static bool sendVersionInformation();
        static void setSendVersionInformation(bool value);

        static QString updateVersionNumber();
        static void setUpdateVersionNumber(const QString& version);

        static void rememberedMapPosition(double *xrot, double *yrot, double *zrot, double *zoom, int nr);
        static void setRememberedMapPosition(double xrot, double yrot, double zrot, double zoom, int nr);
        static bool rememberMapPositionOnClose();
        static void setRememberMapPositionOnClose(bool val);

        static int maxLabels();
        static void setMaxLabels(int maxLabels);

        static bool simpleLabels();
        static void setSimpleLabels(bool value);

        static QStringList friends();
        static void addFriend(const QString& friendId);
        static void removeFriend(const QString& friendId);

        static bool resetOnNextStart();
        static void setResetOnNextStart(bool value);

        static bool saveWhazzupData();
        static void setSaveWhazzupData(const bool value);

        static bool downloadClouds();
        static void setDownloadClouds(const bool value);

        static bool useHighResClouds();
        static void setUseHighResClouds(const bool value);

        static int wheelMax();
        static void setWheelMax(const int value);

        static double zoomFactor();
        static void setZoomFactor(const double value);

        static bool useSelectionRectangle();
        static void setUseSelctionRectangle(const bool value);

        static QSize preferencesDialogSize();
        static void setPreferencesDialogSize(const QSize& value);

        static QPoint preferencesDialogPos();
        static void setPreferencesDialogPos(const QPoint& value);

        static QByteArray preferencesDialogGeometry();
        static void setPreferencesDialogGeometry(const QByteArray& value);


        static QSize airportDetailsSize();
        static void setAirportDetailsSize(const QSize& value);

        static QPoint airportDetailsPos();
        static void setAirportDetailsPos(const QPoint& value);

        static QByteArray airportDetailsGeometry();
        static void setAirportDetailsGeometry(const QByteArray& value);


        static QSize bookAtcDialogSize();
        static void setBookAtcDialogSize(const QSize& value);

        static QPoint bookAtcDialogPos();
        static void setBookAtcDialogPos(const QPoint& value);

        static QByteArray bookAtcDialogGeometry();
        static void setBookAtcDialogGeometry(const QByteArray& value);


        static QSize controllerDetailsSize();
        static void setControllerDetailsSize(const QSize& value);

        static QPoint controllerDetailsPos();
        static void setControllerDetailsPos(const QPoint& value);

        static QByteArray controllerDetailsGeometry();
        static void setControllerDetailsGeometry(const QByteArray& value);


        static QSize listClientsDialogSize();
        static void setListClientsDialogSize(const QSize& value);

        static QPoint listClientsDialogPos();
        static void setListClientsDialogPos(const QPoint& value);

        static QByteArray listClientsDialogGeometry();
        static void setListClientsDialogGeometry(const QByteArray& value);


        static QSize pilotDetailsSize();
        static void setPilotDetailsSize(const QSize& value);

        static QPoint pilotDetailsPos();
        static void setPilotDetailsPos(const QPoint& value);

        static QByteArray pilotDetailsGeometry();
        static void setPilotDetailsGeometry(const QByteArray& value);


        static QSize planFlightDialogSize();
        static void setPlanFlightDialogSize(const QSize& value);

        static QPoint planFlightDialogPos();
        static void setPlanFlightDialogPos(const QPoint& value);

        static QByteArray planFlightDialogGeometry();
        static void setPlanFlightDialogGeometry(const QByteArray& value);

        static bool maximized();
        static void saveMaximized(const bool val);

        static QString remoteDataRepository();

        static void setSondeAltSecondarySpan_1k(int value);
        static int sondeAltSecondarySpan_1k();
        static QString sondeUrl();
        static void setSondeUrl(QString val);
        static void setSondeFont(const QFont &font);
        static QFont sondeFont();
private:
        static QSettings *instance();
};

#endif /*SETTINGS_H_*/
