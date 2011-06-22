/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CONTROLLERDETAILS_H_
#define CONTROLLERDETAILS_H_

#include "ui_ControllerDetails.h"

#include "_pch.h"

#include "ClientDetails.h"
#include "Controller.h"

class ControllerDetails: public ClientDetails, private Ui::ControllerDetails
{
    Q_OBJECT

public:
    static ControllerDetails* getInstance(bool createIfNoInstance = true, QWidget *parent = 0);
    void destroyInstance();
    void refresh(Controller* controller = 0);

private slots:
    void on_buttonAddFriend_clicked();
    void on_btnJoinChannel_clicked();

    void on_pbAirportDetails_clicked();

private:
    ControllerDetails(QWidget *parent);
    Controller* controller;
};

#endif /*CONTROLLERDETAILS_H_*/