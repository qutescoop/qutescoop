/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "PreferencesDialog.h"
#include "Settings.h"
#include "SondeData.h"
#include "Window.h"

//singleton instance
PreferencesDialog *preferencesDialogInstance = 0;
PreferencesDialog *PreferencesDialog::instance(bool createIfNoInstance, QWidget *parent) {
    if(preferencesDialogInstance == 0) {
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::instance();
            preferencesDialogInstance = new PreferencesDialog(parent);
        }
    }
    return preferencesDialogInstance;
}

PreferencesDialog::PreferencesDialog(QWidget *parent):
        QDialog(parent),
        _settingsLoaded(false) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);

    foreach(const QByteArray &fmt, QImageWriter::supportedImageFormats())
        if(cbScreenshotFormat->findText(fmt.toLower()) == -1)
            cbScreenshotFormat->addItem(fmt.toLower());
    loadSettings();
}

void PreferencesDialog::loadSettings() {
    _settingsLoaded = false;

    QColor color;
    spinBoxDownloadInterval->setValue(Settings::downloadInterval());
    cbDownloadOnStartup->setChecked(Settings::downloadOnStartup());
    cbDownloadPeriodically->setChecked(Settings::downloadPeriodically());
    cbNetwork->setCurrentIndex(Settings::downloadNetwork());
    editUserDefinedLocation->setText(Settings::userDownloadLocation());
    editUserDefinedLocation->setEnabled(Settings::downloadNetwork() == 1);
    lbluserDefinedLocation->setEnabled(Settings::downloadNetwork() == 1);
    cbSaveWhazzupData->setChecked(Settings::saveWhazzupData());

    // screenshots
    cbShootScreenshots->setChecked(Settings::shootScreenshots());
    cbScreenshotMethod->setCurrentIndex(Settings::screenshotMethod());
    cbScreenshotFormat->setCurrentIndex(cbScreenshotFormat->findText(
            Settings::screenshotFormat()));

    gbDownloadBookings->setChecked(
            Settings::downloadBookings()); // must be after cbNetwork
    editBookingsLocation->setText(Settings::bookingsLocation());
    cbBookingsPeriodically->setChecked(Settings::bookingsPeriodically());
    sbBookingsInterval->setValue(Settings::bookingsInterval());

    sbMaxTextLabels->setValue(Settings::maxLabels());
    cbSimpleLabels->setChecked(Settings::simpleLabels());

    groupBoxProxy->setChecked(Settings::useProxy());
    editProxyServer->setText(Settings::proxyServer());
    editProxyPort->setText(QString("%1").arg(Settings::proxyPort()));
    editProxyUser->setText(Settings::proxyUser());
    editProxyPassword->setText(Settings::proxyPassword());

    // directories
    lblScreenshotsDirectory->setText(Settings::dataDirectory("screenshots/"));
    lblDownloadedDirectory->setText(Settings::dataDirectory("downloaded/"));


    // XPlane-Navdata
    editNavdir->setText(Settings::navdataDirectory());
    editNavdir->setEnabled(Settings::useNavdata());
    browseNavdirButton->setEnabled(Settings::useNavdata());
    cbUseNavDatabase->setChecked(Settings::useNavdata());
    cbShowFixes->setChecked(Settings::showAllWaypoints());

    // Display
    cbRememberMapPositionOnClose->setChecked(Settings::rememberMapPositionOnClose());
    spinBoxTimeline->setValue(Settings::timelineSeconds());

    // OpenGL
    glTextures->setChecked(Settings::glTextures());

    // textures
    QDir texDir = QDir(Settings::dataDirectory("textures/"));
    QStringList nameFilters;
    foreach(const QByteArray fmt, QImageReader::supportedImageFormats())
        nameFilters.append("*." + fmt);
    texDir.setNameFilters(nameFilters);
    qDebug() << "Supported texture formats:"
            << QImageReader::supportedImageFormats() << ". See"
            << Settings::dataDirectory("textures/+notes.txt")
            << "for more information.";
    glTextureEarth->setToolTip(
            QString("Shows all textures from\n%1\n in the supported formats\n"
                    "See +notes.txt in the texture directory for more information.").
            arg(Settings::dataDirectory("textures")));
    glTextureEarth->addItems(texDir.entryList()); // first without icons, use lazy-load
    QTimer::singleShot(100, this, SLOT(lazyloadTextureIcons()));

    glTextureEarth->setCurrentIndex(glTextureEarth->findText(Settings::glTextureEarth()));

    glStippleLines->setChecked(Settings::glStippleLines());
    cbBlend->setChecked(Settings::glBlending());
    cbLineSmoothing->setChecked(Settings::displaySmoothLines());
    cbDotSmoothing->setChecked(Settings::displaySmoothDots());
    cbLighting->setChecked(Settings::glLighting());
    glEarthShininess->setValue(Settings::earthShininess());
    glLights->setValue(Settings::glLights());
    glLightsSpread->setValue(Settings::glLightsSpread());

    color = Settings::sunLightColor();
    pbSunLightColor->setText(color.name());
    pbSunLightColor->setPalette(QPalette(color));

    color = Settings::specularColor();
    pbSpecularLightColor->setText(color.name());
    pbSpecularLightColor->setPalette(QPalette(color));

    // stylesheet
    tedStylesheet->setPlainText(Settings::stylesheet());

    // earthspace
    sbEarthGridEach->setValue(Settings::earthGridEach());

    color = Settings::backgroundColor();
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

    color = Settings::firHighlightedBorderLineColor();
    pbFirHighlightedBorderLineColor->setText(color.name());
    pbFirHighlightedBorderLineColor->setPalette(QPalette(color));
    sbFirHighlightedBorderLineStrength->setValue(Settings::firHighlightedBorderLineStrength());

    color = Settings::firHighlightedFillColor();
    pbFirHighlightedFillColor->setText(color.name());
    pbFirHighlightedFillColor->setPalette(QPalette(color));

    color = Settings::windColor();
    pbUpperWindColor->setText(color.name());
    pbUpperWindColor->setPalette(QPalette(color));
    sbWindSize->setValue(Settings::windArrowSize());
    sbWindSecondarySpan->setValue(Settings::sondeAltSecondarySpan_1k());

    // airport traffic settings
    cbFilterTraffic->setChecked(Settings::filterTraffic());
    spFilterDistance->setValue(Settings::filterDistance());
    spFilterArriving->setValue(Settings::filterArriving());
    cbShowCongestion->setChecked(Settings::showAirportCongestion());
    sbCongestionBorderLineStrength->setValue(
            Settings::airportCongestionBorderLineStrength());
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
    sbAppBorderLineStrength->setValue(Settings::appBorderLineWidth());

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
    sbGndBorderLineStrength->setValue(Settings::gndBorderLineWidth());

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

    color = Settings::leaderLineColor();
    pbTimeLineColor->setText(color.name());
    pbTimeLineColor->setPalette(QPalette(color));
    sbTimeLineStrength->setValue(Settings::timeLineStrength());

    color = Settings::depLineColor();
    pbDepLineColor->setText(color.name());
    pbDepLineColor->setPalette(QPalette(color));
    sbDepLineStrength->setValue(Settings::depLineStrength());
    cbDepLineDashed->setChecked(Settings::depLineDashed());

    color = Settings::destLineColor();
    pbDestLineColor->setText(color.name());
    pbDestLineColor->setPalette(QPalette(color));
    sbDestLineStrength->setValue(Settings::destLineStrength());
    cbDestLineDashed->setChecked(Settings::destLineDashed());

    color = Settings::waypointsDotColor();
    waypointsDotColor->setText(color.name());
    waypointsDotColor->setPalette(QPalette(color));
    waypointsDotSize->setValue(Settings::waypointsDotSize());

    color = Settings::waypointsFontColor();
    waypointsFontColor->setText(color.name());
    waypointsFontColor->setPalette(QPalette(color));
    waypointsFont->setFont(Settings::waypointsFont());

    // updates + feedback
    cbCheckForUpdates->setChecked(Settings::checkForUpdates());
    cbSendVersionInfo->setChecked(Settings::sendVersionInformation());

    // zooming
    sbZoomFactor->setValue(Settings::zoomFactor());
    useSelectionRectangle->setChecked(Settings::useSelectionRectangle());

    //highlight Friends
    sb_highlightFriendsLineWidth->setValue(Settings::highlightLineWidth());
    pb_highlightFriendsColor->setPalette(QPalette(Settings::friendsHighlightColor()));
    pb_highlightFriendsColor->setText(Settings::friendsHighlightColor().name());
    cb_Animation->setChecked(Settings::useHighlightAnimation());

    // FINISHED
    _settingsLoaded = true;
}

// icons: this might be time-consuming because all textures are loaded, but it is
// also very "Qute" ;)
// memory seems to be handled fine and released directly after painting. It
// expands to +40MB though while loading.
void PreferencesDialog::lazyloadTextureIcons() {
    for (int i=0; i < glTextureEarth->count(); i++) {
        if (glTextureEarth->itemIcon(i).isNull()) {
            qDebug() << "PreferencesDialog::lazyloadTextureIcons()" <<
                    Settings::dataDirectory(
                            QString("textures/%1").arg(glTextureEarth->itemText(i)));
            QPixmap *pm = new QPixmap(Settings::dataDirectory(
                    QString("textures/%1").arg(glTextureEarth->itemText(i))));
            QIcon icon = QIcon( // smooth transform uses ~2x the CPU..
                    pm->scaled(128, 64, Qt::KeepAspectRatio, // ..cycles compared to..
                               //Qt::SmoothTransformation)); // fast transform
                               Qt::FastTransformation));
            glTextureEarth->setItemIcon(i, icon);
            delete pm;
            QTimer::singleShot(100, this, SLOT(lazyloadTextureIcons()));
            return;
        }
    }
}

// airport traffic settings
void PreferencesDialog::on_cbFilterTraffic_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setFilterTraffic(state);
}

void PreferencesDialog::on_spFilterDistance_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setFilterDistance(value);
}

void PreferencesDialog::on_spFilterArriving_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setFilterArriving(value);
}

void PreferencesDialog::on_sbCongestionMinimum_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setAirportCongestionMinimum(value);
}

void PreferencesDialog::on_sbMaxTextLabels_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setMaxLabels(value);
}

void PreferencesDialog::on_spinBoxDownloadInterval_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setDownloadInterval(value);
}

void PreferencesDialog::on_cbDownloadPeriodically_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setDownloadPeriodically(state == Qt::Checked);
}

void PreferencesDialog::on_cbUseNavDatabase_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setUseNavdata(state == Qt::Checked);
}

void PreferencesDialog::on_cbResetConfiguration_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setResetOnNextStart(state == Qt::Checked);
}

void PreferencesDialog::on_cbCheckForUpdates_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setCheckForUpdates(state == Qt::Checked);
}

void PreferencesDialog::on_cbSendVersionInfo_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setSendVersionInformation(state == Qt::Checked);
}

void PreferencesDialog::on_cbDownloadOnStartup_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setDownloadOnStartup(state == Qt::Checked);
}

void PreferencesDialog::on_cbNetwork_currentIndexChanged(int index) {
    if (!_settingsLoaded)
        return;
    Settings::setDownloadNetwork(index);

    switch(index) {
    case 0: // VATSIM
        gbDownloadBookings->setChecked(true);
        break;
    case 1: // user defined
        gbDownloadBookings->setChecked(false);
        break;
    }

    editUserDefinedLocation->setEnabled(index == 1);
    lbluserDefinedLocation->setEnabled(index == 1);
}

void PreferencesDialog::on_editUserDefinedLocation_editingFinished() {
    if (!_settingsLoaded)
        return;
    Settings::setUserDownloadLocation(editUserDefinedLocation->text());
    Settings::setDownloadNetwork(Settings::downloadNetwork()); // To force a reload of the status data
}

void PreferencesDialog::on_cbSaveWhazzupData_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setSaveWhazzupData(state == Qt::Checked);
}

void PreferencesDialog::on_groupBoxProxy_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setUseProxy(checked);
}

void PreferencesDialog::on_editProxyServer_editingFinished() {
    if (!_settingsLoaded)
        return;
    Settings::setProxyServer(editProxyServer->text());
}

void PreferencesDialog::on_editProxyPort_editingFinished() {
    if (!_settingsLoaded)
        return;
    Settings::setProxyPort(editProxyPort->text().toInt());
}

void PreferencesDialog::on_editProxyUser_editingFinished() {
    if (!_settingsLoaded)
        return;
    Settings::setProxyUser(editProxyUser->text());
}

void PreferencesDialog::on_editProxyPassword_editingFinished() {
    if (!_settingsLoaded)
        return;
    Settings::setProxyPassword(editProxyPassword->text());
}

void PreferencesDialog::on_spinBoxTimeline_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setTimelineSeconds(value);
}

void PreferencesDialog::on_cbLineSmoothing_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setDisplaySmoothLines(state == Qt::Checked);
}

void PreferencesDialog::on_cbDotSmoothing_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setDisplaySmoothDots(state == Qt::Checked);
}

void PreferencesDialog::on_editNavdir_editingFinished() {
    if (!_settingsLoaded)
        return;
    Settings::setNavdataDirectory(editNavdir->text());
}

void PreferencesDialog::on_browseNavdirButton_clicked() {
    QFileDialog* dialog = new QFileDialog(this, "Select Database Directory",
            Settings::navdataDirectory());
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOption(QFileDialog::ShowDirsOnly);
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
    QColor color = QColorDialog::getColor(Settings::backgroundColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbBackgroundColor->setText(color.name());
        pbBackgroundColor->setPalette(QPalette(color));
        Settings::setBackgroundColor(color);
    }
}

void PreferencesDialog::on_pbGlobeColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::globeColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbGlobeColor->setText(color.name());
        pbGlobeColor->setPalette(QPalette(color));
        Settings::setGlobeColor(color);
    }
}

void PreferencesDialog::on_pbGridLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::gridLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbGridLineColor->setText(color.name());
        pbGridLineColor->setPalette(QPalette(color));
        Settings::setGridLineColor(color);
    }
}

void PreferencesDialog::on_sbGridLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setGridLineStrength(value);
}

void PreferencesDialog::on_pbCountryLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::countryLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbCountryLineColor->setText(color.name());
        pbCountryLineColor->setPalette(QPalette(color));
        Settings::setCountryLineColor(color);
    }
}

void PreferencesDialog::on_sbCountryLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setCountryLineStrength(value);
}

void PreferencesDialog::on_pbCoastLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::coastLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbCoastLineColor->setText(color.name());
        pbCoastLineColor->setPalette(QPalette(color));
        Settings::setCoastLineColor(color);
    }
}

void PreferencesDialog::on_sbCoastLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setCoastLineStrength(value);
}

void PreferencesDialog::on_buttonResetEarthSpace_clicked() {
    QSettings settings;
    settings.beginGroup("earthSpace");
    settings.remove("");
    settings.endGroup();

    settings.beginGroup("gl");
    settings.remove("");
    settings.endGroup();

    loadSettings();
}

void PreferencesDialog::on_pbFirBorderLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::firBorderLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbFirBorderLineColor->setText(color.name());
        pbFirBorderLineColor->setPalette(QPalette(color));
        Settings::setFirBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbFirBorderLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setFirBorderLineStrength(value);
}

void PreferencesDialog::on_pbFirFontColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::firFontColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
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
    QColor color = QColorDialog::getColor(Settings::firFillColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbFirFillColor->setText(color.name());
        pbFirFillColor->setPalette(QPalette(color));
        Settings::setFirFillColor(color);
    }
}

void PreferencesDialog::on_pbFirHighlightedBorderLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::firHighlightedBorderLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbFirHighlightedBorderLineColor->setText(color.name());
        pbFirHighlightedBorderLineColor->setPalette(QPalette(color));
        Settings::setFirHighlightedBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbFirHighlightedBorderLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setFirHighlightedBorderLineStrength(value);
}

void PreferencesDialog::on_pbFirHighlightedFillColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::firHighlightedFillColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbFirHighlightedFillColor->setText(color.name());
        pbFirHighlightedFillColor->setPalette(QPalette(color));
        Settings::setFirHighlightedFillColor(color);
    }
}

void PreferencesDialog::on_buttonResetFir_clicked() {
    QSettings settings;
    settings.beginGroup("firDisplay");
    settings.remove("");
    settings.endGroup();
    loadSettings();
}

void PreferencesDialog::on_pbAirportFontColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::airportFontColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
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
    QColor color = QColorDialog::getColor(Settings::appBorderLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbAppBorderLineColor->setText(color.name());
        pbAppBorderLineColor->setPalette(QPalette(color));
        Settings::setAppBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbAppBorderLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setAppBorderLineStrength(value);
}

void PreferencesDialog::on_pbAppColorCenter_clicked() {
    QColor color = QColorDialog::getColor(Settings::appCenterColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbAppColorCenter->setText(color.name());
        pbAppColorCenter->setPalette(QPalette(color));
        Settings::setAppCenterColor(color);
    }
}

void PreferencesDialog::on_pbAppColorMargin_clicked() {
    QColor color = QColorDialog::getColor(Settings::appMarginColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbAppColorMargin->setText(color.name());
        pbAppColorMargin->setPalette(QPalette(color));
        Settings::setAppMarginColor(color);
    }
}

void PreferencesDialog::on_pbTwrColorCenter_clicked() {
    QColor color = QColorDialog::getColor(Settings::twrCenterColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbTwrColorCenter->setText(color.name());
        pbTwrColorCenter->setPalette(QPalette(color));
        Settings::setTwrCenterColor(color);
    }
}

void PreferencesDialog::on_pbTwrColorMargin_clicked() {
    QColor color = QColorDialog::getColor(Settings::twrMarginColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbTwrColorMargin->setText(color.name());
        pbTwrColorMargin->setPalette(QPalette(color));
        Settings::setTwrMarginColor(color);
    }
}

void PreferencesDialog::on_pbGndBorderLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::gndBorderLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbGndBorderLineColor->setText(color.name());
        pbGndBorderLineColor->setPalette(QPalette(color));
        Settings::setGndBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbGndBorderLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setGndBorderLineStrength(value);
}

void PreferencesDialog::on_pbGndFillColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::gndFillColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbGndFillColor->setText(color.name());
        pbGndFillColor->setPalette(QPalette(color));
        Settings::setGndFillColor(color);
    }
}

void PreferencesDialog::on_pbAirportDotColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::airportDotColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbAirportDotColor->setText(color.name());
        pbAirportDotColor->setPalette(QPalette(color));
        Settings::setAirportDotColor(color);
    }
}

void PreferencesDialog::on_sbAirportDotSize_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setAirportDotSize(value);
}

void PreferencesDialog::on_pbPilotFontColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::pilotFontColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
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
    QColor color = QColorDialog::getColor(Settings::pilotDotColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbPilotDotColor->setText(color.name());
        pbPilotDotColor->setPalette(QPalette(color));
        Settings::setPilotDotColor(color);
    }
}

void PreferencesDialog::on_sbPilotDotSize_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setPilotDotSize(value);
}

void PreferencesDialog::on_pbTimeLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::leaderLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbTimeLineColor->setText(color.name());
        pbTimeLineColor->setPalette(QPalette(color));
        Settings::setLeaderLineColor(color);
    }
}

void PreferencesDialog::on_sbTimeLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setTimeLineStrength(value);
}

void PreferencesDialog::on_pbDepLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::depLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbDepLineColor->setText(color.name());
        pbDepLineColor->setPalette(QPalette(color));
        Settings::setDepLineColor(color);
    }
}
void PreferencesDialog::on_sbDepLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setDepLineStrength(value);
}
void PreferencesDialog::on_pbDestLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::destLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbDestLineColor->setText(color.name());
        pbDestLineColor->setPalette(QPalette(color));
        Settings::setDestLineColor(color);
    }
}
void PreferencesDialog::on_sbDestLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setDestLineStrength(value);
}
void PreferencesDialog::on_cbDepLineDashed_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setDepLineDashed(checked);
}

void PreferencesDialog::on_cbDestLineDashed_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setDestLineDashed(checked);
}

void PreferencesDialog::on_waypointsDotSize_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setWaypointsDotSize(value);
}
void PreferencesDialog::on_waypointsDotColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::waypointsDotColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        waypointsDotColor->setText(color.name());
        waypointsDotColor->setPalette(QPalette(color));
        Settings::setWaypointsDotColor(color);
    }
}
void PreferencesDialog::on_waypointsFontColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::waypointsFontColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        waypointsFontColor->setText(color.name());
        waypointsFontColor->setPalette(QPalette(color));
        Settings::setWaypointsFontColor(color);
    }
}
void PreferencesDialog::on_waypointsFont_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Settings::waypointsFont(), this);
    if(ok) {
        waypointsFont->setFont(font);
        Settings::setWaypointsFont(font);
    }
}

void PreferencesDialog::on_buttonResetPilot_clicked() {
    QSettings settings;
    settings.beginGroup("pilotDisplay");
    settings.remove("");
    settings.endGroup();
    loadSettings();
}

void PreferencesDialog::on_cbShowFixes_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setShowAllWaypoints(checked);
}

void PreferencesDialog::on_pbInactAirportFontColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::inactiveAirportFontColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbInactAirportFontColor->setText(color.name());
        pbInactAirportFontColor->setPalette(QPalette(color));
        Settings::setInactiveAirportFontColor(color);
    }
}

void PreferencesDialog::on_pbInactAirportFont_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Settings::inactiveAirportFont(), this);
    if(ok) {
        pbInactAirportFont->setFont(font);
        Settings::setInactiveAirportFont(font);
    }
}

void PreferencesDialog::on_pbInactAirportDotColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::inactiveAirportDotColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbInactAirportDotColor->setText(color.name());
        pbInactAirportDotColor->setPalette(QPalette(color));
        Settings::setInactiveAirportDotColor(color);
    }
}

void PreferencesDialog::on_sbInactAirportDotSize_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setInactiveAirportDotSize(value);
}

void PreferencesDialog::on_cbShowCongestion_clicked(bool checked) {
    Settings::setAirportCongestion(checked);
}

void PreferencesDialog::on_pbCongestionBorderLineColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::airportCongestionBorderLineColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbCongestionBorderLineColor->setText(color.name());
        pbCongestionBorderLineColor->setPalette(QPalette(color));
        Settings::setAirportCongestionBorderLineColor(color);
    }
}

void PreferencesDialog::on_sbCongestionBorderLineStrength_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setAirportCongestionBorderLineStrength(value);
}

void PreferencesDialog::on_buttonResetAirportTraffic_clicked() {
    QSettings settings;
    settings.beginGroup("airportDisplay");
    settings.remove("");
    settings.endGroup();

    settings.beginGroup("airportTraffic");
    settings.remove("");
    settings.endGroup();

    loadSettings();
}

void PreferencesDialog::on_cbBlend_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setGlBlending(checked);
}

void PreferencesDialog::on_editBookingsLocation_editingFinished() {
    if (!_settingsLoaded)
        return;
    Settings::setBookingsLocation(editBookingsLocation->text());
}

void PreferencesDialog::on_cbBookingsPeriodically_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setBookingsPeriodically(checked);
}

void PreferencesDialog::on_sbBookingsInterval_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setBookingsInterval(value);
}

// zooming
void PreferencesDialog::on_pbWheelCalibrate_clicked() {
    QSettings settings;
    settings.beginGroup("mouseWheel");
    settings.remove("");
    settings.endGroup();

    loadSettings();
}

void PreferencesDialog::on_sbZoomFactor_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setZoomFactor(value);
}

void PreferencesDialog::on_gbDownloadBookings_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setDownloadBookings(checked);
    qobject_cast<Window *>(this->parent())->setEnableBookedAtc(checked);
}


//import & export
void PreferencesDialog::on_pbImportFromFile_clicked() {
    QString fileName = QFileDialog::getOpenFileName(
            this, "Import from File",
            QApplication::applicationDirPath(), "Settings Files (*.ini);; All Files (*.*)"
    );
    if (!fileName.isEmpty()) {
        Settings::importFromFile(fileName);
        QMessageBox::information(this, "Settings loaded. Restart required.",
                                 "QuteScoop closes now. Please restart"
                                 "for the settings to take effect.");
        loadSettings();
        qApp->quit();
    }
}

void PreferencesDialog::on_pbExportToFile_clicked() {
    QString fileName = QFileDialog::getSaveFileName(
            this, "Export to File",
            QApplication::applicationDirPath(), "Settings Files (*.ini);; All Files (*.*)"
    );
    if (!fileName.isEmpty()) {
        Settings::exportToFile(fileName);
        QMessageBox::information(this, "Success", "Settings exported.");
    }
}

void PreferencesDialog::on_cbShootScreenshots_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setShootScreenshots(checked);
}

void PreferencesDialog::on_pbStylesheetUpdate_clicked() {
    qobject_cast<Window *>(this->parent())->setStyleSheet(tedStylesheet->toPlainText());
    Settings::setStylesheet(tedStylesheet->toPlainText());
}

void PreferencesDialog::on_pbStylesheetExample1_clicked() {
    if (
            QMessageBox::question(
                    this, "Overwrite stylesheet?", "Are you sure?",
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

void PreferencesDialog::on_pbStylesheetExample2_clicked() {
    if (
            QMessageBox::question(
                    this, "Overwrite stylesheet?", "Are you sure?",
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

void PreferencesDialog::on_cbLighting_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setEnableLighting(checked);
}

void PreferencesDialog::on_pbSunLightColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::sunLightColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbSunLightColor->setText(color.name());
        pbSunLightColor->setPalette(QPalette(color));
        Settings::setSunLightColor(color);
    }
}

void PreferencesDialog::on_pbSpecularLightColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::specularColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbSpecularLightColor->setText(color.name());
        pbSpecularLightColor->setPalette(QPalette(color));
        Settings::setSpecularColor(color);
    }

}

void PreferencesDialog::on_glLights_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setGlLights(value);
    if (value == 1)
        glLightsSpread->setEnabled(false);
    else
        glLightsSpread->setEnabled(true);
}

void PreferencesDialog::on_glEarthShininess_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setEarthShininess(value);
}

void PreferencesDialog::on_glLightsSpread_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setGlLightsSpread(value);
}

void PreferencesDialog::on_pbReinitOpenGl_clicked() {
    if (Window::instance(false) != 0) {
        Window::instance()->mapScreen->glWidget->initializeGL();
        Window::instance()->mapScreen->glWidget->update();
    }
}

void PreferencesDialog::on_sbEarthGridEach_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setEarthGridEach(value);
}

void PreferencesDialog::on_applyAirports_clicked() {
    if (Window::instance(false) != 0) {
        Window::instance()->mapScreen->glWidget->invalidateAirports();
        Window::instance()->mapScreen->glWidget->invalidateControllers();
    }

}

void PreferencesDialog::on_applyPilots_clicked() {
    qDebug() << "PreferencesDialog::on_applyPilots_clicked()";
    if (Window::instance(false) != 0) {
        Window::instance()->mapScreen->glWidget->invalidatePilots();
        Window::instance()->mapScreen->glWidget->invalidateControllers();
    }
    qDebug() << "PreferencesDialog::on_applyPilots_clicked() -- finished";
}

void PreferencesDialog::on_glStippleLines_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setGlStippleLines(checked);
}

void PreferencesDialog::on_glTextures_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setGlTextures(checked);
}

void PreferencesDialog::on_glTextureEarth_currentIndexChanged(QString tex) {
    if (!_settingsLoaded)
        return;
    Settings::setGlTextureEarth(tex);
}

void PreferencesDialog::on_cbScreenshotMethod_currentIndexChanged(int index) {
    if (!_settingsLoaded)
        return;
    Settings::setScreenshotMethod(index);
}

void PreferencesDialog::on_cbScreenshotFormat_currentIndexChanged(QString value) {
    if (!_settingsLoaded)
        return;
    Settings::setScreenshotFormat(value);
}

void PreferencesDialog::on_useSelectionRectangle_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setUseSelctionRectangle(checked);
}

void PreferencesDialog::on_pbUpperWindColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::windColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        pbUpperWindColor->setText(color.name());
        pbUpperWindColor->setPalette(QPalette(color));
        Settings::setWindColor(color);
        if (SondeData::instance(false) != 0) {
            SondeData::instance()->invalidateWindLists();
            if (Window::instance(false) != 0)
                Window::instance()->mapScreen->glWidget->update();
        }
    }
}

void PreferencesDialog::closeEvent(QCloseEvent *event) {
    Settings::setPreferencesDialogSize(size());
    Settings::setPreferencesDialogPos(pos());
    Settings::setPreferencesDialogGeometry(saveGeometry());
    event->accept();
}

void PreferencesDialog::on_sb_highlightFriendsLineWidth_valueChanged(double value) {
    if (!_settingsLoaded)
        return;
    Settings::setHighlightLineWidth(value);
}

void PreferencesDialog::on_cb_Animation_stateChanged(int state) {
    if (!_settingsLoaded)
        return;
    Settings::setUseHighlightAnimation(state == Qt::Checked);
}

void PreferencesDialog::on_pb_highlightFriendsColor_clicked() {
    QColor color = QColorDialog::getColor(Settings::friendsHighlightColor(), this,
                                          "Select color", QColorDialog::ShowAlphaChannel);

    if(color.isValid()) {
        pb_highlightFriendsColor->setText(color.name());
        pb_highlightFriendsColor->setPalette(QPalette(color));
        Settings::setFriendsHighlightColor(color);
    }
}

void PreferencesDialog::on_cbSimpleLabels_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setSimpleLabels(checked);
}

void PreferencesDialog::on_cbRememberMapPositionOnClose_toggled(bool checked) {
    if (!_settingsLoaded)
        return;
    Settings::setRememberMapPositionOnClose(checked);
}

void PreferencesDialog::on_sbWindSize_valueChanged(int factor) {
    if (!_settingsLoaded)
        return;
    Settings::setWindArrowSize(factor);
    if (SondeData::instance(false) != 0) {
        SondeData::instance()->invalidateWindLists();
        if (Window::instance(false) != 0)
            Window::instance()->mapScreen->glWidget->update();
    }
}

void PreferencesDialog::on_sbWindSecondarySpan_valueChanged(int value) {
    if (!_settingsLoaded)
        return;
    Settings::setSondeAltSecondarySpan_1k(value);
    if (Window::instance(false) != 0)
        Window::instance()->mapScreen->glWidget->update();
}

