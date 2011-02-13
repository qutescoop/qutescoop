/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef AIRPORTDETAILSARRIVALSMODEL_H_
#define AIRPORTDETAILSARRIVALSMODEL_H_

#include "_pch.h"

#include "Pilot.h"

class AirportDetailsArrivalsModel : public QAbstractTableModel {
	Q_OBJECT

public:
	AirportDetailsArrivalsModel(QObject *parent = 0) : QAbstractTableModel(parent) {}

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation,
	                         int role = Qt::DisplayRole) const;
	
public slots:
	void setClients(const QList<Pilot*>& pilots);
	void modelSelected(const QModelIndex& index);

private:
	QList<Pilot*> pilots;
};

#endif /*AIRPORTDETAILSARRIVALSMODEL_H_*/
