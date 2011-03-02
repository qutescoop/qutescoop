/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Client.h"

#include "ClientSelectionWidget.h"

ClientSelectionWidget::ClientSelectionWidget(QWidget *parent):
    QDialog(parent)
{
    setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setFocusPolicy(Qt::StrongFocus);
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(dialogForItem(QListWidgetItem*)));
    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(dialogForItem(QListWidgetItem*)));
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
    //listWidget->grabKeyboard(); // gives GTK+ errors in Gnome
}

void ClientSelectionWidget::clearClients() {
    listWidget->clear();
    displayClients.clear();
}

void ClientSelectionWidget::dialogForItem(QListWidgetItem *item) {
    for(int i = 0; i < displayClients.size(); i++) {
        if(item->text() == displayClients[i]->toolTip()) {
            displayClients[i]->showDetailsDialog();
            //listWidget->releaseKeyboard();
            close();
            return;
        }
    }
}

void ClientSelectionWidget::focusOutEvent(QFocusEvent *event) {
    if(event->reason() != Qt::MouseFocusReason) {
        //listWidget->releaseKeyboard();
        close();
    }
}