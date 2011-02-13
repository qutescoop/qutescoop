/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ControllerDetails.h"

#include "Window.h"
#include "Settings.h"

//singleton instance
ControllerDetails *controllerDetails = 0;
ControllerDetails* ControllerDetails::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(controllerDetails == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::getInstance(true);
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
    controller(0)
{
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
//    setWindowFlags(Qt::Tool);

    connect(buttonShowOnMap, SIGNAL(clicked()), this, SLOT(showOnMap()));
    connect(this, SIGNAL(showOnMap(double, double)), qobject_cast<Window *>(this->parent()), SLOT(showOnMap(double, double)));
}

void ControllerDetails::refresh(Controller *newController) {
    if(newController != 0) {
        controller = newController;
    } else {
        controller = Whazzup::getInstance()->whazzupData().getController(callsign);
    }
    if(controller == 0) return;
    setMapObject(controller);

    setWindowTitle(controller->label);

    // Controller Information
    QString controllerInfo = QString("<strong>ATC: %1</strong><br>").arg(controller->displayName(true));

    QString details = controller->detailInformation();
    if(!details.isEmpty()) controllerInfo += details + "<br>";

    controllerInfo += QString("On %3 for %4")
                      .arg(controller->server)
                      .arg(controller->onlineTime());
    QString software = controller->clientInformation();
    if(!software.isEmpty()) controllerInfo += ", " + software;
    lblControllerInfo->setText(controllerInfo);

    QString stationInfo = controller->facilityString();
    if(!controller->isObserver() && !controller->frequency.length() == 0)
        stationInfo += QString(" on frequency %1<br>Voice: %2")
        .arg(controller->frequency)
        .arg(controller->voiceServer);
    lblStationInformatoin->setText(stationInfo);

    QString atis = controller->atisMessage;
    if (controller->assumeOnlineUntil.isValid())
        atis += QString("<p><i>QuteScoop assumes from this information that this controller will be online until %1z</i></p>")
            .arg(controller->assumeOnlineUntil.toString("HHmm"));

    lblAtis->setText(atis);

    if(controller->isFriend())
        buttonAddFriend->setText("Remove Friend");
    else
        buttonAddFriend->setText("Add Friend");

    btnJoinChannel->setVisible(Settings::voiceType() != Settings::NONE);
    btnJoinChannel->setEnabled(!controller->voiceLink().isEmpty());

    // check if we know UserId
    buttonAddFriend->setDisabled(controller->userId.isEmpty());

    // check if we know position
    buttonShowOnMap->setDisabled(controller->lat == 0 && controller->lon == 0);
}

void ControllerDetails::on_buttonAddFriend_clicked() {
    friendClicked();
    refresh();
}

void ControllerDetails::on_btnJoinChannel_clicked() {
    if(controller == 0) return;
    if(Settings::voiceType() == Settings::NONE) return;

    if(Settings::voiceUser().isEmpty()) {
        QMessageBox::information(this, tr("Voice Settings"),
                "You have to enter your network user ID and password in the preferences before you can join a TeamSpeak channel.");
        return;
    }

#ifdef Q_WS_WIN
    QString program = "start";
#endif
#ifdef Q_WS_MAC
    QString program = "open";
#endif
#ifdef Q_WS_X11
    QString program = "xdg-open";
#endif

    QString command = program + " " + controller->voiceLink();
    int ret = system(command.toAscii());
    qDebug() << program << "returned" << ret;
}
