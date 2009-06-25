/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
 **************************************************************************/

#include <QFocusEvent>
#include <QListWidgetItem>
#include "ClientSelectionWidget.h"

ClientSelectionWidget::ClientSelectionWidget(QWidget *parent):
	QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setFocusPolicy(Qt::StrongFocus);
	connect(listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(dialogForItem(QListWidgetItem*)));
}

void ClientSelectionWidget::setObjects(QList<MapObject*> objects) {
	clearClients();
	displayClients = objects;
	for(int i = 0; i < objects.size(); i++) {
		listWidget->addItem(objects[i]->toolTip());
	}
}

void ClientSelectionWidget::clearClients() {
	listWidget->clear();
	displayClients.clear();
}

void ClientSelectionWidget::dialogForItem(QListWidgetItem *item) {
	for(int i = 0; i < displayClients.size(); i++) {
		if(item->text() == displayClients[i]->toolTip()) {
			displayClients[i]->showDetailsDialog();
			close();
			return;
		}
	}
}

void ClientSelectionWidget::focusOutEvent(QFocusEvent *event) {
	if(event->reason() != Qt::MouseFocusReason)
		close();
}
