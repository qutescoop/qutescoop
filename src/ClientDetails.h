/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CLIENTDETAILS_H_
#define CLIENTDETAILS_H_

#include "MapObject.h"

#include <QDialog>

class Client;
class MapObject;

class ClientDetails : public QDialog {
        Q_OBJECT

    public slots:
        void showOnMap() const;
        void friendClicked() const;

    protected:
        ClientDetails(QWidget*);
        void setMapObject(MapObject*);

    protected:
        double _lat, _lon;
        QString userId, callsign;
};

#endif /*CLIENTDETAILS_H_*/
