#ifndef CONTROLLERDETAILS_H_
#define CONTROLLERDETAILS_H_

#include "ui_ControllerDetails.h"

#include "ClientDetails.h"
#include "../Controller.h"

class ControllerDetails: public ClientDetails, private Ui::ControllerDetails {
    Q_OBJECT
    public:
        static ControllerDetails* instance(bool createIfNoInstance = true, QWidget* parent = 0);
        void destroyInstance();
        void refresh(Controller* = 0);
    protected:
        void closeEvent(QCloseEvent* event);
    private slots:
        void on_buttonAddFriend_clicked();

        // @todo move to ClientDetails
        void on_pbAlias_clicked();

    private:
        ControllerDetails(QWidget* parent);
        Controller* _controller;

        constexpr static char m_preferencesName[] = "controllerDetails";
};

#endif /*CONTROLLERDETAILS_H_*/
