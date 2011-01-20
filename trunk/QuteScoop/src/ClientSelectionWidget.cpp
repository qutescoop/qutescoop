/**************************************************************************
 *  This file is part of QuteScoop. See README for license
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
    //listWidget->grabKeyboard(); // gives GTK+ errors in Gnome
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
            //listWidget->releaseKeyboard();
            close();
            return;
        }
    }
}

void ClientSelectionWidget::focusOutEvent(QFocusEvent *event) {
    if(event->reason() != Qt::MouseFocusReason) {
        timerDefault.stop();
        //listWidget->releaseKeyboard();
        close();
    }
}

void ClientSelectionWidget::timerDefaultTriggered() {
    progressBar->setValue(progressBar->value() - 2);
    if(progressBar->value() < 5) {
        timerDefault.stop();
        progressBar->setValue(100);
        //listWidget->releaseKeyboard();
        close();
        displayClients[0]->showDetailsDialog();
        return;
    }
}

void ClientSelectionWidget::userActs() {
    timerDefault.stop();
    progressBar->setValue(100);
    progressBar->setVisible(false);
}
