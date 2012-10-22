/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Settings.h"

#include "Whazzup.h"
#include "Window.h"

QSettings *settings_instance = 0;
QSettings* Settings::getSettings() {
    if(settings_instance == 0) {
        settings_instance = new QSettings();
        qDebug() << "Expecting settings at" << settings_instance->fileName();
    }
    return settings_instance;
}

// .ini File Functions
void Settings::exportToFile(QString fileName) {
    QSettings* settings_file = new QSettings(fileName, QSettings::IniFormat);
    for (int i = 0; i < getSettings()->allKeys().length(); i++)
        settings_file->setValue(getSettings()->allKeys()[i], getSettings()->value(getSettings()->allKeys()[i]));
    delete settings_file;
}

void Settings::importFromFile(QString fileName) {
    QSettings* settings_file = new QSettings(fileName, QSettings::IniFormat);
    for (int i = 0; i < settings_file->allKeys().length(); i++)
        getSettings()->setValue(settings_file->allKeys()[i], settings_file->value(settings_file->allKeys()[i]));
    delete settings_file;
}


// returns NotOpen / ReadOnly / ReadWrite
QIODevice::OpenMode Settings::testDirectory(QString &dir) {
    QIODevice::OpenMode capabilities = QIODevice::NotOpen;
    if (QDir(dir).exists()) {
        QFile testFile(dir + "/test");
        if (testFile.open(QIODevice::ReadWrite)) {
            capabilities |= QIODevice::ReadWrite;
            testFile.close();
            testFile.remove();
        } else
            capabilities |= QIODevice::ReadOnly;
    }
    //qDebug() << "Settings::testDirectory()" << dir << capabilities;
    return capabilities;
}

// priority:
// 1) DataLocation ('dirs.first()') has subdirs and is writeable
    // on Mac: /Users/<user>/Library/Application Support/QuteScoop/QuteScoop
    // on Ubuntu: /home/<user>/.local/share/data/QuteScoop/QuteScoop
    // on WinXP 32: C:\Dokumente und Einstellungen\<user>\Lokale Einstellungen\Anwendungsdaten\QuteScoop\QuteScoop
    // on Win7 64: \Users\<user>\AppData\local\QuteScoop\QuteScoop
// 2) other location in 'dirs' has subdirs and is writeable
// 3) any location has subdirs
// 3a) if 'dirs.first()' is writeable: copy data there
// 4) if all that fails: fall back to executable-directory
void Settings::calculateApplicationDataDirectory() {
    QStringList dirs; // possible locations, 1st preferred
    dirs << QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    dirs << QCoreApplication::applicationDirPath();

    QStringList subdirs; // needed subDirs
    subdirs << "data" << "downloaded" << "screenshots" << "textures"; // 'texture' does not have to be writeable

    QList< QIODevice::OpenMode> dirCapabilities;
    for (int i = 0; i < dirs.size(); i++) {
        dirCapabilities << QIODevice::ReadWrite; // init, might be diminished by the AND-combine
        // looking for capabilities of the subdirs
        for (int j = 0; j < subdirs.size(); j++) {
            QString dir = QString("%1/%2").arg(dirs[i], subdirs[j]);
            dirCapabilities[i] &= testDirectory(dir); // AND-combine: returns lowest capability
        }
        //qDebug() << "Settings::calculateApplicationsDirectory():" << dirs[i] << "has capabilities:" << dirCapabilities[i];
        if (i == 0) { // preferred location writeable - no further need to look
            if (dirCapabilities[0].testFlag(QIODevice::ReadWrite)) {
                getSettings()->setValue("general/calculatedApplicationDataDirectory", dirs[0]);
                return;
            }
        } else { // another location is writeable - also OK.
            if (dirCapabilities[i].testFlag(QIODevice::ReadWrite)) {
                getSettings()->setValue("general/calculatedApplicationDataDirectory", dirs[i]);
                return;
            }
        }
    }

    // last ressort: looking for readable subdirs
    for (int i = 0; i < dirs.size(); i++) {
        if (dirCapabilities[i].testFlag(QIODevice::ReadOnly)) { // we found the data at least readonly
            QString warningStr(QString(
                    "The directories '%1' where found at '%2' but are readonly. This means that neither automatic sectorfile-download "
                    "nor saving logs, screenshots or downloaded Whazzups will work.\n"
                    "Preferrably, data should be at '%3' and this location should be writable.")
                                 .arg(subdirs.join("', '"))
                                 .arg(dirs[i])
                                 .arg(dirs.first()));
            qWarning() << warningStr;
            QMessageBox::warning(0, "Warning", warningStr);

            // data found in other location than the preferred one and
            // preferred location creatable or already existing: Copy data directories/files there?
            if ((i != 0) && (QDir(dirs.first()).exists() || QDir().mkpath(dirs.first()))) {
                QString questionStr(QString(
                        "The preferred data directory '%1' exists or could be created.\n"
                        "Do you want QuteScoop to install its data files there [recommended]?")
                                     .arg(dirs.first()));
                qDebug() << questionStr;
                if (QMessageBox::question(0, "Install data files?", questionStr, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
                    == QMessageBox::Yes) {
                    QString copyErrors;
                    for (int j = 0; j < subdirs.size(); j++) {
                        QString sourceDir = QString("%1/%2").arg(dirs[i], subdirs[j]);
                        QString destDir = QString("%1/%2").arg(dirs.first(), subdirs[j]);

                        // subdirectory exists or could be created
                        if (QDir(destDir).exists()
                            || QDir(dirs.first()).mkpath(subdirs[j])) {
                            QStringList fileNames = QDir(sourceDir)
                                                    .entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::NoSymLinks | QDir::Readable);
                            QMessageBox::information(0, "Copying", QString("Now copying files\n'%1'\n to directory '%2'")
                                                     .arg(fileNames.join("',\n'"), destDir));
                            foreach(const QString &fileName, fileNames) {
                                QString sourceFilePath = QString("%1/%2").arg(sourceDir, fileName);
                                QString destFilePath = QString("%1/%2").arg(destDir, fileName);
                                if (QFile::exists(destFilePath)) // remove if file is already existing
                                    if (!QFile::remove(destFilePath))
                                        QMessageBox::critical(0, "Error", QString(
                                                "Error removing existing file '%1'. Please consider removing it by hand.")
                                                              .arg(destFilePath));
                                if (QFile::copy(sourceFilePath, destFilePath))
                                    qDebug() << "copied" << sourceFilePath << "to" << destFilePath;
                                else {
                                    copyErrors.append(QString("Error copying file '%1' to '%2'\n").arg(sourceFilePath, destFilePath));
                                    qDebug() << "Error copying file" << sourceFilePath << "to" << destFilePath;
                                }
                            }
                        } else { // error creating subdirectory
                            copyErrors.append(QString("Error creating directory '%1'\n").arg(destDir));
                            qDebug() << QString("Error creating %1").arg(destDir);
                        }
                    }
                    if (copyErrors.isEmpty()) {
                        QMessageBox::information(0, "Success",
                                                 QString("Data files installed. QuteScoop will now use '%1' as data directory.")
                                                 .arg(dirs.first()));
                        qDebug() << "Datafiles installed to" << dirs.first();
                        getSettings()->setValue("general/calculatedApplicationDataDirectory", dirs.first());
                        return;
                    } else { // errors during the copy operations
                        QMessageBox::critical(0, "Error", QString("The following errors occurred during copy:\n %1\n"
                                                                  "When in doubt if important files where left out, "
                                                                  "delete the new data directory and let "
                                                                  "QuteScoop copy the files again.")
                                              .arg(copyErrors));
                    }
                }
            }
            getSettings()->setValue("general/calculatedApplicationDataDirectory", dirs[i]);
            return;
        }
    }

    QString criticalStr = QString("No complete data directory, neither read- nor writable, was found. QuteScoop "
                                  "might be behaving unexpectedly.\n"
                                  "Preferrably, '%1' should have the subdirectories '%2' and "
                                  "these locations should be writable.\n"
                                  "QuteScoop will look for the data files in the following locations, too:\n"
                                  "'%3'")
           .arg(dirs.first(), subdirs.join("', '"), QStringList(dirs.mid(1, -1)).join("',\n'"));
    QMessageBox::critical(0, "Critical", criticalStr);
    qCritical() << criticalStr;
    getSettings()->setValue("general/calculatedApplicationDataDirectory", QCoreApplication::applicationDirPath());
    return;
}

QString Settings::applicationDataDirectory(const QString &composeFilePath) {
    return QString("%1/%2")
            .arg(getSettings()->value("general/calculatedApplicationDataDirectory", QVariant()).toString())
            .arg(composeFilePath);
}

//

bool Settings::shootScreenshots() {
    return getSettings()->value("screenshots/shootScreenshots", false).toBool();
}
void Settings::setShootScreenshots(bool value) {
    getSettings()->setValue("screenshots/shootScreenshots", value);
}

int Settings::screenshotMethod() {
    return getSettings()->value("screenshots/method", 0).toInt();
}
void Settings::setScreenshotMethod(int value) {
    getSettings()->setValue("screenshots/method", value);
}

QString Settings::screenshotFormat() {
    return getSettings()->value("screenshots/format", "png").toString();
}
void Settings::setScreenshotFormat(const QString &value) {
    getSettings()->setValue("screenshots/format", value);
}

int Settings::downloadInterval() {
    return getSettings()->value("download/interval", 2).toInt();
}
void Settings::setDownloadInterval(int value) {
    getSettings()->setValue("download/interval", value);
}

bool Settings::downloadOnStartup() {
    return getSettings()->value("download/downloadOnStartup", true).toBool();
}

void Settings::setDownloadOnStartup(bool value) {
    getSettings()->setValue("download/downloadOnStartup", value);
}

bool Settings::downloadPeriodically() {
    return getSettings()->value("download/downloadPeriodically", true).toBool();
}

void Settings::setDownloadPeriodically(bool value) {
    getSettings()->setValue("download/downloadPeriodically", value);
}

bool Settings::downloadBookings() {
    return getSettings()->value("download/downloadBookings", downloadNetwork() == 1).toBool();
}

void Settings::setDownloadBookings(bool value) {
    getSettings()->setValue("download/downloadBookings", value);
}

QString Settings::bookingsLocation() {
    return getSettings()->value("download/bookingsLocation", "http://vatbook.euroutepro.com/servinfo.asp").toString();
}

void Settings::setBookingsLocation(const QString& value) {
    getSettings()->setValue("download/bookingsLocation", value);
}

bool Settings::bookingsPeriodically() {
    return getSettings()->value("download/bookingsPeriodically", true).toBool();
}

void Settings::setBookingsPeriodically(bool value) {
    getSettings()->setValue("download/bookingsPeriodically", value);
}

int Settings::bookingsInterval() {
    return getSettings()->value("download/bookingsInterval", 30).toInt();
}

void Settings::setBookingsInterval(int value) {
    getSettings()->setValue("download/bookingsInterval", value);
}

//
bool Settings::useSupFile() {
    return getSettings()->value("data/useSupFile", true).toBool();
}

void Settings::setUseSupFile(bool value) {
    getSettings()->setValue("data/useSupFile", value);
}

int Settings::downloadNetwork() {
    return getSettings()->value("download/network", 1).toInt();
}

void Settings::setDownloadNetwork(int i) {
    getSettings()->setValue("download/network", i);
}

QString Settings::downloadNetworkName() {
    switch(downloadNetwork()) {
    case 0: return "IVAO"; break;
    case 1: return "VATSIM"; break;
    case 2: return "User Network"; break;
    }
    return "Unknown";
}

QString Settings::userDownloadLocation() {
    return getSettings()->value("download/userLocation", "http://www.network.org/status.txt").toString();
}

void Settings::setUserDownloadLocation(const QString& location) {
    getSettings()->setValue("download/userLocation", location);
}

bool Settings::checkForUpdates() {
    return getSettings()->value("download/checkForUpdates", true).toBool();
}

void Settings::setCheckForUpdates(bool value) {
    getSettings()->setValue("download/checkForUpdates", value);
}

bool Settings::sendVersionInformation() {
    return getSettings()->value("download/sendVersionInfo", true).toBool();
}

void Settings::setSendVersionInformation(bool value) {
    getSettings()->setValue("download/sendVersionInfo", value);
}

QString Settings::updateVersionNumber() {
    return getSettings()->value("download/updateVersionNumber", "-1").toString();
}

void Settings::setUpdateVersionNumber(const QString& version) {
    getSettings()->setValue("download/updateVersionNumber", version);
}

QString Settings::statusLocation() {
    return getSettings()->value("download/statusLocation", "http://status.vatsim.net/").toString();
}

void Settings::setStatusLocation(const QString& location) {
    getSettings()->setValue("download/statusLocation", location);
    Whazzup::getInstance()->setStatusLocation(location);
}

bool Settings::useProxy() {
    return getSettings()->value("proxy/useProxy", false).toBool();
}

void Settings::setUseProxy(bool value) {
    getSettings()->setValue("proxy/useProxy", value);
}

QString Settings::proxyServer() {
    return getSettings()->value("proxy/server").toString();
}

void Settings::setProxyServer(const QString& server) {
    getSettings()->setValue("proxy/server", server);
}

int Settings::proxyPort() {
    return getSettings()->value("proxy/port", 8080).toInt();
}

void Settings::setProxyPort(int value) {
    getSettings()->setValue("proxy/port", value);
}

QString Settings::proxyUser() {
    return getSettings()->value("proxy/user").toString();
}

void Settings::setProxyUser(QString user) {
    getSettings()->setValue("proxy/user", user);
}

QString Settings::proxyPassword() {
    return getSettings()->value("proxy/password").toString();
}

void Settings::setProxyPassword(QString password) {
    getSettings()->setValue("proxy/password", password);
}

void Settings::applyProxySetting(QHttp *http) {
    if(!useProxy() || http == 0)
        return;

    QString user = Settings::proxyUser();
    QString pass = Settings::proxyPassword();
    if(user.isEmpty()) user = QString();
    if(pass.isEmpty()) pass = QString();

    if(!proxyServer().isEmpty())
        http->setProxy(proxyServer(), proxyPort(), user, pass);
}

QString Settings::navdataDirectory() {
    return getSettings()->value("database/path").toString();
}

void Settings::setNavdataDirectory(const QString& directory) {
    getSettings()->setValue("database/path", directory);
}

bool Settings::useNavdata() {
    return getSettings()->value("database/use", false).toBool();
}

void Settings::setUseNavdata(bool value) {
    getSettings()->setValue("database/use", value);
}

bool Settings::useESAirlines(){
    return getSettings()->value("database/useESAirlines", false).toBool();
}
void Settings::setUseESAirlnies(bool value){
    getSettings()->setValue("database/useESAirlines", value);
}

QString Settings::ESAirlinesDirectory(){
    return getSettings()->value("database/ESAirlinesDirectory").toString();
}
void Settings::setESAirlinesDirectory(QString directory){
    getSettings()->setValue("database/ESAirlinesDirectory", directory);
}

bool Settings::showAllWaypoints() {
    return getSettings()->value("database/showAllWaypoints", false).toBool();
}

void Settings::setShowAllWaypoints(bool value) {
    getSettings()->setValue("database/showAllWaypoints", value);
}

int Settings::metarDownloadInterval() {
    return getSettings()->value("display/metarInterval", 10).toInt();
}

void Settings::setMetarDownloadInterval(int minutes) {
    getSettings()->setValue("download/metarInterval", minutes);
}

//Display
bool Settings::showCTR(){
    return getSettings()->value("display/showCTR", true).toBool();
}
void Settings::setShowCTR(bool value){
    getSettings()->setValue("display/showCTR", value);
}

bool Settings::showAPP(){
    return getSettings()->value("display/showAPP", true).toBool();
}
void Settings::setShowAPP(bool value){
    return getSettings()->setValue("display/showAPP", value);
}

bool Settings::showTWR(){
    return getSettings()->value("display/showTWR", true).toBool();
}
void Settings::setShowTWR(bool value){
    getSettings()->setValue("display/showTWR", value);
}

bool Settings::showGND(){
    return getSettings()->value("display/showGND", true).toBool();
}
void Settings::setShowGND(bool value){
    getSettings()->setValue("display/showGND", value);
}

bool Settings::showAllSectors(){
    return getSettings()->value("display/showALLSectors", false).toBool();
}
void Settings::setShowAllSectors(bool value){
    getSettings()->setValue("display/showALLSectors", value);
}

bool Settings::showUpperWind(){
    return getSettings()->value("display/showUpperWind", false).toBool();
}
void Settings::setShowUpperWind(bool value){
    getSettings()->setValue("display/showUpperWind", value);
}

int Settings::upperWindAlt(){
    return getSettings()->value("display/upperWindAlt", 10000).toInt();
}
void Settings::setUpperWindAlt(int value){
    getSettings()->setValue("display/upperWindAlt", value);
}

QColor Settings::upperWindColor(){
    return getSettings()->value("display/upperWindColor", QColor::fromRgb(136, 255, 134)).value<QColor>();
}
void Settings::setUpperWindColor(const QColor &value){
    getSettings()->setValue("display/upperWindColor", value);
}

bool Settings::showRouteFix(){
    return getSettings()->value("display/showRouteFix", false).toBool();
}
void Settings::setShowRouteFix(bool value){
    getSettings()->setValue("display/showRouteFix", value);
}

bool Settings::showPilotsLabels(){
    return getSettings()->value("display/showPilotsLabels", true).toBool();
}
void Settings::setShowPilotsLabels(bool value){
    getSettings()->setValue("display/showPilotsLabels", value);
}

bool Settings::showInactiveAirports() {
    return getSettings()->value("display/showInactive", false).toBool(); // time-intensive function
}
void Settings::setShowInactiveAirports(const bool& value) {
    getSettings()->setValue("display/showInactive", value);
}

bool Settings::showClouds(){
    return getSettings()->value("display/showClouds", false).toBool();
}
void Settings::setShowClouds(const bool value){
    getSettings()->setValue("display/showClouds", value);
}

bool Settings::highlightFriends(){
    return getSettings()->value("display/highlightFriends", false).toBool();
}
void Settings::setHighlightFriends(bool value){
    getSettings()->setValue("display/highlightFriends", value);
}

// OpenGL

bool Settings::displaySmoothLines() {
    return getSettings()->value("gl/smoothLines", true).toBool();
}

void Settings::setDisplaySmoothLines(bool value) {
    getSettings()->setValue("gl/smoothLines", value);
}

bool Settings::glStippleLines() {
    return getSettings()->value("gl/stippleLines", true).toBool();
}

void Settings::setGlStippleLines(bool value) {
    getSettings()->setValue("gl/stippleLines", value);
}

bool Settings::displaySmoothDots() {
    return getSettings()->value("gl/smoothDots", true).toBool();
}

void Settings::setDisplaySmoothDots(bool value) {
    getSettings()->setValue("gl/smoothDots", value);
}

int Settings::maxLabels() {
    return getSettings()->value("gl/maxLabels", 150).toInt();
}

void Settings::setMaxLabels(int maxLabels) {
    getSettings()->setValue("gl/maxLabels", maxLabels);
}

bool Settings::glBlending() {
    return getSettings()->value("gl/blend", true).toBool();
}
void Settings::setGlBlending(bool value) {
    getSettings()->setValue("gl/blend", value);
}

int Settings::glCirclePointEach() {
    return getSettings()->value("gl/circlePointEach", 3).toInt();
}
void Settings::setGlCirclePointEach(int value) {
    getSettings()->setValue("gl/circlePointEach", value);
}

bool Settings::glLighting() {
    return getSettings()->value("gl/lighting", true).toBool();
}
void Settings::setEnableLighting(bool value) {
    getSettings()->setValue("gl/lighting", value);
}

int Settings::glLights() {
    return getSettings()->value("gl/lights", 6).toInt();
}
void Settings::setGlLights(int value) {
    getSettings()->setValue("gl/lights", value);
}

int Settings::glLightsSpread() {
    return getSettings()->value("gl/lightsSpread", 35).toInt();
}
void Settings::setGlLightsSpread(int value) {
    getSettings()->setValue("gl/lightsSpread", value);
}

bool Settings::glTextures() {
    return getSettings()->value("gl/earthTexture", true).toBool();
}
void Settings::setGlTextures(bool value) {
    getSettings()->setValue("gl/earthTexture", value);
}
QString Settings::glTextureEarth() {
    return getSettings()->value("gl/textureEarth", "2048px.png").toString();
}
void Settings::setGlTextureEarth(QString value) {
    getSettings()->setValue("gl/textureEarth", value);
}

QColor Settings::sunLightColor() {
    return getSettings()->value("gl/sunLightColor", QColor::fromRgb(255, 255, 255)).value<QColor>();
}

void Settings::setSunLightColor(const QColor& color) {
    getSettings()->setValue("gl/sunLightColor", color);
}

QColor Settings::specularColor() {
    return getSettings()->value("gl/sunSpecularColor", QColor::fromRgb(50, 22, 3)).value<QColor>();
}

void Settings::setSpecularColor(const QColor& color) {
    getSettings()->setValue("gl/sunSpecularColor", color);
}

double Settings::earthShininess() {
    return getSettings()->value("gl/earthShininess", 25).toDouble();
}

void Settings::setEarthShininess(double strength) {
    getSettings()->setValue("gl/earthShininess", strength);
}


// Stylesheet
QString Settings::stylesheet() {
    return getSettings()->value("display/stylesheet", QString()).toString();
}

void Settings::setStylesheet(const QString& value) {
    getSettings()->setValue("display/stylesheet", value);
}

// earthspace
int Settings::earthGridEach() {
    return getSettings()->value("earthSpace/earthGridEach", 30).toInt();
}
void Settings::setEarthGridEach(int value) {
    getSettings()->setValue("earthSpace/earthGridEach", value);
}

QColor Settings::backgroundColor() {
    return getSettings()->value("earthSpace/backgroundColor", QColor::fromRgbF(0, 0, 0)).value<QColor>();
}

void Settings::setBackgroundColor(const QColor& color) {
    getSettings()->setValue("earthSpace/backgroundColor", color);
}

QColor Settings::globeColor() {
    return getSettings()->value("earthSpace/globeColor", QColor::fromRgb(255, 255, 255)).value<QColor>();
}

void Settings::setGlobeColor(const QColor& color) {
    getSettings()->setValue("earthSpace/globeColor", color);
}

QColor Settings::gridLineColor() {
    return getSettings()->value("earthSpace/gridLineColor", QColor::fromRgb(100, 100, 100, 80)).value<QColor>();
}

void Settings::setGridLineColor(const QColor& color) {
    getSettings()->setValue("earthSpace/gridLineColor", color);
}

double Settings::gridLineStrength() {
    return getSettings()->value("earthSpace/gridLineStrength", 0.5).toDouble();
}

void Settings::setGridLineStrength(double strength) {
    getSettings()->setValue("earthSpace/gridLineStrength", strength);
}

QColor Settings::countryLineColor() {
    return getSettings()->value("earthSpace/countryLineColor", QColor::fromRgb(102, 85, 67, 150)).value<QColor>();
}

void Settings::setCountryLineColor(const QColor& color) {
    getSettings()->setValue("earthSpace/countryLineColor", color);
}

double Settings::countryLineStrength() {
    return getSettings()->value("earthSpace/countryLineStrength", 1.).toDouble();
}

void Settings::setCountryLineStrength(double strength) {
    getSettings()->setValue("earthSpace/countryLineStrength", strength);
}

QColor Settings::coastLineColor() {
    return getSettings()->value("earthSpace/coastLineColor", QColor::fromRgb(102, 85, 67, 200)).value<QColor>();
}

void Settings::setCoastLineColor(const QColor& color) {
    getSettings()->setValue("earthSpace/coastLineColor", color);
}

double Settings::coastLineStrength() {
    return getSettings()->value("earthSpace/coastLineStrength", 2.).toDouble();
}

void Settings::setCoastLineStrength(double strength) {
    getSettings()->setValue("earthSpace/coastLineStrength", strength);
}

// FIRs

QColor Settings::firBorderLineColor() {
    return getSettings()->value("firDisplay/borderLineColor", QColor::fromRgbF(0.0, 0.0, 1.0)).value<QColor>();
}

void Settings::setFirBorderLineColor(const QColor& color) {
    getSettings()->setValue("firDisplay/borderLineColor", color);
}

double Settings::firBorderLineStrength() {
    return getSettings()->value("firDisplay/borderLineStrength", 1.5).toDouble();
}

void Settings::setFirBorderLineStrength(double strength) {
    getSettings()->setValue("firDisplay/borderLineStrength", strength);
}

QColor Settings::firFontColor() {
    return getSettings()->value("firDisplay/fontColor", QColor::fromRgb(170, 255, 255)).value<QColor>();
}

void Settings::setFirFontColor(const QColor& color) {
    getSettings()->setValue("firDisplay/fontColor", color);
}

QColor Settings::firFillColor() {
    return getSettings()->value("firDisplay/fillColor", QColor::fromRgb(42, 163, 214, 100)).value<QColor>();
}

void Settings::setFirFillColor(const QColor& color) {
    getSettings()->setValue("firDisplay/fillColor", color);
}

QFont Settings::firFont() {
    QFont defaultFont;
    defaultFont.setBold(true);
    defaultFont.setPixelSize(11);
    QFont result = getSettings()->value("firDisplay/font", defaultFont).value<QFont>();
    result.setStyleHint( QFont::SansSerif, QFont::PreferAntialias );
    return result;
}

void Settings::setFirFont(const QFont& font) {
    getSettings()->setValue("firDisplay/font", font);
}

//airport
QColor Settings::airportFontColor() {
    return getSettings()->value("airportDisplay/fontColor", QColor::fromRgb(255, 255, 127)).value<QColor>();
}

void Settings::setAirportFontColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/fontColor", color);
}

QColor Settings::airportDotColor() {
    return getSettings()->value("airportDisplay/dotColor", QColor::fromRgb(85, 170, 255)).value<QColor>();
}

void Settings::setAirportDotColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/dotColor", color);
}

double Settings::airportDotSize() {
    return getSettings()->value("airportDisplay/dotSizer", 4).toDouble();
}

void Settings::setAirportDotSize(double value) {
    getSettings()->setValue("airportDisplay/dotSizer", value);
}

QFont Settings::airportFont() {
    QFont defaultResult;
    defaultResult.setPixelSize(9);
    QFont result = getSettings()->value("airportDisplay/font", defaultResult).value<QFont>();
    result.setStyleHint( QFont::SansSerif, QFont::PreferAntialias );
    return result;
}

void Settings::setAirportFont(const QFont& font) {
    getSettings()->setValue("airportDisplay/font", font);
}

QColor Settings::inactiveAirportFontColor() {
    return getSettings()->value("airportDisplay/inactiveFontColor", QColor::fromRgbF(0.4, 0.4, 0.4, 1)).value<QColor>();
}
void Settings::setInactiveAirportFontColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/inactiveFontColor", color);
}

QColor Settings::inactiveAirportDotColor() {
    return getSettings()->value("airportDisplay/inactiveDotColor", QColor::fromRgbF(0.5, 0.5, 0.5, 1)).value<QColor>();
}
void Settings::setInactiveAirportDotColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/inactiveDotColor", color);
}

double Settings::inactiveAirportDotSize() {
    return getSettings()->value("airportDisplay/inactiveDotSizer", 2).toDouble();
}
void Settings::setInactiveAirportDotSize(double value) {
    getSettings()->setValue("airportDisplay/inactiveDotSizer", value);
}

QFont Settings::inactiveAirportFont() {
    QFont defaultResult;
    defaultResult.setPixelSize(8);
    QFont result = getSettings()->value("airportDisplay/inactiveFont", defaultResult).value<QFont>();
    result.setStyleHint( QFont::SansSerif, QFont::PreferAntialias );
    return result;
}
void Settings::setInactiveAirportFont(const QFont& font) {
    getSettings()->setValue("airportDisplay/inactiveFont", font);
}



QColor Settings::appBorderLineColor() {
    return getSettings()->value("airportDisplay/appBorderLineColor", QColor::fromRgb(255, 255, 127)).value<QColor>();
}

void Settings::setAppBorderLineColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/appBorderLineColor", color);
}

double Settings::appBorderLineStrength() {
    return getSettings()->value("airportDisplay/appBorderLineStrength", 1.5).toDouble();
}

void Settings::setAppBorderLineStrength(double value) {
    getSettings()->setValue("airportDisplay/appBorderLineStrength", value);
}

QColor Settings::appCenterColor() {
    return getSettings()->value("airportDisplay/appCenterColor", QColor::fromRgbF(0.0, 0.0, 1.0, 0.0)).value<QColor>();
}

void Settings::setAppCenterColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/appCenterColor", color);
}

QColor Settings::appMarginColor() {
    return getSettings()->value("airportDisplay/appMarginColor", QColor::fromRgb(85, 170, 255)).value<QColor>();
}

void Settings::setAppMarginColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/appMarginColor", color);
}

QColor Settings::twrMarginColor() {
    return getSettings()->value("airportDisplay/twrMarginColor", QColor::fromRgb(102, 85, 67)).value<QColor>();
}

void Settings::setTwrMarginColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/twrMarginColor", color);
}

QColor Settings::twrCenterColor() {
    return getSettings()->value("airportDisplay/twrCenterColor", QColor::fromRgbF(0.8, 0.8, 0.0, 0.0)).value<QColor>();
}

void Settings::setTwrCenterColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/twrCenterColor", color);
}

QColor Settings::gndBorderLineColor() {
    return getSettings()->value("airportDisplay/gndBorderLineColor", QColor::fromRgb(179, 0, 179)).value<QColor>();
}

void Settings::setGndBorderLineColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/gndBorderLineColor", color);
}

double Settings::gndBorderLineStrength() {
    return getSettings()->value("airportDisplay/gndBorderLineStrength", 0.2).toDouble();
}

void Settings::setGndBorderLineStrength(double value) {
    getSettings()->setValue("airportDisplay/gndBorderLineStrength", value);
}

QColor Settings::gndFillColor() {
    return getSettings()->value("airportDisplay/gndFillColor", QColor::fromRgb(255, 255, 127)).value<QColor>();
}

void Settings::setGndFillColor(const QColor& color) {
    getSettings()->setValue("airportDisplay/gndFillColor", color);
}

// pilot
QColor Settings::pilotFontColor() {
    return getSettings()->value("pilotDisplay/fontColor", QColor::fromRgb(255, 0, 127)).value<QColor>();
}

void Settings::setPilotFontColor(const QColor& color) {
    getSettings()->setValue("pilotDisplay/fontColor", color);
}

QFont Settings::pilotFont() {
    QFont defaultFont;
    defaultFont.setPixelSize(9);
    return getSettings()->value("pilotDisplay/font", defaultFont).value<QFont>();
}

void Settings::setPilotFont(const QFont& font) {
    getSettings()->setValue("pilotDisplay/font", font);
}

QColor Settings::pilotDotColor() {
    return getSettings()->value("pilotDisplay/dotColor", QColor::fromRgb(255, 0, 127)).value<QColor>();
}

void Settings::setPilotDotColor(const QColor& color) {
    getSettings()->setValue("pilotDisplay/dotColor", color);
}

double Settings::pilotDotSize() {
    return getSettings()->value("pilotDisplay/dotSize", 3).toDouble();
}

void Settings::setPilotDotSize(double value) {
    getSettings()->setValue("pilotDisplay/dotSize", value);
}

int Settings::timelineSeconds() {
    return getSettings()->value("pilotDisplay/timelineSeconds", 120).toInt();
}
void Settings::setTimelineSeconds(int value) {
    getSettings()->setValue("pilotDisplay/timelineSeconds", value);
}

QColor Settings::timeLineColor() {
    return getSettings()->value("pilotDisplay/timeLineColor", QColor::fromRgb(255, 0, 127)).value<QColor>();
}
void Settings::setTimeLineColor(const QColor& color) {
    getSettings()->setValue("pilotDisplay/timeLineColor", color);
}

double Settings::timeLineStrength() {
    return getSettings()->value("pilotDisplay/timeLineStrength", 1.).toDouble();
}

void Settings::setTimeLineStrength(double value) {
    getSettings()->setValue("pilotDisplay/timeLineStrength", value);
}

bool Settings::showUsedWaypoints() {
    return getSettings()->value("display/showUsedWaypoints", false).toBool();
}
void Settings::setShowUsedWaypoints(bool value) {
    getSettings()->setValue("display/showUsedWaypoints", value);
}

QColor Settings::waypointsFontColor() {
    return getSettings()->value("pilotDisplay/waypointsFontColor", QColor::fromRgbF(.7, .7, .7, .7)).value<QColor>();
}
void Settings::setWaypointsFontColor(const QColor& color) {
    getSettings()->setValue("pilotDisplay/waypointsFontColor", color);
}

QColor Settings::waypointsDotColor() {
    return getSettings()->value("pilotDisplay/waypointsDotColor", QColor::fromRgbF(.8, .8, 0., .7)).value<QColor>();
}
void Settings::setWaypointsDotColor(const QColor& color) {
    getSettings()->setValue("pilotDisplay/waypointsDotColor", color);
}

double Settings::waypointsDotSize() {
    return getSettings()->value("pilotDisplay/waypointsDotSizer", 2.).toDouble();
}
void Settings::setWaypointsDotSize(double value) {
    getSettings()->setValue("pilotDisplay/waypointsDotSizer", value);
}

QFont Settings::waypointsFont() {
    QFont defaultResult;
    defaultResult.setPixelSize(8);
    QFont result = getSettings()->value("pilotDisplay/waypointsFont", defaultResult).value<QFont>();
    result.setStyleHint( QFont::SansSerif, QFont::PreferAntialias );
    return result;
}
void Settings::setWaypointsFont(const QFont& font) {
    getSettings()->setValue("pilotDisplay/waypointsFont", font);
}


QColor Settings::depLineColor() {
    return getSettings()->value("pilotDisplay/depLineColor", QColor::fromRgb(170, 255, 127, 150)).value<QColor>();
}

void Settings::setDepLineColor(const QColor& color) {
    getSettings()->setValue("pilotDisplay/depLineColor", color);
}

QColor Settings::destLineColor() {
    return getSettings()->value("pilotDisplay/destLineColor", QColor::fromRgb(255, 170, 0, 150)).value<QColor>();
}

void Settings::setDestLineColor(const QColor& color) {
    getSettings()->setValue("pilotDisplay/destLineColor", color);
}

void Settings::setDepLineDashed(bool value) {
    getSettings()->setValue("pilotDisplay/depLineDashed", value);
}

bool Settings::depLineDashed() {
    return getSettings()->value("pilotDisplay/depLineDashed", false).toBool();
}

void Settings::setDestLineDashed(bool value) {
    getSettings()->setValue("pilotDisplay/destLineDashed", value);
}

bool Settings::destLineDashed() {
    return getSettings()->value("pilotDisplay/destLineDashed", true).toBool();
}

double Settings::depLineStrength() {
    return getSettings()->value("pilotDisplay/depLineStrength", 1.5).toDouble();
}

void Settings::setDepLineStrength(double value) {
    getSettings()->setValue("pilotDisplay/depLineStrength", value);
}

double Settings::destLineStrength() {
    return getSettings()->value("pilotDisplay/destLineStrength", 1.5).toDouble();
}

void Settings::setDestLineStrength(double value) {
    getSettings()->setValue("pilotDisplay/destLineStrength", value);
}

void Settings::getRememberedMapPosition(double *xrot, double *yrot, double *zrot, double *zoom, int nr) {
    *xrot = getSettings()->value("defaultMapPosition/xrot" + QString("%1").arg(nr), *xrot).toDouble();
    // ignore yRot: no Earth tilting
    //*yrot = getSettings()->value("defaultMapPosition/yrot" + QString("%1").arg(nr), *yrot).toDouble();
    *zrot = getSettings()->value("defaultMapPosition/zrot" + QString("%1").arg(nr), *zrot).toDouble();
    *zoom = getSettings()->value("defaultMapPosition/zoom" + QString("%1").arg(nr), *zoom).toDouble();
}

void Settings::setRememberedMapPosition(double xrot, double yrot, double zrot, double zoom, int nr) {
    getSettings()->setValue("defaultMapPosition/xrot" + QString("%1").arg(nr), xrot);
    getSettings()->setValue("defaultMapPosition/yrot" + QString("%1").arg(nr), yrot);
    getSettings()->setValue("defaultMapPosition/zrot" + QString("%1").arg(nr), zrot);
    getSettings()->setValue("defaultMapPosition/zoom" + QString("%1").arg(nr), zoom);
}

void Settings::saveState(const QByteArray& state) {
    getSettings()->setValue("mainWindowState/state", state);
}

QByteArray Settings::getSavedState() {
    return getSettings()->value("mainWindowState/state", QByteArray()).toByteArray();
}

void Settings::saveMaximized(const bool val) {
    getSettings()->setValue("mainWindowState/maximized", val);
}

bool Settings::getMaximized() {
    return getSettings()->value("mainWindowState/maximized", true).toBool();
}

void Settings::saveGeometry(const QByteArray& state) {
    getSettings()->setValue("mainWindowState/geometry", state);
}

QByteArray Settings::getSavedGeometry() {
    return getSettings()->value("mainWindowState/geometry", QByteArray()).toByteArray();
}

QColor Settings::highlightColor(){
    return getSettings()->value("pilotDisplay/highlightColor", QColor::fromRgb(255, 0 , 0, 255)).value<QColor>();
}
void Settings::setHighlightColor(QColor &color){
    getSettings()->setValue("pilotDisplay/highlightColor", color);
}

double Settings::highlightLineWidth(){
    return getSettings()->value("pilotDisplay/highlightLineWidth" , 1.5).toDouble();
}
void Settings::setHighlightLineWidth(double value){
    getSettings()->setValue("pilotDisplay/highlightLineWidth", value);
}

bool Settings::useHighlightAnimation(){
    return getSettings()->value("pilotDisplay/useHighlightAnimation", false).toBool();
}
void Settings::setUseHighlightAnimation(bool value){
    getSettings()->setValue("pilotDisplay/useHighlightAnimation", value);
}

void Settings::saveSize(const QSize& size) {
    getSettings()->setValue("mainWindowState/size", size);
}

QSize Settings::getSavedSize() {
    return getSettings()->value("mainWindowState/size", QSize()).toSize();
}

void Settings::savePosition(const QPoint& pos) {
    getSettings()->setValue("mainWindowState/position", pos);
}

QPoint Settings::getSavedPosition() {
    return getSettings()->value("mainWindowState/position", QPoint()).toPoint();
}

QStringList Settings::friends() {
    return getSettings()->value("friends/friendList", QStringList()).toStringList();
}

void Settings::addFriend(const QString& friendId) {
    QStringList fl = friends();
    if(!fl.contains(friendId))
        fl.append(friendId);
    getSettings()->setValue("friends/friendList", fl);
}

void Settings::removeFriend(const QString& friendId) {
    QStringList fl = friends();
    int i = fl.indexOf(friendId);
    if(i >= 0 && i < fl.size())
        fl.removeAt(i);
    getSettings()->setValue("friends/friendList", fl);
}

bool Settings::resetOnNextStart() {
    return getSettings()->value("general/resetConfiguration", false).toBool();
}

void Settings::setResetOnNextStart(bool value) {
    getSettings()->setValue("general/resetConfiguration", value);
}

Settings::VoiceType Settings::voiceType() {
    return (VoiceType) getSettings()->value("voice/type", NONE).toInt();
}

void Settings::setVoiceType(Settings::VoiceType type) {
    getSettings()->setValue("voice/type", (int)type);
}

QString Settings::voiceCallsign() {
    return getSettings()->value("voice/callsign").toString();
}

void Settings::setVoiceCallsign(const QString& value) {
    getSettings()->setValue("voice/callsign", value);
}

QString Settings::voiceUser() {
    return getSettings()->value("voice/user").toString();
}

void Settings::setVoiceUser(const QString& value) {
    getSettings()->setValue("voice/user", value);
}

QString Settings::voicePassword() {
    return getSettings()->value("voice/password").toString();
}

void Settings::setVoicePassword(const QString& value) {
    getSettings()->setValue("voice/password", value);
}

// Airport traffic
bool Settings::filterTraffic() {
    return getSettings()->value("airportTraffic/filterTraffic", true).toBool();
}

void Settings::setFilterTraffic(bool v) {
    getSettings()->setValue("airportTraffic/filterTraffic", v);
}

int Settings::filterDistance() {
    return getSettings()->value("airportTraffic/filterDistance", 5).toInt();
}

void Settings::setFilterDistance(int v) {
    getSettings()->setValue("airportTraffic/filterDistance", v);
}

double Settings::filterArriving() {
    return getSettings()->value("airportTraffic/filterArriving", 1.0).toDouble();
}

void Settings::setFilterArriving(double v) {
    getSettings()->setValue("airportTraffic/filterArriving", v);
}
// airport congestion
bool Settings::showAirportCongestion() {
    return getSettings()->value("airportTraffic/showCongestion", true).toBool();
}
void Settings::setAirportCongestion(bool value) {
    getSettings()->setValue("airportTraffic/showCongestion", value);
}

int Settings::airportCongestionMinimum() {
    return getSettings()->value("airportTraffic/minimumMovements", 8).toInt();
}

void Settings::setAirportCongestionMinimum(int value) {
    getSettings()->setValue("airportTraffic/minimumMovements", value);
}

QColor Settings::airportCongestionBorderLineColor() {
    return getSettings()->value("airportTraffic/borderLineColor", QColor::fromRgb(255, 0, 127, 150)).value<QColor>();
}

void Settings::setAirportCongestionBorderLineColor(const QColor& color) {
    getSettings()->setValue("airportTraffic/borderLineColor", color);
}

double Settings::airportCongestionBorderLineStrength() {
    return getSettings()->value("airportTraffic/borderLineStrength", 2).toDouble();
}

void Settings::setAirportCongestionBorderLineStrength(double value) {
    getSettings()->setValue("airportTraffic/borderLineStrength", value);
}

// zooming
int Settings::wheelMax() {
    return getSettings()->value("mouseWheel/wheelMax", 120).toInt();
}

void Settings::setWheelMax(int value) {
    getSettings()->setValue("mouseWheel/wheelMax", value);
}

double Settings::zoomFactor() {
    return getSettings()->value("zoom/zoomFactor", 1.0).toDouble();
}

void Settings::setZoomFactor(double value) {
    getSettings()->setValue("zoom/zoomFactor", value);
}

bool Settings::useSelectionRectangle() {
    return getSettings()->value("zoom/selectionRectangle", true).toBool();
}
void Settings::setUseSelctionRectangle(bool value) {
    getSettings()->setValue("zoom/selectionRectangle", value);
}


////////////////////////////////////////
// General

bool Settings::saveWhazzupData() {
    return getSettings()->value("general/saveWhazzupData", false).toBool();
}
void Settings::setSaveWhazzupData(bool value) {
    getSettings()->setValue("general/saveWhazzupData" , value);
}

bool Settings::downloadClouds(){
    return getSettings()->value("general/downloadClouds", false).toBool();
}

void Settings::setDownloadClouds(const bool value){
    getSettings()->setValue("general/downloadClouds", value);
}

bool Settings::useHightResClouds(){
    return getSettings()->value("general/useHightResClouds", false).toBool();
}

void Settings::setUseHightResClouds(const bool value){
    getSettings()->setValue("general/useHightResClouds", value);
}


//////////////////////////////////////
// windowmanagment
/////////////////////////////////////

QSize Settings::preferencesDialogSize(){
    return getSettings()->value("windowmanagment/preferncesSize", QSize()).toSize();
}

void Settings::setPreferencesDialogSize(const QSize &value){
    getSettings()->setValue("windowmanagment/preferncesSize", value);
}

QPoint Settings::preferencesDialogPos(){
    return getSettings()->value("windowmanagment/preferncesPos", QPoint()).toPoint();
}

void Settings::setPreferencesDialogPos(const QPoint &value){
    getSettings()->setValue("windowmanagment/preferncesPos", value);
}

QByteArray Settings::preferencesDialogGeometry(){
    return getSettings()->value("windowmanagment/preferncesGeo", QByteArray()).toByteArray();
}

void Settings::setPreferencesDialogGeometry(const QByteArray &value){
    getSettings()->setValue("windowmanagment/preferncesGeo", value);
}


QSize Settings::airportDetailsSize(){
    return getSettings()->value("windowmanagment/airportDetailsSize", QSize()).toSize();
}

void Settings::setAirportDetailsSize(const QSize &value){
    getSettings()->setValue("windowmanagment/airportDetailsSize", value);
}

QPoint Settings::airportDetailsPos(){
    return getSettings()->value("windowmanagment/airportDetailsPos", QPoint()).toPoint();
}

void Settings::setAirportDetailsPos(const QPoint &value){
    getSettings()->setValue("windowmanagment/airportDetailsPos", value);
}

QByteArray Settings::airportDetailsGeometry(){
    return getSettings()->value("windowmanagment/airportDetailsGeo", QByteArray()).toByteArray();
}

void Settings::setAirportDetailsGeometry(const QByteArray &value){
    getSettings()->setValue("windowmanagment/airportDetailsGeo", value);
}


QSize Settings::bookAtcDialogSize(){
    return getSettings()->value("windowmanagment/bookAtcDialogSize", QSize()).toSize();
}

void Settings::setBookAtcDialogSize(const QSize &value){
    getSettings()->setValue("windowmanagment/bookAtcDialogSize", value);
}

QPoint Settings::bookAtcDialogPos(){
    return getSettings()->value("windowmanagment/bookAtcDialogPos", QPoint()).toPoint();
}

void Settings::setBookAtcDialogPos(const QPoint &value){
    getSettings()->setValue("windowmanagment/bookAtcDialogPos", value);
}

QByteArray Settings::bookAtcDialogGeometry(){
    return getSettings()->value("windowmanagment/bookAtcDialogGeo", QByteArray()).toByteArray();
}

void Settings::setBookAtcDialogGeometry(const QByteArray &value){
    getSettings()->setValue("windowmanagment/bookAtcDialogGeo", value);
}


QSize Settings::controllerDetailsSize(){
    return getSettings()->value("windowmanagment/controllerDetailsSize", QSize()).toSize();
}

void Settings::setControllerDetailsSize(const QSize &value){
    getSettings()->setValue("windowmanagment/controllerDetailsSize", value);
}

QPoint Settings::controllerDetailsPos(){
    return getSettings()->value("windowmanagment/controllerDetailsPos", QPoint()).toPoint();
}

void Settings::setControllerDetailsPos(const QPoint &value){
    getSettings()->setValue("windowmanagment/controllerDetailsPos", value);
}

QByteArray Settings::controllerDetailsGeometry(){
    return getSettings()->value("windowmanagment/controllerDetailsGeo", QByteArray()).toByteArray();
}

void Settings::setControllerDetailsGeometry(const QByteArray &value){
    getSettings()->setValue("windowmanagment/controllerDetailsGeo", value);
}


QSize Settings::listClientsDialogSize(){
    return getSettings()->value("windowmanagment/listClientsDialogSize", QSize()).toSize();
}

void Settings::setListClientsDialogSize(const QSize &value){
    getSettings()->setValue("windowmanagment/listClientsDialogSize", value);
}

QPoint Settings::listClientsDialogPos(){
    return getSettings()->value("windowmanagment/listClientsDialogPos", QPoint()).toPoint();
}

void Settings::setListClientsDialogPos(const QPoint &value){
    getSettings()->setValue("windowmanagment/listClientsDialogPos", value);
}

QByteArray Settings::listClientsDialogGeometry(){
    return getSettings()->value("windowmanagment/listClientsDialogGeo", QByteArray()).toByteArray();
}

void Settings::setListClientsDialogGeometry(const QByteArray &value){
    getSettings()->setValue("windowmanagment/listClientsDialogGeo", value);
}


QSize Settings::pilotDetailsSize(){
    return getSettings()->value("windowmanagment/pilotDetailsSize", QSize()).toSize();
}

void Settings::setPilotDetailsSize(const QSize &value){
    getSettings()->setValue("windowmanagment/pilotDetailsSize", value);
}

QPoint Settings::pilotDetailsPos(){
    return getSettings()->value("windowmanagment/pilotDetailsPos", QPoint()).toPoint();
}

void Settings::setPilotDetailsPos(const QPoint &value){
    getSettings()->setValue("windowmanagment/pilotDetailsPos", value);
}

QByteArray Settings::pilotDetailsGeometry(){
    return getSettings()->value("windowmanagment/pilotDetailsGeo", QByteArray()).toByteArray();
}

void Settings::setPilotDetailsGeometry(const QByteArray &value){
    getSettings()->setValue("windowmanagment/pilotDetailsGeo", value);
}

QSize Settings::planFlightDialogSize(){
    return getSettings()->value("windowmanagment/planFlightDialogSize").toSize();
}

void Settings::setPlanFlightDialogSize(const QSize &value){
    getSettings()->setValue("windowmanagment/planFlightDialogSize", value);
}

QPoint Settings::planFlightDialogPos(){
    return getSettings()->value("windowmanagment/planFlightDialogPos").toPoint();
}

void Settings::setPlanFlightDialogPos(const QPoint &value){
    getSettings()->setValue("windowmanagment/planFlightDialogPos", value);
}

QByteArray Settings::planFlightDialogGeometry(){
    return getSettings()->value("windowmanagment/planFlightDialogGeo").toByteArray();
}

void Settings::setPlanFlightDialogGeometry(const QByteArray &value){
    getSettings()->setValue("windowmanagment/planFlightDialogGeo", value);
}

void Settings::setSimpleLabels(bool value) {
    getSettings()->setValue("gl/simpleLabels", value);
}

bool Settings::simpleLabels() {
    return getSettings()->value("gl/simpleLabels", false).toBool();
}
