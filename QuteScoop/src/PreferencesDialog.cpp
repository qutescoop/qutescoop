/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "PreferencesDialog.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QDesktopServices>

#include "math.h"

#include "Settings.h"
#include "Window.h"

PreferencesDialog *preferencesDialogInstance = 0;

PreferencesDialog *PreferencesDialog::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(preferencesDialogInstance == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::getInstance(true);
            preferencesDialogInstance = new PreferencesDialog(parent);
        }
    return preferencesDialogInstance;
}

PreferencesDialog::PreferencesDialog(QWidget *parent):
    QDialog(parent),
    settingsLoaded(false)
{
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);

    cbNetwork->addItems(QStringList() << "IVAO" << "VATSIM" << "User Defined Network");
    cbDashedFrontAfter->addItems(QStringList() << "in front" << "behind");
    loadSettings();
}

void PreferencesDialog::loadSettings() {
    settingsLoaded = false;

    spinBoxDownloadInterval->setValue(Settings::downloadInterval());
    cbDownloadOnStartup->setChecked(Settings::downloadOnStartup());
    cbDownloadPeriodically->setChecked(Settings::downloadPeriodically());
    cbNetwork->setCurrentIndex(Settings::downloadNetwork());
    editUserDefinedLocation->setText(Settings::userDownloadLocation());
    editUserDefinedLocation->setEnabled(Settings::downloadNetwork() == 2);
    lbluserDefinedLocation->setEnabled(Settings::downloadNetwork() == 2);
    cbSaveWhazzupData->setChecked(Settings::saveWhazzupData());
    cbShootScreenshots->setChecked(Settings::shootScreenshots());

    gbDownloadBookings->setChecked(Settings::downloadBookings()); // must be after cbNetwork
    editBookingsLocation->setText(Settings::bookingsLocation());
    cbBookingsPeriodically->setChecked(Settings::bookingsPeriodically());
    sbBookingsInterval->setValue(Settings::bookingsInterval());

    sbMaxTextLabels->setValue(Settings::maxLabels());

    groupBoxProxy->setChecked(Settings::useProxy());
    editProxyServer->setText(Settings::proxyServer());
    editProxyPort->setText(QString("%1").arg(Settings::proxyPort()));
    editProxyUser->setText(Settings::proxyUser());
    editProxyPassword->setText(Settings::proxyPassword());

    // directories
    lblScreenshotsDirectory->setText(Settings::applicationDataDirectory("screenshots/"));
    lblDownloadedDirectory->setText(Settings::applicationDataDirectory("downloaded/"));


    // XPlane-Navdata
    editNavdir->setText(Settings::navdataDirectory());
    editNavdir->setEnabled(Settings::useNavdata());
    browseNavdirButton->setEnabled(Settings::useNavdata());
    cbUseNavDatabase->setChecked(Settings::useNavdata());
    cbShowFixes->setChecked(Settings::showFixes());

    // Display
    cbReadSupFile->setChecked(Settings::useSupFile());

    spinBoxTimeline->setValue(Settings::timelineSeconds());
    cbLineSmoothing->setChecked(Settings::displaySmoothLines());
    cbDotSmoothing->setChecked(Settings::displaySmoothDots());

    // Show routes
    cbDashedFrontAfter->setCurrentIndex(0);
    if(!Settings::dashedTrackInFront())
        cbDashedFrontAfter->setCurrentIndex(1);

    // OpenGL
    cbBlend->setChecked(Settings::enableBlend());

    // stylesheet
    tedStylesheet->setPlainText(Settings::stylesheet());

    // colors
    QColor color = Settings::backgroundColor().dark();
    pbBackgroundColor->setText(color.name());
    pbBackgroundColor->setPalette(QPalette(color));

    color = Settings::globeColor();
    pbGlobeColor->setText(color.name());
    pbGlobeColor->setPalette(QPalette(color));

    color = Settings::gridLineColor();
    pbGridLineColor->setText(color.name());
    pbGridLineColor->setPalette(QPalette(color));
    sbGridLineStrength->setValue(Settings::gridLineStrength());

    color = Settings::coastLineColor();
    pbCoastLineColor->setText(color.name());
    pbCoastLineColor->setPalette(QPalette(color));
    sbCoastLineStrength->setValue(Settings::coastLineStrength());

    color = Settings::countryLineColor();
    pbCountryLineColor->setText(color.name());
    pbCountryLineColor->setPalette(QPalette(color));
    sbCountryLineStrength->setValue(Settings::countryLineStrength());

    color = Settings::firBorderLineColor();
    pbFirBorderLineColor->setText(color.name());
    pbFirBorderLineColor->setPalette(QPalette(color));
    sbFirBorderLineStrength->setValue(Settings::firBorderLineStrength());

    color = Settings::firFillColor();
    pbFirFillColor->setText(color.name());
    pbFirFillColor->setPalette(QPalette(color));

    color = Settings::firFontColor();
    pbFirFontColor->setText(color.name());
    pbFirFontColor->setPalette(QPalette(color));
    pbFirFont->setFont(Settings::firFont());

    // airport traffic settings
    cbFilterTraffic->setChecked(Settings::filterTraffic());
    spFilterDistance->setValue(Settings::filterDistance());
    spFilterArriving->setValue(Settings::filterArriving());
    cbShowCongestion->setChecked(Settings::showAirportCongestion());
    sbCongestionBorderLineStrength->setValue(Settings::airportCongestionBorderLineStrength());
    color = Settings::airportCongestionBorderLineColor();
    pbCongestionBorderLineColor->setText(color.name());
    pbCongestionBorderLineColor->setPalette(QPalette(color));
    sbCongestionMinimum->setValue(Settings::airportCongestionMinimum());

    // airport settings
    color = Settings::inactiveAirportFontColor();
    pbInactAirportFontColor->setText(color.name());
    pbInactAirportFontColor->setPalette(QPalette(color));
    pbInactAirportFont->setFont(Settings::inactiveAirportFont());

    color = Settings::airportFontColor();
    pbAirportFontColor->setText(color.name());
    pbAirportFontColor->setPalette(QPalette(color));
    pbAirportFont->setFont(Settings::airportFont());

    color = Settings::appBorderLineColor();
    pbAppBorderLineColor->setText(color.name());
    pbAppBorderLineColor->setPalette(QPalette(color));
    sbAppBorderLineStrength->setValue(Settings::appBorderLineStrength());

    color = Settings::appCenterColor();
    pbAppColorCenter->setText(color.name());
    pbAppColorCenter->setPalette(QPalette(color));

    color = Settings::appMarginColor();
    pbAppColorMargin->setText(color.name());
    pbAppColorMargin->setPalette(QPalette(color));

    color = Settings::twrCenterColor();
    pbTwrColorCenter->setText(color.name());
    pbTwrColorCenter->setPalette(QPalette(color));

    color = Settings::twrMarginColor();
    pbTwrColorMargin->setText(color.name());
    pbTwrColorMargin->setPalette(QPalette(color));

    color = Settings::gndBorderLineColor();
    pbGndBorderLineColor->setText(color.name());
    pbGndBorderLineColor->setPalette(QPalette(color));
    sbGndBorderLineStrength->setValue(Settings::gndBorderLineStrength());

    color = Settings::gndFillColor();
    pbGndFillColor->setText(color.name());
    pbGndFillColor->setPalette(QPalette(color));

    color = Settings::airportDotColor();
    pbAirportDotColor->setText(color.name());
    pbAirportDotColor->setPalette(QPalette(color));
    sbAirportDotSize->setValue(Settings::airportDotSize());

    color = Settings::inactiveAirportDotColor();
    pbInactAirportDotColor->setText(color.name());
    pbInactAirportDotColor->setPalette(QPalette(color));
    sbInactAirportDotSize->setValue(Settings::inactiveAirportDotSize());

    // pilot settings
    color = Settings::pilotFontColor();
    pbPilotFontColor->setText(color.name());
    pbPilotFontColor->setPalette(QPalette(color));
    pbPilotFont->setFont(Settings::pilotFont());

    color = Settings::pilotDotColor();
    pbPilotDotColor->setText(color.name());
    pbPilotDotColor->setPalette(QPalette(color));
    sbPilotDotSize->setValue(Settings::pilotDotSize());

    color = Settings::timeLineColor();
    pbTimeLineColor->setText(color.name());
    pbTimeLineColor->setPalette(QPalette(color));
    sbTimeLineStrength->setValue(Settings::timeLineStrength());

    color = Settings::trackLineColor();
    pbTrackLineColor->setText(color.name());
    pbTrackLineColor->setPalette(QPalette(color));
    sbTrackLineStrength->setValue(Settings::trackLineStrength());

    color = Settings::planLineColor();
    pbPlanLineColor->setText(color.name());
    pbPlanLineColor->setPalette(QPalette(color));
    sbPlanLineStrength->setValue(Settings::planLineStrength());

    cbTrackFront->setChecked(Settings::trackFront());
    cbTrackAfter->setChecked(Settings::trackAfter());

    // voice
    editVoiceCallsign->setText(Settings::voiceCallsign());
    editVoiceUser->setText(Settings::voiceUser());
    editVoicePassword->setText(Settings::voicePassword());
    rbNone->setChecked(Settings::voiceType() == Settings::NONE);
    rbTeamSpeak->setChecked(Settings::voiceType() == Settings::TEAMSPEAK);
    rbVRC->setChecked(Settings::voiceType() == Settings::VRC);
    editVoiceCallsign->setEnabled(Settings::voiceType() == Settings::TEAMSPEAK);
    editVoicePassword->setEnabled(Settings::voiceType() == Settings::TEAMSPEAK);
    editVoiceUser->setEnabled(Settings::voiceType() == Settings::TEAMSPEAK);

    // updates + feedback
    cbCheckForUpdates->setChecked(Settings::checkForUpdates());
    cbSendVersionInfo->setChecked(Settings::sendVersionInformation());

    // zooming
    sbZoomFactor->setValue(Settings::zoomFactor());

    // FINISHED
    settingsLoaded = true;
}

// airport traffic settings
void PreferencesDialog::on_cbFilterTraffic_stateChanged(int state) {
    Settings::setFilterTraffic(state);
}

void PreferencesDialog::on_spFilterDistance_valueChanged(int value) {
    Settings::setFilterDistance(value);
}

void PreferencesDialog::on_spFilterArriving_valueChanged(double value) {
    Settings::setFilterArriving(value);
}

void PreferencesDialog::on_sbCongestionMinimum_valueChanged(int value)
{
    Settings::setAirportCongestionMinimum(value);
}
//

void PreferencesDialog::on_sbMaxTextLabels_valueChanged(int value) {
    Settings::setMaxLabels(value);
}

void PreferencesDialog::on_spinBoxDownloadInterval_valueChanged(int value) {
    Settings::setDownloadInterval(value);
}

void PreferencesDialog::on_cbDownloadPeriodically_stateChanged(int state) {
    Settings::setDownloadPeriodically(state == Qt::Checked);
}

void PreferencesDialog::on_cbUseNavDatabase_stateChanged(int state) {
    Settings::setUseNavdata(state == Qt::Checked);
}

void PreferencesDialog::on_cbResetConfiguration_stateChanged(int state) {
    Settings::setResetOnNextStart(state == Qt::Checked);
}

void PreferencesDialog::on_cbCheckForUpdates_stateChanged(int state) {
    Settings::setCheckForUpdates(state == Qt::Checked);
}

void PreferencesDialog::on_cbSendVersionInfo_stateChanged(int state) {
    Settings::setSendVersionInformation(state == Qt::Checked);
}

void PreferencesDialog::on_cbDownloadOnStartup_stateChanged(int state) {
    Settings::setDownloadOnStartup(state == Qt::Checked);
}

void PreferencesDialog::on_cbReadSupFile_stateChanged(int state) {
    Settings::setUseSupFile(state == Qt::Checked);
}

void PreferencesDialog::on_cbNetwork_currentIndexChanged(int index) {
    // event is triggered when combobox is being created, so ignore it until we're ready
    if(!settingsLoaded)
        return;

    Settings::setDownloadNetwork(index);

    switch(index) {
    case 0: // IVAO
        Settings::setStatusLocation("http://www.ivao.aero/whazzup/status.txt");
        gbDownloadBookings->setChecked(false);
        break;
    case 1: // VATSIM
        Settings::setStatusLocation("http://www.vatsim.net/data/status.txt");
        gbDownloadBookings->setChecked(true);
        break;
    case 2: // user defined
        Settings::setStatusLocation(editUserDefinedLocation->text());
        gbDownloadBookings->setChecked(false);
        break;
    }

    editUserDefinedLocation->setEnabled(index == 2);
    lbluserDefinedLocation->setEnabled(index == 2);
}

void PreferencesDialog::on_cbDashedFrontAfter_currentIndexChanged(int index) {
    Settings::setDashedTrackInFront(index == 0);
}

void PreferencesDialog::on_editUserDefinedLocation_editingFinished() {
    Settings::setUserDownloadLocation(editUserDefinedLocation->text());
    Settings::setStatusLocation(editUserDefinedLocation->text());
}

void PreferencesDialog::on_cbSaveWhazzupData_stateChanged(int state)
{
    Settings::setSaveWhazzupData(state == Qt::Checked);
}

void PreferencesDialog::on_groupBoxProxy_toggled(bool checked) {
    Settings::setUseProxy(checked);
}

void PreferencesDialog::on_editProxyServer_editingFinished() {
    Settings::setProxyServer(editProxyServer->text());
}

void PreferencesDialog::on_editProxyPort_editingFinished() {
    Settings::setProxyPort(editProxyPort->text().toInt());
}

void PreferencesDialog::on_editProxyUser_editingFinished() {
    Settings::setProxyUser(editProxyUser->text());
}

void PreferencesDialog::on_editProxyPassword_editingFinished() {
    Settings::setProxyPassword(editProxyPassword->text());
}

void PreferencesDialog::on_spinBoxTimeline_valueChanged(int value) {
    Settings::setTimelineSeconds(value);
}

void PreferencesDialog::on_cbLineSmoothing_stateChanged(int state) {
    Settings::setDisplaySmoothLines(state == Qt::Checked);
}

void PreferencesDialog::on_cbDotSmoothing_stateChanged(int state) {
    Settings::setDisplaySmoothDots(state == Qt::Checked);
}

void PreferencesDialog::on_editNavdir_editingFinished() {
    Settings::setNavdataDirectory(editNavdir->text());
}

void PreferencesDialog::on_browseNavdirButton_clicked() {
    QFileDialog* dialog = new QFileDialog(this, tr("Select Database Directory"),
            Settings::navdataDirectory());
    dialog->setFileMode(QFileDialog::DirectoryOnly);
    int result = dialog->exec();
    if(result == QDialog::Rejected) {
        delete dialog;
        return;
    }

    QString dir = dialog->directory().absolutePath();
    editNavdir->setText(dir);
    Settings::setNavdataDirectory(dir);

    delete dialog;
}

void PreferencesDialog::on_pbBackgroundColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::backgroundColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbBackgroundColor->setText(color.dark().name());
        pbBackgroundColor->setPalette(QPalette(color.dark()));
        Settings::setBackgroundColor(color);
    }
}

void PreferencesDialog::on_pbGlobeColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::globeColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbGlobeColor->setText(color.name());
        pbGlobeColor->setPalette(QPalette(color));
        Settings::setGlobeColor(color);
    }
}

void PreferencesDialog::on_pbGridLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::gridLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbGridLineColor->setText(color.name());
        pbGridLineColor->setPalette(QPalette(color));
        Settings::setGridLineColor(color);
    }
}

void PreferencesDialog::on_sbGridLineStrength_valueChanged(double value) {
    Settings::setGridLineStrength(value);
}

void PreferencesDialog::on_pbCountryLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::countryLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbCountryLineColor->setText(color.name());
        pbCountryLineColor->setPalette(QPalette(color));
        Settings::setCountryLineColor(color);
    }
}

void PreferencesDialog::on_sbCountryLineStrength_valueChanged(double value) {
    Settings::setCountryLineStrength(value);
}

void PreferencesDialog::on_pbCoastLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::coastLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbCoastLineColor->setText(color.name());
        pbCoastLineColor->setPalette(QPalette(color));
        Settings::setCoastLineColor(color);
    }
}

void PreferencesDialog::on_sbCoastLineStrength_valueChanged(double value) {
    Settings::setCoastLineStrength(value);
}

void PreferencesDialog::on_buttonResetEarthSpace_clicked() {
    Settings::deleteEarthSpaceSettings();
    loadSettings();
}

void PreferencesDialog::on_pbFirBorderLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::firBorderLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbFirBorderLineColor->setText(color.name());
        pbFirBorderLineColor->setPalette(QPalette(color));
        Settings::setFirBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbFirBorderLineStrength_valueChanged(double value) {
    Settings::setFirBorderLineStrength(value);
}

void PreferencesDialog::on_pbFirFontColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::firFontColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbFirFontColor->setText(color.name());
        pbFirFontColor->setPalette(QPalette(color));
        Settings::setFirFontColor(color);
    }
}

void PreferencesDialog::on_pbFirFont_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Settings::firFont(), this);
    if(ok) {
        pbFirFont->setFont(font);
        Settings::setFirFont(font);
    }
}

void PreferencesDialog::on_pbFirFillColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::firFillColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbFirFillColor->setText(color.name());
        pbFirFillColor->setPalette(QPalette(color));
        Settings::setFirFillColor(color);
    }
}

void PreferencesDialog::on_buttonResetFir_clicked() {
    Settings::deleteFirSettings();
    loadSettings();
}

void PreferencesDialog::on_pbAirportFontColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::airportFontColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbAirportFontColor->setText(color.name());
        pbAirportFontColor->setPalette(QPalette(color));
        Settings::setAirportFontColor(color);
    }
}

void PreferencesDialog::on_pbAirportFont_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Settings::airportFont(), this);
    if(ok) {
        pbAirportFont->setFont(font);
        Settings::setAirportFont(font);
    }
}

void PreferencesDialog::on_pbAppBorderLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::appBorderLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbAppBorderLineColor->setText(color.name());
        pbAppBorderLineColor->setPalette(QPalette(color));
        Settings::setAppBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbAppBorderLineStrength_valueChanged(double value) {
    Settings::setAppBorderLineStrength(value);
}

void PreferencesDialog::on_pbAppColorCenter_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::appCenterColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbAppColorCenter->setText(color.name());
        pbAppColorCenter->setPalette(QPalette(color));
        Settings::setAppCenterColor(color);
    }
}

void PreferencesDialog::on_pbAppColorMargin_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::appMarginColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbAppColorMargin->setText(color.name());
        pbAppColorMargin->setPalette(QPalette(color));
        Settings::setAppMarginColor(color);
    }
}

void PreferencesDialog::on_pbTwrColorCenter_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::twrCenterColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbTwrColorCenter->setText(color.name());
        pbTwrColorCenter->setPalette(QPalette(color));
        Settings::setTwrCenterColor(color);
    }
}

void PreferencesDialog::on_pbTwrColorMargin_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::twrMarginColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbTwrColorMargin->setText(color.name());
        pbTwrColorMargin->setPalette(QPalette(color));
        Settings::setTwrMarginColor(color);
    }
}

void PreferencesDialog::on_pbGndBorderLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::gndBorderLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbGndBorderLineColor->setText(color.name());
        pbGndBorderLineColor->setPalette(QPalette(color));
        Settings::setGndBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbGndBorderLineStrength_valueChanged(double value) {
    Settings::setGndBorderLineStrength(value);
}

void PreferencesDialog::on_pbGndFillColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::gndFillColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbGndFillColor->setText(color.name());
        pbGndFillColor->setPalette(QPalette(color));
        Settings::setGndFillColor(color);
    }
}

void PreferencesDialog::on_pbAirportDotColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::airportDotColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbAirportDotColor->setText(color.name());
        pbAirportDotColor->setPalette(QPalette(color));
        Settings::setAirportDotColor(color);
    }
}

void PreferencesDialog::on_sbAirportDotSize_valueChanged(double value) {
    Settings::setAirportDotSize(value);
}

void PreferencesDialog::on_buttonResetAirport_clicked() {
    Settings::deleteAirportSettings();
    loadSettings();
}

void PreferencesDialog::on_pbPilotFontColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::pilotFontColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbPilotFontColor->setText(color.name());
        pbPilotFontColor->setPalette(QPalette(color));
        Settings::setPilotFontColor(color);
    }
}

void PreferencesDialog::on_pbPilotFont_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Settings::pilotFont(), this);
    if(ok) {
        pbPilotFont->setFont(font);
        Settings::setPilotFont(font);
    }
}

void PreferencesDialog::on_pbPilotDotColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::pilotDotColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbPilotDotColor->setText(color.name());
        pbPilotDotColor->setPalette(QPalette(color));
        Settings::setPilotDotColor(color);
    }
}

void PreferencesDialog::on_sbPilotDotSize_valueChanged(double value) {
    Settings::setPilotDotSize(value);
}

void PreferencesDialog::on_pbTimeLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::timeLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbTimeLineColor->setText(color.name());
        pbTimeLineColor->setPalette(QPalette(color));
        Settings::setTimeLineColor(color);
    }
}

void PreferencesDialog::on_sbTimeLineStrength_valueChanged(double value) {
    Settings::setTimeLineStrength(value);
}

void PreferencesDialog::on_pbTrackLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::trackLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbTrackLineColor->setText(color.name());
        pbTrackLineColor->setPalette(QPalette(color));
        Settings::setTrackLineColor(color);
    }
}

void PreferencesDialog::on_sbTrackLineStrength_valueChanged(double value) {
    Settings::setTrackLineStrength(value);
}

void PreferencesDialog::on_pbPlanLineColor_clicked() {
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::planLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbPlanLineColor->setText(color.name());
        pbPlanLineColor->setPalette(QPalette(color));
        Settings::setPlanLineColor(color);
    }
}

void PreferencesDialog::on_sbPlanLineStrength_valueChanged(double value) {
    Settings::setPlanLineStrength(value);
}

void PreferencesDialog::on_buttonResetPilot_clicked() {
    Settings::deleteAircraftSettings();
    loadSettings();
}

void PreferencesDialog::on_editVoiceCallsign_editingFinished() {
    Settings::setVoiceCallsign(editVoiceCallsign->text());
}

void PreferencesDialog::on_editVoiceUser_editingFinished() {
    Settings::setVoiceUser(editVoiceUser->text());
}

void PreferencesDialog::on_editVoicePassword_editingFinished() {
    Settings::setVoicePassword(editVoicePassword->text());
}

void PreferencesDialog::on_rbNone_clicked(bool value) {
    if(value) {
        Settings::setVoiceType(Settings::NONE);
    }
}

void PreferencesDialog::on_rbTeamSpeak_clicked(bool value) {
    if(value) {
        Settings::setVoiceType(Settings::TEAMSPEAK);
    }
}

void PreferencesDialog::on_rbVRC_clicked(bool value) {
    if(value) {
        Settings::setVoiceType(Settings::VRC);
    }
}

void PreferencesDialog::on_cbShowFixes_toggled(bool checked)
{
    Settings::setShowFixes(checked);
}

void PreferencesDialog::on_pbInactAirportFontColor_clicked()
{
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::inactiveAirportFontColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbInactAirportFontColor->setText(color.name());
        pbInactAirportFontColor->setPalette(QPalette(color));
        Settings::setInactiveAirportFontColor(color);
    }
}

void PreferencesDialog::on_pbInactAirportFont_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Settings::inactiveAirportFont(), this);
    if(ok) {
        pbInactAirportFont->setFont(font);
        Settings::setInactiveAirportFont(font);
    }
}

void PreferencesDialog::on_pbInactAirportDotColor_clicked()
{
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::inactiveAirportDotColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbInactAirportDotColor->setText(color.name());
        pbInactAirportDotColor->setPalette(QPalette(color));
        Settings::setInactiveAirportDotColor(color);
    }
}

void PreferencesDialog::on_sbInactAirportDotSize_valueChanged(double value)
{
    Settings::setInactiveAirportDotSize(value);
}

void PreferencesDialog::on_cbShowCongestion_clicked(bool checked)
{
    Settings::setAirportCongestion(checked);
}

void PreferencesDialog::on_pbCongestionBorderLineColor_clicked()
{
    bool ok;
    QRgb rgba = QColorDialog::getRgba(Settings::airportCongestionBorderLineColor().rgba(), &ok, this);
    if(ok) {
        QColor color = QColor::fromRgba(rgba);
        pbCongestionBorderLineColor->setText(color.name());
        pbCongestionBorderLineColor->setPalette(QPalette(color));
        Settings::setAirportCongestionBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbCongestionBorderLineStrength_valueChanged(double value)
{
    Settings::setAirportCongestionBorderLineStrength(value);
}

void PreferencesDialog::on_buttonResetAirportTraffic_clicked()
{
    Settings::deleteAirportTrafficSettings();
    loadSettings();
}

void PreferencesDialog::on_cbBlend_toggled(bool checked)
{
    Settings::setEnableBlend(checked);
}

void PreferencesDialog::on_editBookingsLocation_editingFinished()
{
    Settings::setBookingsLocation(editBookingsLocation->text());
}

void PreferencesDialog::on_cbBookingsPeriodically_toggled(bool checked)
{
    Settings::setBookingsPeriodically(checked);
}

void PreferencesDialog::on_sbBookingsInterval_valueChanged(int value)
{
    Settings::setBookingsInterval(value);
}

// zooming
void PreferencesDialog::on_pbWheelCalibrate_clicked()
{
    Settings::deleteWheelSettings();
}

void PreferencesDialog::on_sbZoomFactor_valueChanged(double value)
{
    Settings::setZoomFactor(value);
}

void PreferencesDialog::on_gbDownloadBookings_toggled(bool checked)
{
    Settings::setDownloadBookings(checked);
    qobject_cast<Window *>(this->parent())->setEnableBookedAtc(checked);
}

void PreferencesDialog::on_cbTrackFront_toggled(bool checked)
{
    Settings::setTrackFront(checked);
    if(!checked && !cbTrackAfter->isChecked()) cbTrackAfter->setChecked(true);
}

void PreferencesDialog::on_cbTrackAfter_toggled(bool checked)
{
    Settings::setTrackAfter(checked);
    if(!checked && !cbTrackFront->isChecked()) cbTrackFront->setChecked(true);
}


//import & export
void PreferencesDialog::on_pbImportFromFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
            this, tr("Import from File"),
            QApplication::applicationDirPath(), tr("Settings Files (*.ini);; All Files (*.*)")
    );
    if (!fileName.isEmpty()) {
        Settings::importFromFile(fileName);
        QMessageBox::information(this, tr("Settings loaded. Restart required."),
                                 tr("QuteScoop closes now. Please restart for the settings to take effect."));
        loadSettings();
        qApp->processEvents();
        qobject_cast<Window *>(this->parent())->close();
    }
}

void PreferencesDialog::on_pbExportToFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
            this, tr("Export to File"),
            QApplication::applicationDirPath(), tr("Settings Files (*.ini);; All Files (*.*)")
    );
    if (!fileName.isEmpty()) {
        Settings::exportToFile(fileName);
        QMessageBox::information(this, tr("Success"), tr("Settings exported."));
    }
}

void PreferencesDialog::on_cbShootScreenshots_toggled(bool checked)
{
    Settings::setShootScreenshots(checked);
}

void PreferencesDialog::on_pbStylesheetUpdate_clicked()
{
    qobject_cast<Window *>(this->parent())->setStyleSheet(tedStylesheet->toPlainText());
    Settings::setStylesheet(tedStylesheet->toPlainText());
}

void PreferencesDialog::on_pbStylesheetExample1_clicked()
{
    if (
            QMessageBox::question(
                    this, tr("Overwrite stylesheet?"), tr("Are you sure?"),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            == QMessageBox::Yes) {
        tedStylesheet->setPlainText(QLatin1String("\
QCheckBox::indicator:hover, QDateTimeEdit, QSpinBox, QComboBox, QAbstractItemView, QLineEdit, QSpinBox, QDoubleSpinBox, QTabWidget::pane, QGroupBox {\n\
background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n\
      stop: 0 #f6f7fa, stop: 1 #dadbde);\n\
}\n\
QDateTimeEdit::drop-down, QToolButton, QAbstractSpinBox::up-button, QAbstractSpinBox::down-button, QComboBox::drop-down, QTabBar::tab:selected, QPushButton, QMenuBar:selected, QMenu:selected, QComboBox:selected, QMenuBar, QMenu, QTabBar::tab  {\n\
border: 1px solid #aaa;\n\
padding: 3px;\n\
background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n\
      stop: 0 #dadbde, stop: 1 #f6f7fa);\n\
}\n\
QDialog, QMainWindow {\n\
background: qradialgradient(spread:repeat, cx:0.55, cy:0.5, radius:0.077, fx:0.5, fy:0.5, stop:0 rgba(200, 239, 255, 255), stop:0.497326 rgba(200, 230, 230, 47), stop:1 rgba(200, 239, 235, 255))\n\
}\n\
QMenuBar, QMenu, QTabBar::tab {\n\
background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #eee, stop: 1 #fff);\n\
}\n\
        "));
    }
}

void PreferencesDialog::on_pbStylesheetExample2_clicked()
{
    if (
            QMessageBox::question(
                    this, tr("Overwrite stylesheet?"), tr("Are you sure?"),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            == QMessageBox::Yes) {
        tedStylesheet->setPlainText(QLatin1String("\
QDialog, QMainWindow {\n\
  background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #bef, stop: 1 #eff);\n\
}\n\
QMenuBar, QMenu, QTabBar::tab {\n\
  background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #eff, stop: 1 #bef);\n\
}\n\
QDateTimeEdit::drop-down, QToolButton, QAbstractSpinBox::up-button, QAbstractSpinBox::down-button, QComboBox::drop-down, QTabBar::tab:selected, QPushButton, QMenuBar:selected, QMenu:selected, QComboBox:selected {\n\
  color: white;\n\
  background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #88d, stop: 0.1 #99e, stop: 0.49 #77c, stop: 0.5 #66b, stop: 1 #77c);\n\
  border: 1px solid #339;\n\
  border-radius: 5px;\n\
  padding: 3px;\n\
  padding-left: 5px;\n\
  padding-right: 5px;\n\
}\n\
QCheckBox::indicator:hover, QDateTimeEdit, QSpinBox, QComboBox, QAbstractItemView, QLineEdit, QSpinBox, QDoubleSpinBox, QTabWidget::pane, QGroupBox {\n\
  border-style: solid;\n\
  border: 1px solid gray;\n\
  border-radius: 5px;\n\
}\n\
QLabel {\n\
  background: rgba(0,0,0,0);\n\
}\n\
        "));
    }
}


