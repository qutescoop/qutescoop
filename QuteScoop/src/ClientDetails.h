/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CLIENTDETAILS_H_
#define CLIENTDETAILS_H_

#include "_pch.h"

#include "MapObject.h"

class MapObject;

class ClientDetails : public QDialog
{
    Q_OBJECT

public:
    virtual void refresh() {}

signals:
    void showOnMap(double lat, double lon);

protected:
    ClientDetails(QWidget *parent);
    void setMapObject(MapObject *object);

protected slots:
    void showOnMap();
    void friendClicked();

protected:
    double lat, lon;
    QString userId, callsign;
};

#endif /*CLIENTDETAILS_H_*/
