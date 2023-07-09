#include "Settings.h"

#include "GuiMessage.h"
#include "Whazzup.h"
#include "dialogs/AirportDetails.h"
#include "dialogs/ControllerDetails.h"
#include "dialogs/PilotDetails.h"
#include "dialogs/Window.h"

//singleton instance
QSettings* settingsInstance = 0;
QSettings* Settings::instance() {
    if (settingsInstance == 0) {
        settingsInstance = new QSettings();
        if (Settings::resetOnNextStart()) {
            settingsInstance->clear();
            Settings::setResetOnNextStart(false);
        }
        migrate(settingsInstance);
    }

    return settingsInstance;
}

void Settings::migrate(QSettings* settingsInstance) {
    struct Migration {
        int version; // settings version this migrates to
        const char* name;
        std::function<void()> run;
    };

    const std::vector<Migration> migrations {
        {
            1,
            "Whazzup status",
            [settingsInstance]() {
                if (
                    settingsInstance->value("download/network", 0).toInt() == 1
                    && settingsInstance->value("download/statusLocation", "").toString() == "http://status.vatsim.net/"
                ) {
                    settingsInstance->setValue("download/network", 0);
                    qDebug() << "Found a user defined network, but VATSIM status location. Migrated.";
                }
                settingsInstance->remove("download/statusLocation");
            }
        },
        {
            2,
            "Invalid aliases",
            [settingsInstance]() {
                settingsInstance->beginGroup("clients");
                QStringList keys = settingsInstance->childKeys();
                foreach (const auto key, keys) {
                    if (key.startsWith("alias_")) {
                        QString id = key.mid(6);
                        if (!Client::isValidID(id)) {
                            settingsInstance->remove(key);
                            qDebug() << "Found an alias for (invalid) client " << id << " and removed it. For more information see https://github.com/qutescoop/qutescoop/issues/130";
                        }
                    }
                }
                settingsInstance->endGroup();
                QStringList friendList = settingsInstance->value("friends/friendList", QStringList()).toStringList();
                foreach (const auto friendID, friendList) {
                    if (!Client::isValidID(friendID)) {
                        friendList.removeAt(friendList.indexOf(friendID));
                        qDebug() << "Found a friend list entry for (invalid) client " << friendID << " and removed it. For more information see https://github.com/qutescoop/qutescoop/issues/130";
                    }
                }
                settingsInstance->setValue("friends/friendList", friendList);
            }
        },
        {
            3,
            "Bookings URL",
            [settingsInstance]() {
                const auto loc = settingsInstance->value("download/bookingsLocation").toString();
                if (!loc.isEmpty() && loc != "http://vatbook.euroutepro.com/servinfo.asp") {
                    GuiMessages::criticalUserInteraction(
                        QString("You have set the location for bookings to %1.\nThis is different from the old default location. Due to an update in QuteScoop the old format for bookings is no longer supported.\n\nYou will be migrated to the new default VATSIM bookings URL.")
                        .arg(settingsInstance->value("download/bookingsLocation").toString()),
                        "Update of bookings format"
                    );
                }
                settingsInstance->setValue("download/bookingsLocation", "https://atc-bookings.vatsim.net/api/booking");
            }
        },
        {
            4,
            "Removing typos in setting keys",
            [settingsInstance]() {
                const QList<QPair<QString, QString> > fromTos = {
                    { "airportDisplay/dotSizer", "airportDisplay/dotSize" },
                    { "airportDisplay/inactiveDotSizer", "airportDisplay/inactiveDotSize" },
                    { "pilotDisplay/waypointsDotSizer", "pilotDisplay/waypointsDotSize" },
                };

                foreach (const auto fromTo, fromTos) {
                    if (settingsInstance->contains(fromTo.first)) {
                        settingsInstance->setValue(
                            fromTo.second,
                            settingsInstance->value(fromTo.first).value<double>()
                        );
                        qDebug() << "replaced" << fromTo.first << "with" << fromTo.second;
                        settingsInstance->remove(fromTo.first);
                    }
                }
            }
        },
        {
            5,
            "Remove obsolete settings",
            [settingsInstance]() {
                QList<QString> obsoleteSettings = {
                    "database/showAllWaypoints",
                    "screenshots/shootScreenshots",
                    "screenshots/method",
                    "screenshots/format",
                    "download/sendVersionInfo",
                    "download/updateVersionNumber",
                    "display/showALLSectors"
                };

                foreach (const auto setting, obsoleteSettings) {
                    settingsInstance->remove(setting);
                }
            }
        },
        {
            6,
            "Whazzup download interval is now seconds",
            [settingsInstance]() {
                settingsInstance->setValue(
                    "download/interval",
                    settingsInstance->value("download/interval", 1).value<int>() * 60
                );
            }
        },
        {
            7,
            "Adding more congestion options",
            [settingsInstance]() {
                // QColor
                QList<QPair<QString, QString> > fromTos = {
                    { "airportTraffic/borderLineColor", "airportTraffic/congestionColorMin" },
                    { "airportTraffic/minimumMovements", "airportTraffic/congestionMovementsMin" },
                    { "airportTraffic/borderLineStrength", "airportTraffic/borderLineStrengthMin" },
                };
                foreach (const auto fromTo, fromTos) {
                    if (settingsInstance->contains(fromTo.first)) {
                        settingsInstance->setValue(
                            fromTo.second,
                            settingsInstance->value(fromTo.first)
                        );
                        qDebug() << "replaced" << fromTo.first << "with" << fromTo.second;
                        settingsInstance->remove(fromTo.first);
                    }
                }
            }
        },
    };

    foreach (const auto &migration, migrations) {
        if (settingsInstance->value("settings/version", 0).toInt() < migration.version) {
            qCritical() << QString("Migrating to settings version v%1: %2...").arg(migration.version).arg(migration.name);
            migration.run();
            settingsInstance->setValue("settings/version", migration.version);
        }
    }
}

/**
 * path to .ini or Windows registry path (\HKEY_CURRENT_USER\Software\QuteScoop\QuteScoop)
 */
QString Settings::fileName() {
    return instance()->fileName();
}

void Settings::exportToFile(QString fileName) {
    QSettings* settings_file = new QSettings(fileName, QSettings::IniFormat);
    QStringList settings_keys = instance()->allKeys();
    foreach (const QString &key, instance()->allKeys()) {
        settings_file->setValue(key, instance()->value(key));
    }
    delete settings_file;
}

void Settings::importFromFile(QString fileName) {
    QSettings* settings_file = new QSettings(fileName, QSettings::IniFormat);
    foreach (const QString &key, settings_file->allKeys()) {
        instance()->setValue(key, settings_file->value(key));
    }
    delete settings_file;
}

/**
 * @param composeFilePath path/file (/ is multi-platform, also on Win)
 * @returns fully qualified path to the 'application data directory'.
 **/
QString Settings::dataDirectory(const QString &composeFilePath) {
    return QString("%1/%2")
        .arg(QCoreApplication::applicationDirPath(), composeFilePath);
}

const QColor Settings::lightTextColor() {
    auto _default = QGuiApplication::palette().text().color();
    _default.setAlphaF(.5);

    return instance()->value("display/lightTextColor", _default).value<QColor>();
}

int Settings::downloadInterval() {
    return instance()->value("download/interval", 60).toInt();
}
void Settings::setDownloadInterval(int value) {
    instance()->setValue("download/interval", value);
}

bool Settings::downloadOnStartup() {
    // policy: see https://github.com/vatsimnetwork/developer-info/wiki/Data-Feeds-Live-Data
    return instance()->value("download/downloadOnStartup", true).toBool();
}

void Settings::setDownloadOnStartup(bool value) {
    instance()->setValue("download/downloadOnStartup", value);
}

bool Settings::downloadPeriodically() {
    return instance()->value("download/downloadPeriodically", true).toBool();
}

void Settings::setDownloadPeriodically(bool value) {
    instance()->setValue("download/downloadPeriodically", value);
}

bool Settings::downloadBookings() {
    return instance()->value("download/downloadBookings", downloadNetwork() == 1).toBool();
}

void Settings::setDownloadBookings(bool value) {
    instance()->setValue("download/downloadBookings", value);
}

QString Settings::bookingsLocation() {
    return instance()->value("download/bookingsLocation", "http://vatbook.euroutepro.com/servinfo.asp").toString();
}

void Settings::setBookingsLocation(const QString& value) {
    instance()->setValue("download/bookingsLocation", value);
}

bool Settings::bookingsPeriodically() {
    return instance()->value("download/bookingsPeriodically", true).toBool();
}

void Settings::setBookingsPeriodically(bool value) {
    instance()->setValue("download/bookingsPeriodically", value);
}

int Settings::bookingsInterval() {
    return instance()->value("download/bookingsInterval", 30).toInt();
}

void Settings::setBookingsInterval(int value) {
    instance()->setValue("download/bookingsInterval", value);
}

int Settings::downloadNetwork() {
    return instance()->value("download/network", 0).toInt();
}

void Settings::setDownloadNetwork(int i) {
    instance()->setValue("download/network", i);
    Whazzup::instance()->setStatusLocation(statusLocation());
}

QString Settings::downloadNetworkName() {
    switch (downloadNetwork()) {
        case 0: return "VATSIM"; break;
        case 1: return "User Network"; break;
    }
    return "Unknown";
}

QString Settings::userDownloadLocation() {
    return instance()->value("download/userLocation", "http://www.network.org/status.txt").toString();
}

void Settings::setUserDownloadLocation(const QString& location) {
    instance()->setValue("download/userLocation", location);
}

bool Settings::checkForUpdates() {
    return instance()->value("download/checkForUpdates", true).toBool();
}

void Settings::setCheckForUpdates(bool value) {
    instance()->setValue("download/checkForUpdates", value);
}

QString Settings::statusLocation() {
    switch (downloadNetwork()) {
        case 0: // VATSIM
            return "https://status.vatsim.net/status.json";
        case 1: // user defined
            return userDownloadLocation();
    }
    return QString();
}

bool Settings::useProxy() {
    return instance()->value("proxy/useProxy", false).toBool();
}

void Settings::setUseProxy(bool value) {
    instance()->setValue("proxy/useProxy", value);
}

QString Settings::proxyServer() {
    return instance()->value("proxy/server").toString();
}

void Settings::setProxyServer(const QString& server) {
    instance()->setValue("proxy/server", server);
}

int Settings::proxyPort() {
    return instance()->value("proxy/port", 8080).toInt();
}

void Settings::setProxyPort(int value) {
    instance()->setValue("proxy/port", value);
}

QString Settings::proxyUser() {
    return instance()->value("proxy/user").toString();
}

void Settings::setProxyUser(QString user) {
    instance()->setValue("proxy/user", user);
}

QString Settings::proxyPassword() {
    return instance()->value("proxy/password").toString();
}

void Settings::setProxyPassword(QString password) {
    instance()->setValue("proxy/password", password);
}

QString Settings::navdataDirectory() {
    return instance()->value("database/path").toString();
}

void Settings::setNavdataDirectory(const QString& directory) {
    instance()->setValue("database/path", directory);
}

bool Settings::useNavdata() {
    return instance()->value("database/use", false).toBool();
}

void Settings::setUseNavdata(bool value) {
    instance()->setValue("database/use", value);
}

int Settings::metarDownloadInterval() {
    return instance()->value("display/metarInterval", 1).toInt();
}

void Settings::setMetarDownloadInterval(int minutes) {
    instance()->setValue("download/metarInterval", minutes);
}

//Display
bool Settings::showCTR() {
    return instance()->value("display/showCTR", true).toBool();
}
void Settings::setShowCTR(bool value) {
    instance()->setValue("display/showCTR", value);
}

bool Settings::showAPP() {
    return instance()->value("display/showAPP", true).toBool();
}
void Settings::setShowAPP(bool value) {
    return instance()->setValue("display/showAPP", value);
}

bool Settings::showTWR() {
    return instance()->value("display/showTWR", true).toBool();
}
void Settings::setShowTWR(bool value) {
    instance()->setValue("display/showTWR", value);
}

bool Settings::showGND() {
    return instance()->value("display/showGND", true).toBool();
}
void Settings::setShowGND(bool value) {
    instance()->setValue("display/showGND", value);
}

bool Settings::showRouteFix() {
    return instance()->value("display/showRouteFix", false).toBool();
}
void Settings::setShowRouteFix(bool value) {
    instance()->setValue("display/showRouteFix", value);
}

bool Settings::showPilotsLabels() {
    return instance()->value("display/showPilotsLabels", true).toBool();
}
void Settings::setShowPilotsLabels(bool value) {
    instance()->setValue("display/showPilotsLabels", value);
}

bool Settings::showInactiveAirports() {
    return instance()->value("display/showInactive", true).toBool();
}
void Settings::setShowInactiveAirports(const bool& value) {
    instance()->setValue("display/showInactive", value);
}

bool Settings::highlightFriends() {
    return instance()->value("display/highlightFriends", false).toBool();
}
void Settings::setHighlightFriends(bool value) {
    instance()->setValue("display/highlightFriends", value);
}

// OpenGL
bool Settings::showFps() {
    return instance()->value("gl/showFps", false).toBool();
}
void Settings::setShowFps(bool value) {
    instance()->setValue("gl/showFps", value);
}

bool Settings::displaySmoothLines() {
    return instance()->value("gl/smoothLines", true).toBool();
}

void Settings::setDisplaySmoothLines(bool value) {
    instance()->setValue("gl/smoothLines", value);
}

bool Settings::glStippleLines() {
    return instance()->value("gl/stippleLines", true).toBool();
}

void Settings::setGlStippleLines(bool value) {
    instance()->setValue("gl/stippleLines", value);
}

bool Settings::displaySmoothDots() {
    return instance()->value("gl/smoothDots", true).toBool();
}

void Settings::setDisplaySmoothDots(bool value) {
    instance()->setValue("gl/smoothDots", value);
}

int Settings::maxLabels() {
    return instance()->value("gl/maxLabels", 130).toInt();
}

void Settings::setMaxLabels(int maxLabels) {
    instance()->setValue("gl/maxLabels", maxLabels);
}

bool Settings::glBlending() {
    return instance()->value("gl/blend", true).toBool();
}
void Settings::setGlBlending(bool value) {
    instance()->setValue("gl/blend", value);
}

int Settings::glCirclePointEach() {
    return instance()->value("gl/circlePointEach", 3).toInt();
}
void Settings::setGlCirclePointEach(int value) {
    instance()->setValue("gl/circlePointEach", value);
}

bool Settings::glLighting() {
    return instance()->value("gl/lighting", true).toBool();
}
void Settings::setEnableLighting(bool value) {
    instance()->setValue("gl/lighting", value);
}

int Settings::glLights() {
    return instance()->value("gl/lights", 6).toInt();
}
void Settings::setGlLights(int value) {
    instance()->setValue("gl/lights", value);
}

int Settings::glLightsSpread() {
    return instance()->value("gl/lightsSpread", 35).toInt();
}
void Settings::setGlLightsSpread(int value) {
    instance()->setValue("gl/lightsSpread", value);
}

bool Settings::glTextures() {
    return instance()->value("gl/earthTexture", true).toBool();
}
void Settings::setGlTextures(bool value) {
    instance()->setValue("gl/earthTexture", value);
}
QString Settings::glTextureEarth() {
    return instance()->value("gl/textureEarth", "4096px.png").toString();
}
void Settings::setGlTextureEarth(QString value) {
    instance()->setValue("gl/textureEarth", value);
}

QColor Settings::sunLightColor() {
    return instance()->value("gl/sunLightColor", QColor::fromRgb(70, 70, 70)).value<QColor>();
}

void Settings::setSunLightColor(const QColor& color) {
    instance()->setValue("gl/sunLightColor", color);
}

QColor Settings::specularColor() {
    return instance()->value("gl/sunSpecularColor", QColor::fromRgb(50, 22, 3)).value<QColor>();
}

void Settings::setSpecularColor(const QColor& color) {
    instance()->setValue("gl/sunSpecularColor", color);
}

double Settings::earthShininess() {
    return instance()->value("gl/earthShininess", 25).toDouble();
}

void Settings::setEarthShininess(double strength) {
    instance()->setValue("gl/earthShininess", strength);
}


// Stylesheet
QString Settings::stylesheet() {
    return instance()->value("display/stylesheet", QString()).toString();
}

void Settings::setStylesheet(const QString& value) {
    instance()->setValue("display/stylesheet", value);
}

// earthspace
int Settings::earthGridEach() {
    return instance()->value("earthSpace/earthGridEach", 30).toInt();
}
void Settings::setEarthGridEach(int value) {
    instance()->setValue("earthSpace/earthGridEach", value);
}

QColor Settings::backgroundColor() {
    return instance()->value("earthSpace/backgroundColor", QColor::fromRgbF(0, 0, 0)).value<QColor>();
}

void Settings::setBackgroundColor(const QColor& color) {
    instance()->setValue("earthSpace/backgroundColor", color);
}

QColor Settings::globeColor() {
    return instance()->value("earthSpace/globeColor", QColor::fromRgb(100, 100, 100)).value<QColor>();
}

void Settings::setGlobeColor(const QColor& color) {
    instance()->setValue("earthSpace/globeColor", color);
}

QColor Settings::gridLineColor() {
    return instance()->value("earthSpace/gridLineColor", QColor::fromRgb(100, 100, 100, 80)).value<QColor>();
}

void Settings::setGridLineColor(const QColor& color) {
    instance()->setValue("earthSpace/gridLineColor", color);
}

double Settings::gridLineStrength() {
    return instance()->value("earthSpace/gridLineStrength", 0.5).toDouble();
}

void Settings::setGridLineStrength(double strength) {
    instance()->setValue("earthSpace/gridLineStrength", strength);
}

QColor Settings::countryLineColor() {
    return instance()->value("earthSpace/countryLineColor", QColor::fromRgb(102, 85, 67, 150)).value<QColor>();
}

void Settings::setCountryLineColor(const QColor& color) {
    instance()->setValue("earthSpace/countryLineColor", color);
}

double Settings::countryLineStrength() {
    return instance()->value("earthSpace/countryLineStrength", 1.).toDouble();
}

void Settings::setCountryLineStrength(double strength) {
    instance()->setValue("earthSpace/countryLineStrength", strength);
}

bool Settings::labelAlwaysBackdropped() {
    return instance()->value("labels/alwaysShowBackdrop", false).toBool();
}

void Settings::setLabelAlwaysBackdropped(const bool v) {
    instance()->setValue("labels/alwaysShowBackdrop", v);
}

QColor Settings::labelHoveredBgColor() {
    return instance()->value("labelHover/labelHoveredBgColor", QColor::fromRgb(255, 255, 255, 190)).value<QColor>();
}

void Settings::setLabelHoveredBgColor(const QColor &color) {
    instance()->setValue("labelHover/labelHoveredBgColor", color);
}

QColor Settings::labelHoveredBgDarkColor() {
    return instance()->value("labelHover/labelHoveredBgDarkColor", QColor::fromRgb(0, 0, 0, 190)).value<QColor>();
}

void Settings::setLabelHoveredBgDarkColor(const QColor &color) {
    instance()->setValue("labelHover/labelHoveredBgDarkColor", color);
}

bool Settings::showToolTips() {
    return instance()->value("mapUi/showTooltips", false).toBool();
}

void Settings::setShowToolTips(const bool v) {
    instance()->setValue("mapUi/showTooltips", v);
}

int Settings::hoverDebounceMs() {
    return instance()->value("labelHover/debounceMs", 50).toInt();
}
void Settings::setHoverDebounceMs(int value) {
    instance()->setValue("labelHover/debounceMs", value);
}

bool Settings::onlyShowHoveredLabels() {
    return instance()->value("mapUi/onlyShowHoveredLabels", false).toBool();
}

void Settings::setOnlyShowHoveredLabels(const bool v) {
    instance()->setValue("mapUi/onlyShowHoveredLabels", v);
}

QColor Settings::coastLineColor() {
    return instance()->value("earthSpace/coastLineColor", QColor::fromRgb(102, 85, 67, 200)).value<QColor>();
}

void Settings::setCoastLineColor(const QColor& color) {
    instance()->setValue("earthSpace/coastLineColor", color);
}

double Settings::coastLineStrength() {
    return instance()->value("earthSpace/coastLineStrength", 2.).toDouble();
}

void Settings::setCoastLineStrength(double strength) {
    instance()->setValue("earthSpace/coastLineStrength", strength);
}

// FIRs
QColor Settings::firBorderLineColor() {
    return instance()->value("firDisplay/borderLineColor", QColor::fromRgb(42, 163, 214, 120)).value<QColor>();
}

void Settings::setFirBorderLineColor(const QColor& color) {
    instance()->setValue("firDisplay/borderLineColor", color);
}

double Settings::firBorderLineStrength() {
    return instance()->value("firDisplay/borderLineStrength", 1.5).toDouble();
}

void Settings::setFirBorderLineStrength(double strength) {
    instance()->setValue("firDisplay/borderLineStrength", strength);
}

QColor Settings::firFontColor() {
    return instance()->value("firDisplay/fontColor", QColor::fromRgb(170, 255, 255)).value<QColor>();
}

void Settings::setFirFontColor(const QColor& color) {
    instance()->setValue("firDisplay/fontColor", color);
}

QFont Settings::firFont() {
    QFont defaultFont;
    defaultFont.setPointSize(13);
    QFont result = instance()->value("firDisplay/font", defaultFont).value<QFont>();
    result.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    return result;
}

void Settings::setFirFont(const QFont& font) {
    instance()->setValue("firDisplay/font", font);
}

QColor Settings::firFontSecondaryColor() {
    return instance()->value("firDisplay/fontSecondaryColor", firFontColor()).value<QColor>();
}

void Settings::setFirFontSecondaryColor(const QColor& color) {
    instance()->setValue("firDisplay/fontSecondaryColor", color);
}

QFont Settings::firFontSecondary() {
    QFont defaultFont = firFont();
    if (defaultFont.pointSize() > -1) {
        defaultFont.setPointSize(defaultFont.pointSize() - 1);
    } else {
        defaultFont.setPixelSize(defaultFont.pixelSize() - 1);
    }

    QFont result = instance()->value("firDisplay/fontSecondary", defaultFont).value<QFont>();
    result.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    return result;
}

void Settings::setFirFontSecondary(const QFont& font) {
    instance()->setValue("firDisplay/fontSecondary", font);
}


QColor Settings::firFillColor() {
    return instance()->value("firDisplay/fillColor", QColor::fromRgb(42, 163, 214, 22)).value<QColor>();
}

void Settings::setFirFillColor(const QColor& color) {
    instance()->setValue("firDisplay/fillColor", color);
}

QColor Settings::firHighlightedBorderLineColor() {
    return instance()->value("firDisplay/borderLineColorHighlighted", QColor::fromRgb(200, 13, 214, 77)).value<QColor>();
}

void Settings::setFirHighlightedBorderLineColor(const QColor& color) {
    instance()->setValue("firDisplay/borderLineColorHighlighted", color);
}

double Settings::firHighlightedBorderLineStrength() {
    return instance()->value("firDisplay/borderLineStrengthHighlighted", 1.5).toDouble();
}

void Settings::setFirHighlightedBorderLineStrength(double strength) {
    instance()->setValue("firDisplay/borderLineStrengthHighlighted", strength);
}

QColor Settings::firHighlightedFillColor() {
    return instance()->value("firDisplay/fillColorHighlighted", QColor::fromRgb(200, 13, 214, 35)).value<QColor>();
}

void Settings::setFirHighlightedFillColor(const QColor& color) {
    instance()->setValue("firDisplay/fillColorHighlighted", color);
}

QString Settings::firPrimaryContent() {
    return instance()->value("firDisplay/primaryContent", "{sectorOrLogin}").toString();
}

void Settings::setFirPrimaryContent(const QString &value) {
    instance()->setValue("firDisplay/primaryContent", value);
}

QString Settings::firPrimaryContentHovered() {
    return instance()->value("firDisplay/primaryContentHovered", "{sectorOrLogin}").toString();
}

void Settings::setFirPrimaryContentHovered(const QString &value) {
    instance()->setValue("firDisplay/primaryContentHovered", value);
}

QString Settings::firSecondaryContent() {
    return instance()->value("firDisplay/secondaryContent", "{#cpdlc}@{cpdlc}{/cpdlc}{#livestream}📺{/livestream}").toString();
}

void Settings::setFirSecondaryContent(const QString &value) {
    instance()->setValue("firDisplay/secondaryContent", value);
}

QString Settings::firSecondaryContentHovered() {
    return instance()->value("firDisplay/secondaryContentHovered", "{#cpdlc}CPDLC@{cpdlc}{/cpdlc}\n{sector} {frequency}\n{name} {rating}\n{livestream}").toString();
}

void Settings::setFirSecondaryContentHovered(const QString &value) {
    instance()->setValue("firDisplay/secondaryContentHovered", value);
}


//airport
QFont Settings::airportFont() {
    QFont defaultResult;
    defaultResult.setPointSize(9);
    QFont result = instance()->value("airportDisplay/font", defaultResult).value<QFont>();
    result.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    return result;
}

void Settings::setAirportFont(const QFont& font) {
    instance()->setValue("airportDisplay/font", font);
}

QColor Settings::airportFontColor() {
    return instance()->value("airportDisplay/fontColor", QColor::fromRgb(255, 255, 127, 200)).value<QColor>();
}

void Settings::setAirportFontColor(const QColor& color) {
    instance()->setValue("airportDisplay/fontColor", color);
}

QFont Settings::airportFontSecondary() {
    QFont defaultResult;
    defaultResult.setPointSize(8);
    QFont result = instance()->value("airportDisplay/fontSecondary", defaultResult).value<QFont>();
    result.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    return result;
}

void Settings::setAirportFontSecondary(const QFont& font) {
    instance()->setValue("airportDisplay/fontSecondary", font);
}

QColor Settings::airportFontSecondaryColor() {
    return instance()->value("airportDisplay/fontSecondaryColor", QColor::fromRgb(255, 255, 255, 180)).value<QColor>();
}

void Settings::setAirportFontSecondaryColor(const QColor& color) {
    instance()->setValue("airportDisplay/fontSecondaryColor", color);
}

QString Settings::airportPrimaryContent() {
    return instance()->value("airportDisplay/primaryContent", "{code}{#pdc}@{/pdc} {> trafficArrows} {#livestream}📺{/livestream}").toString();
}

void Settings::setAirportPrimaryContent(const QString &value) {
    instance()->setValue("airportDisplay/primaryContent", value);
}

QString Settings::airportPrimaryContentHovered() {
    return instance()->value("airportDisplay/primaryContentHovered", "{code}{#pdc}@{/pdc} {> trafficArrows} {#livestream}📺{/livestream}").toString();
}

void Settings::setAirportPrimaryContentHovered(const QString &value) {
    instance()->setValue("airportDisplay/primaryContentHovered", value);
}

QString Settings::airportSecondaryContent() {
    return instance()->value("airportDisplay/secondaryContent", "").toString();
}

void Settings::setAirportSecondaryContent(const QString &value) {
    instance()->setValue("airportDisplay/secondaryContent", value);
}

QString Settings::airportSecondaryContentHovered() {
    return instance()->value("airportDisplay/secondaryContentHovered", "{prettyName}\n{#pdc}PDC@{pdc}{/pdc}\n{frequencies}\n{livestream}").toString();
}

void Settings::setAirportSecondaryContentHovered(const QString &value) {
    instance()->setValue("airportDisplay/secondaryContentHovered", value);
}

QColor Settings::airportDotColor() {
    return instance()->value("airportDisplay/dotColor", QColor::fromRgb(85, 170, 255, 150)).value<QColor>();
}

void Settings::setAirportDotColor(const QColor& color) {
    instance()->setValue("airportDisplay/dotColor", color);
}

double Settings::airportDotSize() {
    return instance()->value("airportDisplay/dotSize", 4).toDouble();
}

void Settings::setAirportDotSize(double value) {
    instance()->setValue("airportDisplay/dotSize", value);
}

QColor Settings::inactiveAirportFontColor() {
    return instance()->value("airportDisplay/inactiveFontColor", QColor::fromRgb(255, 255, 127, 100)).value<QColor>();
}
void Settings::setInactiveAirportFontColor(const QColor& color) {
    instance()->setValue("airportDisplay/inactiveFontColor", color);
}

QColor Settings::inactiveAirportDotColor() {
    return instance()->value("airportDisplay/inactiveDotColor", QColor::fromRgb(85, 170, 255, 50)).value<QColor>();
}
void Settings::setInactiveAirportDotColor(const QColor& color) {
    instance()->setValue("airportDisplay/inactiveDotColor", color);
}

double Settings::inactiveAirportDotSize() {
    return instance()->value("airportDisplay/inactiveDotSize", 2).toDouble();
}
void Settings::setInactiveAirportDotSize(double value) {
    instance()->setValue("airportDisplay/inactiveDotSize", value);
}

QFont Settings::inactiveAirportFont() {
    QFont defaultResult;
    defaultResult.setPointSize(8);
    QFont result = instance()->value("airportDisplay/inactiveFont", defaultResult).value<QFont>();
    result.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    return result;
}

void Settings::setInactiveAirportFont(const QFont& font) {
    instance()->setValue("airportDisplay/inactiveFont", font);
}

QColor Settings::twrBorderLineColor() {
    return instance()->value("airportDisplay/twrBorderLineColor", appBorderLineColor()).value<QColor>();
}

void Settings::setTwrBorderLineColor(const QColor& color) {
    instance()->setValue("airportDisplay/twrBorderLineColor", color);
}

double Settings::twrBorderLineWidth() {
    return instance()->value("airportDisplay/twrBorderLineStrength", appBorderLineWidth()).toDouble();
}

void Settings::setTwrBorderLineStrength(double value) {
    instance()->setValue("airportDisplay/twrBorderLineStrength", value);
}

QColor Settings::appBorderLineColor() {
    return instance()->value("airportDisplay/appBorderLineColor", QColor::fromRgb(42, 163, 214, 80)).value<QColor>();
}

void Settings::setAppBorderLineColor(const QColor& color) {
    instance()->setValue("airportDisplay/appBorderLineColor", color);
}

double Settings::appBorderLineWidth() {
    return instance()->value("airportDisplay/appBorderLineStrength", 1.).toDouble();
}

void Settings::setAppBorderLineStrength(double value) {
    instance()->setValue("airportDisplay/appBorderLineStrength", value);
}

QColor Settings::appCenterColor() {
    return instance()->value("airportDisplay/appCenterColor", QColor::fromRgb(0, 0, 0, 0)).value<QColor>();
}

void Settings::setAppCenterColor(const QColor& color) {
    instance()->setValue("airportDisplay/appCenterColor", color);
}

QColor Settings::appMarginColor() {
    return instance()->value("airportDisplay/appMarginColor", QColor::fromRgb(34, 85, 99, 88)).value<QColor>();
}

void Settings::setAppMarginColor(const QColor& color) {
    instance()->setValue("airportDisplay/appMarginColor", color);
}

QColor Settings::twrCenterColor() {
    return instance()->value("airportDisplay/twrCenterColor", QColor::fromRgb(34, 85, 99, 10)).value<QColor>();
}

void Settings::setTwrCenterColor(const QColor& color) {
    instance()->setValue("airportDisplay/twrCenterColor", color);
}

QColor Settings::twrMarginColor() {
    return instance()->value("airportDisplay/twrMarginColor", QColor::fromRgb(144, 8, 255, 30)).value<QColor>();
}

void Settings::setTwrMarginColor(const QColor& color) {
    instance()->setValue("airportDisplay/twrMarginColor", color);
}

QColor Settings::gndBorderLineColor() {
    return instance()->value("airportDisplay/gndBorderLineColor", QColor::fromRgb(255, 255, 127, 40)).value<QColor>();
}

void Settings::setGndBorderLineColor(const QColor& color) {
    instance()->setValue("airportDisplay/gndBorderLineColor", color);
}

double Settings::gndBorderLineWidth() {
    return instance()->value("airportDisplay/gndBorderLineStrength", 1.0).toDouble();
}

void Settings::setGndBorderLineStrength(double value) {
    instance()->setValue("airportDisplay/gndBorderLineStrength", value);
}

QColor Settings::gndFillColor() {
    return instance()->value("airportDisplay/gndFillColor", QColor::fromRgb(12, 30, 35, 186)).value<QColor>();
}

void Settings::setGndFillColor(const QColor& color) {
    instance()->setValue("airportDisplay/gndFillColor", color);
}


QColor Settings::delBorderLineColor() {
    return instance()->value("airportDisplay/delBorderLineColor", gndBorderLineColor()).value<QColor>();
}

void Settings::setDelBorderLineColor(const QColor& color) {
    instance()->setValue("airportDisplay/delBorderLineColor", color);
}

double Settings::delBorderLineWidth() {
    return instance()->value("airportDisplay/delBorderLineStrength", gndBorderLineWidth()).toDouble();
}

void Settings::setDelBorderLineStrength(double value) {
    instance()->setValue("airportDisplay/delBorderLineStrength", value);
}

QColor Settings::delFillColor() {
    return instance()->value("airportDisplay/delFillColor", gndFillColor()).value<QColor>();
}

void Settings::setDelFillColor(const QColor& color) {
    instance()->setValue("airportDisplay/delFillColor", color);
}


// Airport traffic
bool Settings::filterTraffic() {
    return instance()->value("airportTraffic/filterTraffic", true).toBool();
}

void Settings::setFilterTraffic(bool v) {
    instance()->setValue("airportTraffic/filterTraffic", v);
}

int Settings::filterDistance() {
    return instance()->value("airportTraffic/filterDistance", 5).toInt();
}

void Settings::setFilterDistance(int v) {
    instance()->setValue("airportTraffic/filterDistance", v);
}

double Settings::filterArriving() {
    return instance()->value("airportTraffic/filterArriving", 1.0).toDouble();
}

void Settings::setFilterArriving(double v) {
    instance()->setValue("airportTraffic/filterArriving", v);
}
// airport congestion
bool Settings::showAirportCongestion() {
    return instance()->value("airportTraffic/showCongestion", true).toBool();
}
void Settings::setAirportCongestion(bool value) {
    instance()->setValue("airportTraffic/showCongestion", value);
}

bool Settings::showAirportCongestionRing() {
    return instance()->value("airportTraffic/showCongestionRing", false).toBool();
}
void Settings::setAirportCongestionRing(bool value) {
    instance()->setValue("airportTraffic/showCongestionRing", value);
}

bool Settings::showAirportCongestionGlow() {
    return instance()->value("airportTraffic/showCongestionGlow", true).toBool();
}
void Settings::setAirportCongestionGlow(bool value) {
    instance()->setValue("airportTraffic/showCongestionGlow", value);
}

int Settings::airportCongestionMovementsMin() {
    return instance()->value("airportTraffic/congestionMovementsMin", 10).toInt();
}

void Settings::setAirportCongestionMovementsMin(int value) {
    instance()->setValue("airportTraffic/congestionMovementsMin", value);
}

int Settings::airportCongestionRadiusMin() {
    return instance()->value("airportTraffic/congestionRadiusMin", 20).toInt();
}

void Settings::setAirportCongestionRadiusMin(int value) {
    instance()->setValue("airportTraffic/congestionRadiusMin", value);
}

QColor Settings::airportCongestionColorMin() {
    return instance()->value("airportTraffic/congestionColorMin", QColor::fromRgb(20, 100, 170, 60)).value<QColor>();
}

void Settings::setAirportCongestionColorMin(const QColor& color) {
    instance()->setValue("airportTraffic/congestionColorMin", color);
}

double Settings::airportCongestionBorderLineStrengthMin() {
    return instance()->value("airportTraffic/borderLineStrengthMin", 2).toDouble();
}

void Settings::setAirportCongestionBorderLineStrengthMin(double value) {
    instance()->setValue("airportTraffic/borderLineStrengthMin", value);
}

int Settings::airportCongestionMovementsMax() {
    return instance()->value("airportTraffic/congestionMovementsMax", 60).toInt();
}

void Settings::setAirportCongestionMovementsMax(int value) {
    instance()->setValue("airportTraffic/congestionMovementsMax", value);
}

int Settings::airportCongestionRadiusMax() {
    return instance()->value("airportTraffic/congestionRadiusMax", 100).toInt();
}

void Settings::setAirportCongestionRadiusMax(int value) {
    instance()->setValue("airportTraffic/congestionRadiusMax", value);
}

QColor Settings::airportCongestionColorMax() {
    return instance()->value("airportTraffic/congestionColorMax", QColor::fromRgb(20, 100, 170, 160)).value<QColor>();
}

void Settings::setAirportCongestionColorMax(const QColor& color) {
    instance()->setValue("airportTraffic/congestionColorMax", color);
}

double Settings::airportCongestionBorderLineStrengthMax() {
    return instance()->value("airportTraffic/borderLineStrengthMax", 6).toDouble();
}

void Settings::setAirportCongestionBorderLineStrengthMax(double value) {
    instance()->setValue("airportTraffic/borderLineStrengthMax", value);
}


// pilot
QColor Settings::pilotFontColor() {
    return instance()->value("pilotDisplay/fontColor", QColor::fromRgb(255, 0, 127)).value<QColor>();
}

void Settings::setPilotFontColor(const QColor& color) {
    instance()->setValue("pilotDisplay/fontColor", color);
}

QFont Settings::pilotFont() {
    QFont defaultFont;
    defaultFont.setPointSize(8);
    return instance()->value("pilotDisplay/font", defaultFont).value<QFont>();
}

void Settings::setPilotFont(const QFont& font) {
    instance()->setValue("pilotDisplay/font", font);
}

QColor Settings::pilotFontSecondaryColor() {
    return instance()->value("pilotDisplay/fontSecondaryColor", QColor::fromRgb(170, 255, 255, 180)).value<QColor>();
}

void Settings::setPilotFontSecondaryColor(const QColor &color) {
    instance()->setValue("pilotDisplay/fontSecondaryColor", color);
}

QFont Settings::pilotFontSecondary() {
    QFont defaultFont;
    defaultFont.setPointSize(7);
    return instance()->value("pilotDisplay/fontSecondary", defaultFont).value<QFont>();
}

void Settings::setPilotFontSecondary(const QFont &font) {
    instance()->setValue("pilotDisplay/fontSecondary", font);
}

QString Settings::pilotPrimaryContent() {
    return instance()->value("pilotDisplay/primaryContent", "{login} {rulesIfNotIfr}").toString();
}

void Settings::setPilotPrimaryContent(const QString &value) {
    instance()->setValue("pilotDisplay/primaryContent", value);
}

QString Settings::pilotPrimaryContentHovered() {
    return instance()->value("pilotDisplay/primaryContentHovered", "{login} {rulesIfNotIfr}").toString();
}

void Settings::setPilotPrimaryContentHovered(const QString &value) {
    instance()->setValue("pilotDisplay/primaryContentHovered", value);
}

QString Settings::pilotSecondaryContent() {
    return instance()->value("pilotDisplay/secondaryContent", "{rating} {#livestream}📺{/livestream}").toString();
}

void Settings::setPilotSecondaryContent(const QString &value) {
    instance()->setValue("pilotDisplay/secondaryContent", value);
}

QString Settings::pilotSecondaryContentHovered() {
    return instance()->value("pilotDisplay/secondaryContentHovered", "{FL} {GS10} {type}\n{dest}\n{livestream}").toString();
}

void Settings::setPilotSecondaryContentHovered(const QString &value) {
    instance()->setValue("pilotDisplay/secondaryContentHovered", value);
}

QColor Settings::pilotDotColor() {
    return instance()->value("pilotDisplay/dotColor", QColor::fromRgb(255, 0, 127, 100)).value<QColor>();
}

void Settings::setPilotDotColor(const QColor& color) {
    instance()->setValue("pilotDisplay/dotColor", color);
}

double Settings::pilotDotSize() {
    return instance()->value("pilotDisplay/dotSize", 3).toDouble();
}

void Settings::setPilotDotSize(double value) {
    instance()->setValue("pilotDisplay/dotSize", value);
}

int Settings::timelineSeconds() {
    return instance()->value("pilotDisplay/timelineSeconds", 120).toInt();
}
void Settings::setTimelineSeconds(int value) {
    instance()->setValue("pilotDisplay/timelineSeconds", value);
}

QColor Settings::leaderLineColor() {
    return instance()->value("pilotDisplay/timeLineColor", QColor::fromRgb(255, 0, 127, 80)).value<QColor>();
}
void Settings::setLeaderLineColor(const QColor& color) {
    instance()->setValue("pilotDisplay/timeLineColor", color);
}

double Settings::timeLineStrength() {
    return instance()->value("pilotDisplay/timeLineStrength", 1.).toDouble();
}

void Settings::setTimeLineStrength(double value) {
    instance()->setValue("pilotDisplay/timeLineStrength", value);
}

// Waypoints
bool Settings::showUsedWaypoints() {
    return instance()->value("display/showUsedWaypoints", false).toBool();
}
void Settings::setShowUsedWaypoints(bool value) {
    instance()->setValue("display/showUsedWaypoints", value);
}

QColor Settings::waypointsFontColor() {
    return instance()->value("pilotDisplay/waypointsFontColor", QColor::fromRgbF(.7, .7, .7, .7)).value<QColor>();
}
void Settings::setWaypointsFontColor(const QColor& color) {
    instance()->setValue("pilotDisplay/waypointsFontColor", color);
}

QColor Settings::waypointsDotColor() {
    return instance()->value("pilotDisplay/waypointsDotColor", QColor::fromRgbF(.8, .8, 0., .7)).value<QColor>();
}
void Settings::setWaypointsDotColor(const QColor& color) {
    instance()->setValue("pilotDisplay/waypointsDotColor", color);
}

double Settings::waypointsDotSize() {
    return instance()->value("pilotDisplay/waypointsDotSize", 2.).toDouble();
}
void Settings::setWaypointsDotSize(double value) {
    instance()->setValue("pilotDisplay/waypointsDotSize", value);
}

QFont Settings::waypointsFont() {
    QFont defaultResult;
    defaultResult.setPointSize(8);
    QFont result = instance()->value("pilotDisplay/waypointsFont", defaultResult).value<QFont>();
    result.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    return result;
}
void Settings::setWaypointsFont(const QFont& font) {
    instance()->setValue("pilotDisplay/waypointsFont", font);
}

// routes
bool Settings::showRoutes() {
    return instance()->value("display/showRoutes", false).toBool();
}
void Settings::setShowRoutes(bool value) {
    instance()->setValue("display/showRoutes", value);
}

bool Settings::onlyShowImmediateRoutePart() {
    return instance()->value("display/onlyShowImmediateRoutePart", false).toBool();
}
void Settings::setOnlyShowImmediateRoutePart(bool value) {
    instance()->setValue("display/onlyShowImmediateRoutePart", value);
}


QColor Settings::depLineColor() {
    return instance()->value("pilotDisplay/depLineColor", QColor::fromRgb(170, 255, 127, 40)).value<QColor>();
}

void Settings::setDepLineColor(const QColor& color) {
    instance()->setValue("pilotDisplay/depLineColor", color);
}

double Settings::depLineStrength() {
    return instance()->value("pilotDisplay/depLineStrength", .8).toDouble();
}

void Settings::setDepLineStrength(double value) {
    instance()->setValue("pilotDisplay/depLineStrength", value);
}

bool Settings::depLineDashed() {
    return instance()->value("pilotDisplay/depLineDashed", true).toBool();
}

void Settings::setDepLineDashed(bool value) {
    instance()->setValue("pilotDisplay/depLineDashed", value);
}


int Settings::destImmediateDurationMin() {
    return instance()->value("pilotDisplay/destImmediateDurationMin", 30).toInt();
}
void Settings::setDestImmediateDurationMin(int value) {
    instance()->setValue("pilotDisplay/destImmediateDurationMin", value);
}

QColor Settings::destImmediateLineColor() {
    return instance()->value("pilotDisplay/destImmediateLineColor", QColor::fromRgb(255, 170, 0, 30)).value<QColor>();
}

void Settings::setDestImmediateLineColor(const QColor& color) {
    instance()->setValue("pilotDisplay/destImmediateLineColor", color);
}

double Settings::destImmediateLineStrength() {
    return instance()->value("pilotDisplay/destImmediateLineStrength", destLineStrength() * 3.).toDouble();
}

void Settings::setDestImmediateLineStrength(double value) {
    instance()->setValue("pilotDisplay/destImmediateLineStrength", value);
}


QColor Settings::destLineColor() {
    return instance()->value("pilotDisplay/destLineColor", QColor::fromRgb(255, 170, 0, 30)).value<QColor>();
}

void Settings::setDestLineColor(const QColor& color) {
    instance()->setValue("pilotDisplay/destLineColor", color);
}

void Settings::setDestLineDashed(bool value) {
    instance()->setValue("pilotDisplay/destLineDashed", value);
}

bool Settings::destLineDashed() {
    return instance()->value("pilotDisplay/destLineDashed", false).toBool();
}

double Settings::destLineStrength() {
    return instance()->value("pilotDisplay/destLineStrength", .8).toDouble();
}

void Settings::setDestLineStrength(double value) {
    instance()->setValue("pilotDisplay/destLineStrength", value);
}

void Settings::rememberedMapPosition(
    double* xrot,
    double*, // unused yrot
    double* zrot,
    double* zoom,
    int nr
) {
    *xrot = instance()->value(
        "defaultMapPosition/xrot" + QString("%1").arg(nr), -90.
    ).toDouble();
    // ignore yRot: no Earth tilting
    *zrot = instance()->value(
        "defaultMapPosition/zrot" + QString("%1").arg(nr), 0.
    ).toDouble();
    *zoom = instance()->value(
        "defaultMapPosition/zoom" + QString("%1").arg(nr), 2.
    ).toDouble();
}
void Settings::setRememberedMapPosition(
    double xrot,
    double yrot,
    double zrot,
    double zoom,
    int nr
) {
    instance()->setValue(
        "defaultMapPosition/xrot" + QString("%1").arg(nr), xrot
    );
    instance()->setValue(
        "defaultMapPosition/yrot" + QString("%1").arg(nr), yrot
    );
    instance()->setValue(
        "defaultMapPosition/zrot" + QString("%1").arg(nr), zrot
    );
    instance()->setValue(
        "defaultMapPosition/zoom" + QString("%1").arg(nr), zoom
    );
}
bool Settings::rememberMapPositionOnClose() {
    return instance()->value("defaultMapPosition/rememberMapPositionOnClose", true).toBool();
}
void Settings::setRememberMapPositionOnClose(bool val) {
    instance()->setValue("defaultMapPosition/rememberMapPositionOnClose", val);
}

// URL for data update checks; not in preferences
QString Settings::remoteDataRepository() {
    return instance()->value(
        "data/remoteDataRepository",
        "https://raw.githubusercontent.com/qutescoop/qutescoop/master/data/%1"
    )
        .toString();
}

QColor Settings::friendsHighlightColor() {
    return instance()->value("pilotDisplay/highlightColor", QColor::fromRgb(255, 255, 127, 180)).value<QColor>();
}
void Settings::setFriendsHighlightColor(QColor &color) {
    instance()->setValue("pilotDisplay/highlightColor", color);
}

QColor Settings::friendsPilotDotColor() {
    return instance()->value("friends/pilotDotColor", friendsHighlightColor()).value<QColor>();
}

QColor Settings::friendsAirportDotColor() {
    return instance()->value("friends/airportDotColor", friendsHighlightColor()).value<QColor>();

}

QColor Settings::friendsPilotLabelRectColor() {
    return instance()->value("friends/pilotLabelRectColor", friendsHighlightColor()).value<QColor>();

}

QColor Settings::friendsAirportLabelRectColor() {
    return instance()->value("friends/airportLabelRectColor", friendsHighlightColor()).value<QColor>();

}

QColor Settings::friendsSectorLabelRectColor() {
    return instance()->value("friends/sectorLabelRectColor", friendsHighlightColor()).value<QColor>();

}

double Settings::highlightLineWidth() {
    return instance()->value("pilotDisplay/highlightLineWidth", 2.5).toDouble();
}
void Settings::setHighlightLineWidth(double value) {
    instance()->setValue("pilotDisplay/highlightLineWidth", value);
}

bool Settings::animateFriendsHighlight() {
    return instance()->value("pilotDisplay/useHighlightAnimation", false).toBool();
}
void Settings::setAnimateFriendsHighlight(bool value) {
    instance()->setValue("pilotDisplay/useHighlightAnimation", value);
}

const QStringList Settings::friends() {
    return instance()->value("friends/friendList", QStringList()).toStringList();
}

void Settings::addFriend(const QString& friendId) {
    QStringList fl = friends();
    if (!fl.contains(friendId)) {
        fl.append(friendId);
    }
    instance()->setValue("friends/friendList", fl);
}

void Settings::removeFriend(const QString& friendId) {
    QStringList fl = friends();
    int i = fl.indexOf(friendId);
    if (i >= 0 && i < fl.size()) {
        fl.removeAt(i);
    }
    instance()->setValue("friends/friendList", fl);
}

const QString Settings::clientAlias(const QString &userId) {
    return instance()->value(
        QString("clients/alias_%1").arg(userId)
    ).toString();
}

void Settings::setClientAlias(const QString &userId, const QString &alias) {
    if (alias.isEmpty()) {
        instance()->remove(
            QString("clients/alias_%1").arg(userId)
        );
    } else {
        instance()->setValue(
            QString("clients/alias_%1").arg(userId),
            alias
        );
    }

    // should maybe use signals instead
    auto wnd = Window::instance(false);
    if (wnd != 0) {
        if (wnd->friendsList != 0) {
            wnd->friendsList->reset();
        }
        if (wnd->searchResult != 0) {
            wnd->searchResult->reset();
        }
        if (wnd->mapScreen != 0) {
            if (wnd->mapScreen->glWidget != 0) {
                wnd->mapScreen->glWidget->update();
            }
        }
    }

    if (PilotDetails::instance(false) != 0) {
        PilotDetails::instance()->refresh();
    }

    if (ControllerDetails::instance(false) != 0) {
        ControllerDetails::instance()->refresh();
    }

    if (AirportDetails::instance(false) != 0) {
        AirportDetails::instance()->refresh();
    }
}

bool Settings::resetOnNextStart() {
    return instance()->value("main/resetConfiguration", false).toBool();
}

void Settings::setResetOnNextStart(bool value) {
    instance()->setValue("main/resetConfiguration", value);
}


// zooming
int Settings::wheelMax() {
    return instance()->value("mouseWheel/wheelMax", 120).toInt();
}

void Settings::setWheelMax(int value) {
    instance()->setValue("mouseWheel/wheelMax", value);
}

double Settings::zoomFactor() {
    return instance()->value("zoom/zoomFactor", 1.0).toDouble();
}

void Settings::setZoomFactor(double value) {
    instance()->setValue("zoom/zoomFactor", value);
}

bool Settings::useSelectionRectangle() {
    return instance()->value("zoom/selectionRectangle", true).toBool();
}
void Settings::setUseSelctionRectangle(bool value) {
    instance()->setValue("zoom/selectionRectangle", value);
}


////////////////////////////////////////
// General

bool Settings::saveWhazzupData() {
    return instance()->value("download/saveWhazzupData", true).toBool();
}
void Settings::setSaveWhazzupData(bool value) {
    instance()->setValue("download/saveWhazzupData", value);
}


//////////////////////////////////////
// windowmanagment
/////////////////////////////////////
void Settings::setDialogPreferences(const QString &name, const DialogPreferences &dialogPreferences) {
    instance()->setValue("windowmanagment/" + name + "Size", dialogPreferences.size);
    instance()->setValue("windowmanagment/" + name + "Pos", dialogPreferences.pos);
    instance()->setValue("windowmanagment/" + name + "Geo", dialogPreferences.geometry);
}

Settings::DialogPreferences Settings::dialogPreferences(const QString &name) {
    return DialogPreferences {
        .size = instance()->value("windowmanagment/" + name + "Size").toSize(),
        .pos = instance()->value("windowmanagment/" + name + "Pos").toPoint(),
        .geometry = instance()->value("windowmanagment/" + name + "Geo").toByteArray()
    };
}

void Settings::saveSize(const QSize& size) {
    instance()->setValue("mainWindowState/size", size);
}

QSize Settings::savedSize() {
    return instance()->value("mainWindowState/size", QSize()).toSize();
}

void Settings::savePosition(const QPoint& pos) {
    instance()->setValue("mainWindowState/position", pos);
}

QPoint Settings::savedPosition() {
    return instance()->value("mainWindowState/position", QPoint()).toPoint();
}

void Settings::saveState(const QByteArray& state) {
    instance()->setValue("mainWindowState/state", state);
}

QByteArray Settings::savedState() {
    return instance()->value("mainWindowState/state", QByteArray()).toByteArray();
}

void Settings::saveMaximized(const bool val) {
    instance()->setValue("mainWindowState/maximized", val);
}

Qt::SortOrder Settings::airportDialogAtcSortOrder() {
    return Qt::SortOrder(instance()->value("airportDialog/atcSortOrder", Qt::AscendingOrder).toInt());
}

void Settings::setAirportDialogAtcSortOrder(Qt::SortOrder v) {
    instance()->setValue("airportDialog/atcSortOrder", int(v));
}

QMap<QString, QVariant> Settings::airportDialogAtcExpandedByType() {
    return instance()->value("airportDialog/atcExpandedByType", {}).toMap();
}

void Settings::setAirportDialogAtcExpandedByType(const QMap<QString, QVariant>& v) {
    instance()->setValue("airportDialog/atcExpandedByType", v);
}

void Settings::saveGeometry(const QByteArray& state) {
    instance()->setValue("mainWindowState/geometry", state);
}

QByteArray Settings::savedGeometry() {
    return instance()->value("mainWindowState/geometry", QByteArray()).toByteArray();
}

bool Settings::maximized() {
    return instance()->value("mainWindowState/maximized", true).toBool();
}
