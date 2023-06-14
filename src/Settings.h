#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QtCore>

class Settings {
    public:
        struct DialogPreferences {
            QSize size;
            QPoint pos;
            QByteArray geometry;
        };

        static QString fileName();

        // export/import
        static void exportToFile(QString fileName);
        static void importFromFile(QString fileName);

        // data directory
        static QString dataDirectory(const QString& composeFilePath = QString(""));

        // "constant" settings
        static const QColor lightTextColor();

        // saved settings
        static void saveState(const QByteArray& state);
        static QByteArray savedState();
        static void saveGeometry(const QByteArray& state);
        static QByteArray savedGeometry();
        static void saveSize(const QSize& size);
        static QSize savedSize();
        static void savePosition(const QPoint& pos);
        static QPoint savedPosition();

        static int downloadInterval();
        static void setDownloadInterval(int value);
        static bool downloadOnStartup();
        static void setDownloadOnStartup(bool value);
        static bool downloadPeriodically();
        static void setDownloadPeriodically(bool value);

        static int downloadNetwork();
        static void setDownloadNetwork(int i);
        static QString downloadNetworkName();
        static QString userDownloadLocation();
        static void setUserDownloadLocation(const QString& location);
        static QString statusLocation();
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

        static bool showRouteFix();
        static void setShowRouteFix(bool value);

        static bool showPilotsLabels();
        static void setShowPilotsLabels(bool value);

        static bool highlightFriends();
        static void setHighlightFriends(bool value);

        // Display/OpenGL

        static bool showFps();
        static void setShowFps(bool value);

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

        // Display/hover
        static QColor labelHoveredBgColor();
        static void setLabelHoveredBgColor(const QColor& color);

        static QColor labelHoveredBgDarkColor();
        static void setLabelHoveredBgDarkColor(const QColor& color);

        static bool showToolTips();
        static void setShowToolTips(const bool value);

        // sectors

        static QColor firBorderLineColor();
        static void setFirBorderLineColor(const QColor& color);

        static double firBorderLineStrength();
        static void setFirBorderLineStrength(double strength);

        static QColor firFillColor();
        static void setFirFillColor(const QColor& color);

        static QColor firHighlightedBorderLineColor();
        static void setFirHighlightedBorderLineColor(const QColor& color);

        static double firHighlightedBorderLineStrength();
        static void setFirHighlightedBorderLineStrength(double strength);

        static QColor firHighlightedFillColor();
        static void setFirHighlightedFillColor(const QColor& color);

        static QColor firFontColor();
        static void setFirFontColor(const QColor& color);

        static QFont firFont();
        static void setFirFont(const QFont& font);

        static QColor firFontSecondaryColor();
        static void setFirFontSecondaryColor(const QColor& color);

        static QFont firFontSecondary();
        static void setFirFontSecondary(const QFont& font);

        static QString firPrimaryContent();
        static void setFirPrimaryContent(const QString& value);

        static QString firPrimaryContentHovered();
        static void setFirPrimaryContentHovered(const QString& value);

        static QString firSecondaryContent();
        static void setFirSecondaryContent(const QString& value);

        static QString firSecondaryContentHovered();
        static void setFirSecondaryContentHovered(const QString& value);


        // airports

        static QFont airportFont();
        static void setAirportFont(const QFont& font);

        static QColor airportFontColor();
        static void setAirportFontColor(const QColor& color);

        static QFont airportFontSecondary();
        static void setAirportFontSecondary(const QFont& font);

        static QColor airportFontSecondaryColor();
        static void setAirportFontSecondaryColor(const QColor& color);

        static QString airportPrimaryContent();
        static void setAirportPrimaryContent(const QString& value);

        static QString airportPrimaryContentHovered();
        static void setAirportPrimaryContentHovered(const QString& value);

        static QString airportSecondaryContent();
        static void setAirportSecondaryContent(const QString& value);

        static QString airportSecondaryContentHovered();
        static void setAirportSecondaryContentHovered(const QString& value);

        static QColor airportDotColor();
        static void setAirportDotColor(const QColor& color);

        static double airportDotSize();
        static void setAirportDotSize(double value);

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

        static QColor twrBorderLineColor();
        static void setTwrBorderLineColor(const QColor& color);

        static double twrBorderLineWidth();
        static void setTwrBorderLineStrength(double value);

        static QColor appBorderLineColor();
        static void setAppBorderLineColor(const QColor& color);

        static double appBorderLineWidth();
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

        static double gndBorderLineWidth();
        static void setGndBorderLineStrength(double value);

        static QColor gndFillColor();
        static void setGndFillColor(const QColor& color);

        static QColor delBorderLineColor();
        static void setDelBorderLineColor(const QColor& color);

        static double delBorderLineWidth();
        static void setDelBorderLineStrength(double value);

        static QColor delFillColor();
        static void setDelFillColor(const QColor& color);

        static QColor pilotFontColor();
        static void setPilotFontColor(const QColor& color);

        static QFont pilotFont();
        static void setPilotFont(const QFont& font);

        static QColor pilotFontSecondaryColor();
        static void setPilotFontSecondaryColor(const QColor& color);

        static QFont pilotFontSecondary();
        static void setPilotFontSecondary(const QFont& font);

        static QString pilotPrimaryContent();
        static void setPilotPrimaryContent(const QString& value);

        static QString pilotPrimaryContentHovered();
        static void setPilotPrimaryContentHovered(const QString& value);

        static QString pilotSecondaryContent();
        static void setPilotSecondaryContent(const QString& value);

        static QString pilotSecondaryContentHovered();
        static void setPilotSecondaryContentHovered(const QString& value);

        static QColor pilotDotColor();
        static void setPilotDotColor(const QColor& color);

        static double pilotDotSize();
        static void setPilotDotSize(double value);

        static QColor leaderLineColor();
        static void setLeaderLineColor(const QColor& color);

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

        static bool showRoutes();
        static void setShowRoutes(bool value);

        static bool onlyShowImmediateRoutePart();
        static void setOnlyShowImmediateRoutePart(bool value);

        static QColor depLineColor();
        static void setDepLineColor(const QColor& color);

        static int destImmediateDurationMin();
        static void setDestImmediateDurationMin(int value);

        static QColor destImmediateLineColor();
        static void setDestImmediateLineColor(const QColor& color);

        static double destImmediateLineStrength();
        static void setDestImmediateLineStrength(double value);

        static double destLineStrength();
        static void setDestLineStrength(double value);

        static QColor destLineColor();
        static void setDestLineColor(const QColor& color);

        static double timeLineStrength();
        static void setTimeLineStrength(double value);

        static double depLineStrength();
        static void setDepLineStrength(double value);


        static bool depLineDashed();
        static void setDepLineDashed(bool value);

        static bool destLineDashed();
        static void setDestLineDashed(bool value);

        static QColor friendsHighlightColor();
        static void setFriendsHighlightColor(QColor& color);

        static QColor friendsPilotDotColor();
        static void setFriendsPilotDotColor(QColor& color);

        static QColor friendsAirportDotColor();
        static void setFriendsAirportDotColor(QColor& color);

        static QColor friendsPilotLabelRectColor();
        static void setFriendsPilotLabelRectColor(QColor& color);

        static QColor friendsAirportLabelRectColor();
        static void setFriendsAirportLabelRectColor(QColor& color);

        static QColor friendsSectorLabelRectColor();
        static void setFriendsSectorLabelRectColor(QColor& color);

        static double highlightLineWidth();
        static void setHighlightLineWidth(double value);

        static bool useHighlightAnimation();
        static void setUseHighlightAnimation(bool value);

        static bool checkForUpdates();
        static void setCheckForUpdates(bool value);

        static void rememberedMapPosition(double* xrot, double* yrot, double* zrot, double* zoom, int nr);
        static void setRememberedMapPosition(double xrot, double yrot, double zrot, double zoom, int nr);
        static bool rememberMapPositionOnClose();
        static void setRememberMapPositionOnClose(bool val);

        static int maxLabels();
        static void setMaxLabels(int maxLabels);

        static const QStringList friends();
        static void addFriend(const QString& friendId);
        static void removeFriend(const QString& friendId);

        static const QString clientAlias(const QString& userId);
        static void setClientAlias(const QString& userId, const QString& alias = QString());

        static bool resetOnNextStart();
        static void setResetOnNextStart(bool value);

        static bool saveWhazzupData();
        static void setSaveWhazzupData(const bool value);

        static int wheelMax();
        static void setWheelMax(const int value);

        static double zoomFactor();
        static void setZoomFactor(const double value);

        static bool useSelectionRectangle();
        static void setUseSelctionRectangle(const bool value);

        static DialogPreferences dialogPreferences(const QString& name);
        static void setDialogPreferences(const QString& name, const DialogPreferences& dialogPreferences);

        static bool maximized();
        static void saveMaximized(const bool val);

        static Qt::SortOrder airportDialogAtcSortOrder();
        static void setAirportDialogAtcSortOrder(Qt::SortOrder);

        static QMap<QString, QVariant> airportDialogAtcExpandedByType();
        static void setAirportDialogAtcExpandedByType(const QMap<QString, QVariant>&);

        static QString remoteDataRepository();
    private:
        static QSettings* instance();
        static void migrate(QSettings*);
};

#endif /*SETTINGS_H_*/
