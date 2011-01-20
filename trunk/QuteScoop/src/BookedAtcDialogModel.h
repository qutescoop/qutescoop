/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef BOOKEDATCDIALOGMODEL_H_
#define BOOKEDATCDIALOGMODEL_H_

#include <QAbstractTableModel>
#include <QList>
#include "BookedController.h"

class BookedAtcDialogModel : public QAbstractTableModel {
	Q_OBJECT

public:
	BookedAtcDialogModel(QObject *parent = 0) : QAbstractTableModel(parent) {}

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation,
	                         int role = Qt::DisplayRole) const;
	
public slots:
	void setClients(const QList<BookedController*>& controllers);
	void modelSelected(const QModelIndex& index);
  
private:
	QList<BookedController*> controllers;
};

#endif /*BOOKEDATCDIALOGMODEL_H_*/
