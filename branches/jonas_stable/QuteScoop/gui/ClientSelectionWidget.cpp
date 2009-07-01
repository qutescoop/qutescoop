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
#include <QDebug>
#include "ClientSelectionWidget.h"

ClientSelectionWidget::ClientSelectionWidget(QWidget *parent):
	QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setFocusPolicy(Qt::StrongFocus);
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(dialogForItem(QListWidgetItem*)));
    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(dialogForItem(QListWidgetItem*)));

    connect(listWidget, SIGNAL(viewportEntered()), this, SLOT(userActs()));
    connect(listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(userActs())); // for keyboard interaction
    connect(listWidget, SIGNAL(entered(QModelIndex)), this, SLOT(userActs()));

    connect(&timerDefault, SIGNAL(timeout()), this, SLOT(timerDefaultTriggered()));
}

void ClientSelectionWidget::setObjects(QList<MapObject*> objects) {
	clearClients();
	displayClients = objects;
	for(int i = 0; i < objects.size(); i++) {
        if(i == 0) {
            QListWidgetItem* lwi = new QListWidgetItem(objects[i]->toolTip(), listWidget);
            QFont font = QFont();
            font.setBold(true);
            lwi->setFont(font);
            listWidget->addItem(lwi);
        } else
            listWidget->addItem(objects[i]->toolTip());
    }
    listWidget->setCurrentRow(0);
    listWidget->setFocus();
    listWidget->grabKeyboard();
    progressBar->setValue(100);
    progressBar->setVisible(true);
    timerDefault.start(30);
}

void ClientSelectionWidget::clearClients() {
	listWidget->clear();
	displayClients.clear();
}

void ClientSelectionWidget::dialogForItem(QListWidgetItem *item) {
	for(int i = 0; i < displayClients.size(); i++) {
		if(item->text() == displayClients[i]->toolTip()) {
			displayClients[i]->showDetailsDialog();
            listWidget->releaseKeyboard();
            close();
			return;
		}
	}
}

void ClientSelectionWidget::focusOutEvent(QFocusEvent *event) {
    if(event->reason() != Qt::MouseFocusReason) {
        timerDefault.stop();
        listWidget->releaseKeyboard();
        close();
    }
}

void ClientSelectionWidget::timerDefaultTriggered() {
    progressBar->setValue(progressBar->value() - 2);
    if(progressBar->value() < 5) {
        timerDefault.stop();
        listWidget->releaseKeyboard();
        displayClients[0]->showDetailsDialog();
        close();
        return;
    }
}

void ClientSelectionWidget::userActs() {
    timerDefault.stop();
    progressBar->setValue(100);
    progressBar->setVisible(false);
}
