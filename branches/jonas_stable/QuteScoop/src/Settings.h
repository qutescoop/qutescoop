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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QString>
#include <QColor>
#include <QFont>
#include <QHttp>
#include <QByteArray>
#include <QStringList>
#include <QSettings>

class Settings
{
public:
	static void saveState(const QByteArray& state);
	static QByteArray getSavedState();

	static void saveSize(const QSize& size);
	static QSize getSavedSize();

	static void savePosition(const QPoint& pos);
	static QPoint getSavedPosition();

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

	static void applyProxySetting(QHttp *http);

	// Airport traffic settings --------------------------------
    static void deleteAirportTrafficSettings();
    
	static bool filterTraffic();
	static void setFilterTraffic(bool v);

	static int filterDistance();
	static void setFilterDistance(int v);

	static double filterArriving();
	static void setFilterArriving(double v);
    
    // Airport congestion
    static bool showAirportCongestion();
    static void setAirportCongestion(bool v);

    static int airportCongestionMinimum();
    static void setAirportCongestionMinimum(int v);

    static QColor airportCongestionBorderLineColor();
	static void setAirportCongestionBorderLineColor(const QColor& color);

	static double airportCongestionBorderLineStrength();
	static void setAirportCongestionBorderLineStrength(double value);
	// -----

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

   	static bool showFixes();
	static void setShowFixes(bool value);

	static int metarDownloadInterval();
	static void setMetarDownloadInterval(int minutes);

	// Earth and Space settings ----------------------------
	static void deleteEarthSpaceSettings();

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

	// FIR settings ----------------------------------
	static void deleteFirSettings();

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

	// Airport Settings ----------------------------------
	static void deleteAirportSettings();

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

	// Aircraft Settings -----------------------------------
	static void deleteAircraftSettings();

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

	static QColor trackLineColor();
	static void setTrackLineColor(const QColor& color);

	static QColor planLineColor();
	static void setPlanLineColor(const QColor& color);

	static double timeLineStrength();
	static void setTimeLineStrength(double value);

	static double trackLineStrength();
	static void setTrackLineStrength(double value);

	static double planLineStrength();
	static void setPlanLineStrength(double value);

	static void setDashedTrackInFront(bool value);
	static bool dashedTrackInFront();

	static bool checkForUpdates();
	static void setCheckForUpdates(bool value);

	static bool sendVersionInformation();
	static void setSendVersionInformation(bool value);

	static QString updateVersionNumber();
	static void setUpdateVersionNumber(const QString& version);

    static void getRememberedMapPosition(double *xrot, double *yrot, double *zrot, double *zoom, int nr);
    static void setRememberedMapPosition(double xrot, double yrot, double zrot, double zoom, int nr);

	static int maxLabels();
	static void setMaxLabels(int maxLabels);

	static QStringList friends();
	static void addFriend(const QString& friendId);
	static void removeFriend(const QString& friendId);

	static bool resetOnNextStart();
	static void setResetOnNextStart(bool value);

	enum VoiceType { NONE, TEAMSPEAK, VRC };
	static VoiceType voiceType();
	static void setVoiceType(VoiceType type);

	static QString voiceCallsign();
	static void setVoiceCallsign(const QString& value);

	static QString voiceUser();
	static void setVoiceUser(const QString& value);

	static QString voicePassword();
	static void setVoicePassword(const QString& value);

	static QString dataDirectory();
	static void setDataDirectory(const QString& value);

    static QString bookingsLocation();
	static void setBookingsLocation(const QString& value);

private:
	static QSettings *getSettings();
};

#endif /*SETTINGS_H_*/
