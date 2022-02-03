/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ControllerDetails.h"

#include "Window.h"
#include "Settings.h"
#include "Whazzup.h"

//singleton instance
ControllerDetails *controllerDetails = 0;
ControllerDetails* ControllerDetails::instance(bool createIfNoInstance, QWidget *parent) {
    if(controllerDetails == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::instance();
            controllerDetails = new ControllerDetails(parent);
        }
    return controllerDetails;
}

// destroys a singleton instance
void ControllerDetails::destroyInstance() {
    delete controllerDetails;
    controllerDetails = 0;
}

ControllerDetails::ControllerDetails(QWidget *parent):
        ClientDetails(parent),
        _controller(0) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);

    connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
}

void ControllerDetails::refresh(Controller *newController) {
    if (!Settings::controllerDetailsSize().isNull()) resize(Settings::controllerDetailsSize());
    if (!Settings::controllerDetailsPos().isNull()) move(Settings::controllerDetailsPos());
    if (!Settings::controllerDetailsGeometry().isNull()) restoreGeometry(Settings::controllerDetailsGeometry());

    if(newController != 0)
        _controller = newController;
    else
        _controller = Whazzup::instance()->whazzupData().controllers[callsign];
    if(_controller == 0)
        return;
    setMapObject(_controller);
    setWindowTitle(_controller->toolTip());

    // Controller Information
    QString controllerInfo = QString("<strong>%1</strong><br>").arg(_controller->displayName(true));

    QString details = _controller->detailInformation();
    if(!details.isEmpty())
        controllerInfo += details + "<br>";

    controllerInfo += QString("on %3 for %4")
                      .arg(_controller->server)
                      .arg(_controller->onlineTime());
    if(!_controller->clientInformation().isEmpty())
        controllerInfo += ", " + _controller->clientInformation();
    lblControllerInfo->setText(controllerInfo);

    QString stationInfo = _controller->facilityString();
    if(!_controller->isObserver() && _controller->frequency.length() != 0)
        stationInfo += QString(" on frequency %1")
                .arg(_controller->frequency);
    lblStationInformatoin->setText(stationInfo);

    pbAirportDetails->setVisible(_controller->airport() != 0);
    pbAirportDetails->setText(   _controller->airport() != 0? _controller->airport()->toolTip(): "");

    QString atis = _controller->atisMessage;
    if (_controller->assumeOnlineUntil.isValid())
        atis += QString("<p><i>QuteScoop assumes from this information that this controller will be online until %1z</i></p>")
            .arg(_controller->assumeOnlineUntil.toString("HHmm"));
    lblAtis->setText(atis);
    lblAtis->adjustSize(); // ensure auto-resize

    gbAtis->setTitle(_controller->label.endsWith("_ATIS")? "ATIS" : "Controller info");

    if(_controller->isFriend())
        buttonAddFriend->setText("remove &friend");
    else
        buttonAddFriend->setText("add &friend");

    // check if we know UserId
    buttonAddFriend->setDisabled(_controller->userId.isEmpty());

    // check if we know position
    buttonShowOnMap->setDisabled(qFuzzyIsNull(_controller->lat) && qFuzzyIsNull(_controller->lon));

    adjustSize();
}

void ControllerDetails::on_buttonAddFriend_clicked() {
    friendClicked();
    refresh();
}

void ControllerDetails::on_pbAirportDetails_clicked() {
    if (_controller->airport() != 0)
        _controller->airport()->showDetailsDialog();
}

void ControllerDetails::closeEvent(QCloseEvent *event) {
    Settings::setControllerDetailsPos(pos());
    Settings::setControllerDetailsSize(size());
    Settings::setControllerDetailsGeometry(saveGeometry());
    event->accept();
}
