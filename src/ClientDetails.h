/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CLIENTDETAILS_H_
#define CLIENTDETAILS_H_

#include "_pch.h"

#include "MapObject.h"
class MapObject;

class ClientDetails : public QDialog {
        Q_OBJECT

    protected:
        ClientDetails(QWidget *parent);
        virtual void refresh() {}
        void setMapObject(MapObject *object);

    protected slots:
        void showOnMap() const;
        void friendClicked() const;

    protected:
        double _lat, _lon;
        QString userId, callsign;
};

#endif /*CLIENTDETAILS_H_*/
