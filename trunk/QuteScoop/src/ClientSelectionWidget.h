/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CLIENTSELECTIONWIDGET_H_
#define CLIENTSELECTIONWIDGET_H_

#include "ui_ClientSelectionDialog.h"

#include "_pch.h"

#include "MapObject.h"

class ClientSelectionWidget : public QDialog, private Ui::ClientSelectionDialog
{
	Q_OBJECT

public:
	ClientSelectionWidget(QWidget *parent = 0);

	void setObjects(QList<MapObject*> objects);
	void clearClients();

public slots:
	void dialogForItem(QListWidgetItem *item);

protected:
	void focusOutEvent(QFocusEvent *event);

private:
	QList<MapObject*> displayClients;
};

#endif
