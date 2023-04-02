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

    connect(buttonShowOnMap, &QAbstractButton::clicked, this, &ClientDetails::showOnMap);

    if (!Settings::controllerDetailsSize().isNull()) resize(Settings::controllerDetailsSize());
    if (!Settings::controllerDetailsPos().isNull()) move(Settings::controllerDetailsPos());
    if (!Settings::controllerDetailsGeometry().isNull()) restoreGeometry(Settings::controllerDetailsGeometry());
}

void ControllerDetails::refresh(Controller *newController) {
    if(newController != 0)
        _controller = newController;
    else
        _controller = Whazzup::instance()->whazzupData().controllers[callsign];
    if(_controller == 0) {
        close();
        return;
    }
    setMapObject(_controller);
    setWindowTitle(_controller->toolTipShort());

    // Controller Information
    QString controllerInfo = QString("<strong>%1</strong>").arg(_controller->displayName(true));

    QString details = _controller->detailInformation();
    if(!details.isEmpty())
        controllerInfo += details;

    lblControllerInfo->setText(controllerInfo);

    lblOnline->setText(QString("On %1 for %2 hrs").arg(_controller->server, _controller->onlineTime()));

    if (_controller->sector != 0) {
        lblCallsign->setText(_controller->sector->name);
    }
    lblCallsign->setVisible(_controller->sector != 0);

    QString frequencyHtml;
    if(!_controller->isObserver() && _controller->frequency.length() != 0) {
        frequencyHtml = QString("<h1><pre>%1</pre></h1>").arg(_controller->frequency);
    }
    lblFrequency->setText(frequencyHtml);

    pbAirportDetails->setVisible(_controller->airport() != 0);
    pbAirportDetails->setText(   _controller->airport() != 0? _controller->airport()->toolTip(): "");

    QString atis = QString("<code style='margin: 50px; padding: 50px'>%1</code>").arg(
        QString(_controller->atisMessage.toHtmlEscaped()).replace("\n", "<br>")
    );
    if (_controller->assumeOnlineUntil.isValid())
        atis += QString("<p><i>QuteScoop assumes from this information that this controller will be online until %1z</i></p>")
            .arg(_controller->assumeOnlineUntil.toString("HHmm"));
    lblAtis->setText(atis);
    // This does not seem to be necessary. Changing the content changes the size hint and triggers a window resize.
    // lblAtis->adjustSize(); // ensure auto-resize

    gbAtis->setTitle(_controller->label.endsWith("_ATIS")? "ATIS" : "Controller info");

    if(_controller->isFriend())
        buttonAddFriend->setText("remove &friend");
    else
        buttonAddFriend->setText("add &friend");

    // check if we know UserId
    bool invalidID = !(_controller->hasValidID());
    buttonAddFriend->setDisabled(invalidID);
    pbAlias->setDisabled(invalidID);

    // enable if we know position
    buttonShowOnMap->setDisabled(qFuzzyIsNull(_controller->lat) && qFuzzyIsNull(_controller->lon));

    // @see https://github.com/qutescoop/qutescoop/issues/124
    // adjustSize();
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

void ControllerDetails::on_pbAlias_clicked()
{
    if (_controller->showAliasDialog(this)) {
        refresh();
    }
}

