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

#ifndef PREFERENCESDIALOG_H_
#define PREFERENCESDIALOG_H_

#include <QDialog>

#include "ui_PreferencesDialog.h"

class PreferencesDialog: public QDialog, private Ui::PreferencesDialog
{
	Q_OBJECT

public:
	PreferencesDialog();

private slots:
	// download settings
	void on_spinBoxDownloadInterval_valueChanged(int value);
	void on_cbDownloadPeriodically_stateChanged(int state);
	void on_cbDownloadOnStartup_stateChanged(int state);
	void on_cbNetwork_currentIndexChanged(int index);
	void on_editUserDefinedLocation_editingFinished();

	// proxy settings
	void on_cbUseProxy_stateChanged(int state);
	void on_editProxyServer_editingFinished();
	void on_editProxyPort_editingFinished();
	void on_editProxyUser_editingFinished();
	void on_editProxyPassword_editingFinished();
	
	// airport traffic settings
	void on_cbFilterTraffic_stateChanged(int state);
	void on_spFilterDistance_valueChanged(int value);
	void on_spFilterArriving_valueChanged(double value);
	
	// display
	void on_spinBoxTimeline_valueChanged(int value);
	void on_cbLineSmoothing_stateChanged(int state);
	void on_cbDotSmoothing_stateChanged(int state);
	void on_cbReadSupFile_stateChanged(int state);
	void on_sbMaxTextLabels_valueChanged(int value);

	// navdata
	void on_editNavdir_editingFinished();
	void on_browseNavdirButton_clicked();
	void on_cbUseNavDatabase_stateChanged(int state);

	void loadSettings();

	// earth and space
	void on_pbBackgroundColor_clicked();
	void on_pbGlobeColor_clicked();
	void on_pbGridLineColor_clicked();
	void on_sbGridLineStrength_valueChanged(double value);
	void on_pbCoastLineColor_clicked();
	void on_sbCoastLineStrength_valueChanged(double value);
	void on_pbCountryLineColor_clicked();
	void on_sbCountryLineStrength_valueChanged(double value);
	void on_buttonResetEarthSpace_clicked();

	// FIR
	void on_pbFirBorderLineColor_clicked();
	void on_sbFirBorderLineStrength_valueChanged(double value);
	void on_pbFirFontColor_clicked();
	void on_pbFirFont_clicked();
	void on_pbFirFillColor_clicked();
	void on_buttonResetFir_clicked();

	// Airport
	void on_pbAirportDotColor_clicked();
	void on_sbAirportDotSize_valueChanged(double value);
	void on_pbAirportFontColor_clicked();
	void on_pbAirportFont_clicked();
	void on_pbAppBorderLineColor_clicked();
	void on_sbAppBorderLineStrength_valueChanged(double value);
	void on_pbAppColorCenter_clicked();
	void on_pbAppColorMargin_clicked();
	void on_pbTwrColorCenter_clicked();
	void on_pbTwrColorMargin_clicked();
	void on_pbGndBorderLineColor_clicked();
	void on_sbGndBorderLineStrength_valueChanged(double value);
	void on_pbGndFillColor_clicked();
	void on_buttonResetAirport_clicked();

	// Aircraft
	void on_pbPilotFontColor_clicked();
	void on_pbPilotFont_clicked();
	void on_pbPilotDotColor_clicked();
	void on_sbPilotDotSize_valueChanged(double value);
	void on_pbTimeLineColor_clicked();
	void on_sbTimeLineStrength_valueChanged(double value);
	void on_pbTrackLineColor_clicked();
	void on_sbTrackLineStrength_valueChanged(double value);
	void on_pbPlanLineColor_clicked();
	void on_sbPlanLineStrength_valueChanged(double value);
	void on_buttonResetPilot_clicked();
	void on_cbDashedFrontAfter_currentIndexChanged(int index);

	void on_cbResetConfiguration_stateChanged(int state);

	// voice
	void on_rbNone_clicked(bool value);
	void on_rbTeamSpeak_clicked(bool value);
	void on_rbVRC_clicked(bool value);
	void on_editVoiceCallsign_editingFinished();
	void on_editVoiceUser_editingFinished();
	void on_editVoicePassword_editingFinished();

	// updates + feedback
	void on_cbCheckForUpdates_stateChanged(int state);
	void on_cbSendVersionInfo_stateChanged(int state);

private:
	bool settingsLoaded;

};

#endif /*PREFERENCESDIALOG_H_*/
