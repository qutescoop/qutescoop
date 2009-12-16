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

#include "Settings.h"
#include "Whazzup.h"

#include <QSettings>
#include <QDebug>

QSettings *settings_instance = 0;

QSettings* Settings::getSettings() {
	if(settings_instance == 0)
		settings_instance = new QSettings();
	return settings_instance;
}

int Settings::downloadInterval() {
	return getSettings()->value("download/interval", 5).toInt();
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

bool Settings::useSupFile() {
	return getSettings()->value("data/useSupFile", false).toBool();
}

void Settings::setUseSupFile(bool value) {
	getSettings()->setValue("data/useSupFile", value);
}

int Settings::downloadNetwork() {
	return getSettings()->value("download/network", 0).toInt();
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
	return getSettings()->value("download/updateVersionNumber").toString();
}

void Settings::setUpdateVersionNumber(const QString& version) {
	getSettings()->setValue("download/updateVersionNumber", version);
}

QString Settings::statusLocation() {
	return getSettings()->value("download/statusLocation", "http://www.ivao.aero/whazzup/status.txt").toString();
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

int Settings::timelineSeconds() {
	return getSettings()->value("display/timelineSeconds", 120).toInt();
}

void Settings::setTimelineSeconds(int value) {
	getSettings()->setValue("display/timelineSeconds", value);
}

bool Settings::displaySmoothLines() {
	return getSettings()->value("display/smoothLines", true).toBool();
}

void Settings::setDisplaySmoothLines(bool value) {
	getSettings()->setValue("display/smoothLines", value);
}

bool Settings::displaySmoothDots() {
	return getSettings()->value("display/smoothDots", true).toBool();
}

void Settings::setDisplaySmoothDots(bool value) {
	getSettings()->setValue("display/smoothDots", value);
}

int Settings::maxLabels() {
	return getSettings()->value("display/maxLabels", 80).toInt();
}

void Settings::setMaxLabels(int maxLabels) {
	getSettings()->setValue("display/maxLabels", maxLabels);
}

QString Settings::navdataDirectory() {
	return getSettings()->value("database/path").toString();
}

void Settings::setNavdataDirectory(const QString& directory) {
	getSettings()->setValue("database/path", directory);
}

bool Settings::useNavdata() {
	return getSettings()->value("database/use", true).toBool();
}

void Settings::setUseNavdata(bool value) {
	getSettings()->setValue("database/use", value);
}

int Settings::metarDownloadInterval() {
	return getSettings()->value("display/metarInterval", 10).toInt();
}

void Settings::setMetarDownloadInterval(int minutes) {
	getSettings()->setValue("download/metarInterval", minutes);
}

QColor Settings::backgroundColor() {
	// the default value is the "Trolltech Purple"
	return getSettings()->value("earthSpace/backgroundColor", QColor::fromCmykF(0.39, 0.39, 0.0, 0.0)).value<QColor>();
}

void Settings::setBackgroundColor(const QColor& color) {
	getSettings()->setValue("earthSpace/backgroundColor", color);
}

QColor Settings::globeColor() {
	return getSettings()->value("earthSpace/globeColor", QColor::fromRgbF(0, 0, 0.2, 1)).value<QColor>();
}

void Settings::setGlobeColor(const QColor& color) {
	getSettings()->setValue("earthSpace/globeColor", color);
}

QColor Settings::gridLineColor() {
	return getSettings()->value("earthSpace/gridLineColor", QColor::fromRgbF(0.2, 0.2, 0.2, 1)).value<QColor>();
}

void Settings::setGridLineColor(const QColor& color) {
	getSettings()->setValue("earthSpace/gridLineColor", color);
}

double Settings::gridLineStrength() {
	return getSettings()->value("earthSpace/gridLineStrength", 1).toDouble();
}

void Settings::setGridLineStrength(double strength) {
	getSettings()->setValue("earthSpace/gridLineStrength", strength);
}

QColor Settings::countryLineColor() {
	return getSettings()->value("earthSpace/countryLineColor", QColor::fromRgbF(0.4, 0.4, 0.4, 1)).value<QColor>();
}

void Settings::setCountryLineColor(const QColor& color) {
	getSettings()->setValue("earthSpace/countryLineColor", color);
}

double Settings::countryLineStrength() {
	return getSettings()->value("earthSpace/countryLineStrength", 1).toDouble();
}

void Settings::setCountryLineStrength(double strength) {
	getSettings()->setValue("earthSpace/countryLineStrength", strength);
}

QColor Settings::coastLineColor() {
	return getSettings()->value("earthSpace/coastLineColor", QColor::fromRgbF(0.3, 0.3, 0.3, 1)).value<QColor>();
}

void Settings::setCoastLineColor(const QColor& color) {
	getSettings()->setValue("earthSpace/coastLineColor", color);
}

double Settings::coastLineStrength() {
	return getSettings()->value("earthSpace/coastLineStrength", 2).toDouble();
}

void Settings::setCoastLineStrength(double strength) {
	getSettings()->setValue("earthSpace/coastLineStrength", strength);
}

void Settings::deleteEarthSpaceSettings() {
	QSettings settings;
	settings.beginGroup("earthSpace");
	settings.remove("");
	settings.endGroup();
}

void Settings::deleteFirSettings() {
	QSettings settings;
	settings.beginGroup("firDisplay");
	settings.remove("");
	settings.endGroup();
}

QColor Settings::firBorderLineColor() {
	return getSettings()->value("firDisplay/borderLineColor", QColor::fromRgbF(1.0, 0.0, 0.0, 1)).value<QColor>();
}

void Settings::setFirBorderLineColor(const QColor& color) {
	getSettings()->setValue("firDisplay/borderLineColor", color);
}

double Settings::firBorderLineStrength() {
	return getSettings()->value("firDisplay/borderLineStrength", 2).toDouble();
}

void Settings::setFirBorderLineStrength(double strength) {
	getSettings()->setValue("firDisplay/borderLineStrength", strength);
}

QColor Settings::firFontColor() {
	return getSettings()->value("firDisplay/fontColor", QColor::fromRgbF(1.0, 0.0, 0.0, 1)).value<QColor>();
}

void Settings::setFirFontColor(const QColor& color) {
	getSettings()->setValue("firDisplay/fontColor", color);
}

QColor Settings::firFillColor() {
	return getSettings()->value("firDisplay/fillColor", QColor::fromRgbF(0.5, 0.0, 0.0, 1)).value<QColor>();
}

void Settings::setFirFillColor(const QColor& color) {
	getSettings()->setValue("firDisplay/fillColor", color);
}

QFont Settings::firFont() {
	QFont defaultFont;
	defaultFont.setBold(true);
	defaultFont.setPixelSize(12);
	QFont result = getSettings()->value("firDisplay/font", defaultFont).value<QFont>();
	result.setStyleHint( QFont::SansSerif, QFont::PreferAntialias );
	return result;
}

void Settings::setFirFont(const QFont& font) {
	getSettings()->setValue("firDisplay/font", font);
}

void Settings::deleteAirportSettings() {
	QSettings settings;
	settings.beginGroup("airportDisplay");
	settings.remove("");
	settings.endGroup();
}

QColor Settings::airportFontColor() {
	return getSettings()->value("airportDisplay/fontColor", QColor::fromRgbF(0.1, 1.0, 0.1, 1)).value<QColor>();
}

void Settings::setAirportFontColor(const QColor& color) {
	getSettings()->setValue("airportDisplay/fontColor", color);
}

QColor Settings::airportDotColor() {
	return getSettings()->value("airportDisplay/dotColor", QColor::fromRgbF(0.6, 0.6, 0.6, 1)).value<QColor>();
}

void Settings::setAirportDotColor(const QColor& color) {
	getSettings()->setValue("airportDisplay/dotColor", color);
}

double Settings::airportDotSize() {
	return getSettings()->value("airportDisplay/dotSizer", 0.8).toDouble();
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

QColor Settings::appBorderLineColor() {
	return getSettings()->value("airportDisplay/appBorderLineColor", QColor::fromRgbF(0.0, 0.0, 1.0, 1)).value<QColor>();
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
	return getSettings()->value("airportDisplay/appCenterColor", QColor::fromRgbF(0.0, 0.0, 1.0, 1)).value<QColor>();
}

void Settings::setAppCenterColor(const QColor& color) {
	getSettings()->setValue("airportDisplay/appCenterColor", color);
}

QColor Settings::appMarginColor() {
	return getSettings()->value("airportDisplay/appMarginColor", QColor::fromRgbF(0.0, 0.0, 0.8, 0.6)).value<QColor>();
}

void Settings::setAppMarginColor(const QColor& color) {
	getSettings()->setValue("airportDisplay/appMarginColor", color);
}

QColor Settings::twrMarginColor() {
	return getSettings()->value("airportDisplay/twrMarginColor", QColor::fromRgbF(0.6, 0.6, 0.0, 0.6)).value<QColor>();
}

void Settings::setTwrMarginColor(const QColor& color) {
	getSettings()->setValue("airportDisplay/twrMarginColor", color);
}

QColor Settings::twrCenterColor() {
	return getSettings()->value("airportDisplay/twrCenterColor", QColor::fromRgbF(0.8, 0.8, 0.0, 1.0)).value<QColor>();
}

void Settings::setTwrCenterColor(const QColor& color) {
	getSettings()->setValue("airportDisplay/twrCenterColor", color);
}

QColor Settings::gndBorderLineColor() {
	return getSettings()->value("airportDisplay/gndBorderLineColor", QColor::fromRgbF(1.0, 0.0, 1.0, 1.0)).value<QColor>();
}

void Settings::setGndBorderLineColor(const QColor& color) {
	getSettings()->setValue("airportDisplay/gndBorderLineColor", color);
}

double Settings::gndBorderLineStrength() {
	return getSettings()->value("airportDisplay/gndBorderLineStrength", 1.2).toDouble();
}

void Settings::setGndBorderLineStrength(double value) {
	getSettings()->setValue("airportDisplay/gndBorderLineStrength", value);
}

QColor Settings::gndFillColor() {
	return getSettings()->value("airportDisplay/gndFillColor", QColor::fromRgbF(0.7, 0.0, 0.7, 0.8)).value<QColor>();
}

void Settings::setGndFillColor(const QColor& color) {
	getSettings()->setValue("airportDisplay/gndFillColor", color);
}

void Settings::deleteAircraftSettings() {
	QSettings settings;
	settings.beginGroup("pilotDisplay");
	settings.remove("");
	settings.endGroup();
}

QColor Settings::pilotFontColor() {
	return getSettings()->value("pilotDisplay/fontColor", QColor::fromRgbF(1, 1, 1, 1)).value<QColor>();
}

void Settings::setPilotFontColor(const QColor& color) {
	getSettings()->setValue("pilotDisplay/fontColor", color);
}

QFont Settings::pilotFont() {
	QFont defaultFont;
	defaultFont.setPixelSize(10);
	return getSettings()->value("pilotDisplay/font", defaultFont).value<QFont>();
}

void Settings::setPilotFont(const QFont& font) {
	getSettings()->setValue("pilotDisplay/font", font);
}

QColor Settings::pilotDotColor() {
	return getSettings()->value("pilotDisplay/dotColor", QColor::fromRgbF(1, 1, 1, 1)).value<QColor>();
}

void Settings::setPilotDotColor(const QColor& color) {
	getSettings()->setValue("pilotDisplay/dotColor", color);
}

double Settings::pilotDotSize() {
	return getSettings()->value("pilotDisplay/dotSize", 2.5).toDouble();
}

void Settings::setPilotDotSize(double value) {
	getSettings()->setValue("pilotDisplay/dotSize", value);
}

QColor Settings::timeLineColor() {
	return getSettings()->value("pilotDisplay/timeLineColor", QColor::fromRgbF(0.8, 0.8, 0.8, 0.8)).value<QColor>();
}

void Settings::setTimeLineColor(const QColor& color) {
	getSettings()->setValue("pilotDisplay/timeLineColor", color);
}

QColor Settings::trackLineColor() {
	return getSettings()->value("pilotDisplay/trackLineColor", QColor::fromRgbF(0.8, 0.8, 0.8, 0.6)).value<QColor>();
}

void Settings::setTrackLineColor(const QColor& color) {
	getSettings()->setValue("pilotDisplay/trackLineColor", color);
}

QColor Settings::planLineColor() {
	return getSettings()->value("pilotDisplay/planLineColor", QColor::fromRgbF(0.0, 0.8, 0.0, 0.8)).value<QColor>();
}

void Settings::setPlanLineColor(const QColor& color) {
	getSettings()->setValue("pilotDisplay/planLineColor", color);
}

void Settings::setDashedTrackInFront(bool value) {
	getSettings()->setValue("pilotDisplay/dashedTrackInFront", value);
}

bool Settings::dashedTrackInFront() {
	return getSettings()->value("pilotDisplay/dashedTrackInFront", true).toBool();
}

double Settings::timeLineStrength() {
	return getSettings()->value("pilotDisplay/timeLineStrength", 0.8).toDouble();
}

void Settings::setTimeLineStrength(double value) {
	getSettings()->setValue("pilotDisplay/timeLineStrength", value);
}

double Settings::trackLineStrength() {
	return getSettings()->value("pilotDisplay/trackLineStrength", 2.0).toDouble();
}

void Settings::setTrackLineStrength(double value) {
	getSettings()->setValue("pilotDisplay/trackLineStrength", value);
}

double Settings::planLineStrength() {
	return getSettings()->value("pilotDisplay/planLineStrength", 0.9).toDouble();
}

void Settings::setPlanLineStrength(double value) {
	getSettings()->setValue("pilotDisplay/planLineStrength", value);
}

void Settings::getRememberedMapPosition(double *xrot, double *yrot, double *zrot, double *zoom) {
	if(xrot == 0 || yrot == 0 || zrot == 0 || zoom == 0)
		return;

	*xrot = getSettings()->value("defaultMapPosition/xrot", *xrot).toDouble();
	*yrot = getSettings()->value("defaultMapPosition/yrot", *yrot).toDouble();
	// ignore zoom. we can't tilt the earth, xy2latlon would fuck up
	*zoom = getSettings()->value("defaultMapPosition/zoom", *zoom).toDouble();
}

void Settings::setRememberedMapPosition(double xrot, double yrot, double zrot, double zoom) {
	getSettings()->setValue("defaultMapPosition/xrot", xrot);
	getSettings()->setValue("defaultMapPosition/yrot", yrot);
	getSettings()->setValue("defaultMapPosition/zrot", zrot);
	getSettings()->setValue("defaultMapPosition/zoom", zoom);
}

void Settings::saveState(const QByteArray& state) {
	getSettings()->setValue("mainWindowState/state", state);
}

QByteArray Settings::getSavedState() {
	return getSettings()->value("mainWindowState/state", QByteArray()).toByteArray();
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


QString Settings::dataDirectory() {
#ifdef Q_WS_X11
	return getSettings()->value("general/dataDirectory", "/usr/share/qutescoop/data/").toString();
#else
	return getSettings()->value("general/dataDirectory", "data/").toString();
#endif
}

void Settings::setDataDirectory(const QString& value) {
	getSettings()->setValue("general/dataDirectory", value);
}

Settings::VoiceType Settings::voiceType() {
	return (VoiceType) getSettings()->value("voice/type", TEAMSPEAK).toInt();
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
