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
    if(controllerDetails == 0) {
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::instance();
            controllerDetails = new ControllerDetails(parent);
        }
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
        controllerInfo += details.toHtmlEscaped();

    lblControllerInfo->setText(controllerInfo);

    lblOnline->setText(QString("On %1 for %2 hrs").arg(_controller->server, _controller->onlineTime()));

    if (_controller->sector != 0) {
        lblCallsign->setText(_controller->sector->name.toHtmlEscaped());
    }
    lblCallsign->setVisible(_controller->sector != 0);

    QString frequencyHtml;
    if(!_controller->isObserver() && _controller->frequency.length() != 0) {
        frequencyHtml = QString("<h1><pre>%1</pre></h1>").arg(_controller->frequency);
    }
    lblFrequency->setText(frequencyHtml);

    // airports
    foreach(auto _w, gridAirports->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        _w->disconnect();
        _w->deleteLater();
    }

    delete gridAirportsLayout;
    gridAirportsLayout = new QGridLayout(gridAirports);
    gridAirports->setLayout(gridAirportsLayout);

    int i = 0;
    foreach(auto *_a, _controller->airports()) {
        if (_a == 0) {
            continue;
        }

        auto *_airportPb = new QPushButton(gridAirports);
        _airportPb->setText(_a->toolTip());
        auto _font = _airportPb->font();
        _font.setBold(true);
        _airportPb->setFont(_font);
        connect(
            _airportPb,
            &QPushButton::clicked,
            this,
            [=](bool) {
                _a->showDetailsDialog();
            }
        );
        gridAirportsLayout->addWidget(_airportPb, floor(i / 2), i % 2);
        i++;
    }
    gridAirportsLayout->update();

    // ATIS / controller info
    QString atis = QString("<code style='margin: 50px; padding: 50px'>%1</code>").arg(
        QString(_controller->atisMessage.toHtmlEscaped()).replace("\n", "<br>")
    );
    if (_controller->assumeOnlineUntil.isValid())
        atis += QString("<p><i>QuteScoop assumes from this information that this controller will be online until %1z</i></p>")
            .arg(_controller->assumeOnlineUntil.toString("HHmm"));
    lblAtis->setText(atis);

    gbInfo->setTitle(_controller->label.endsWith("_ATIS")? "ATIS" : "Controller info");

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
}

void ControllerDetails::on_buttonAddFriend_clicked() {
    friendClicked();
    refresh();
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
