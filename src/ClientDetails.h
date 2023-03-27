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

    protected:
        ClientDetails(QWidget*);
        void setMapObject(MapObject*);

    protected slots:
        void showOnMap() const;
        void friendClicked() const;

    protected:
        double _lat, _lon;
        QString userId, callsign;
};

#endif /*CLIENTDETAILS_H_*/
