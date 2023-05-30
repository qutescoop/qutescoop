#include "ClientSelectionWidget.h"

ClientSelectionWidget::ClientSelectionWidget(QWidget *parent):
        QListWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint);
    setFocusPolicy(Qt::StrongFocus);
    setFrameStyle(QFrame::NoFrame);
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, &QListWidget::itemClicked,   this, &ClientSelectionWidget::dialogForItem);
    connect(this, &QListWidget::itemActivated, this, &ClientSelectionWidget::dialogForItem);
}

void ClientSelectionWidget::setObjects(QList<MapObject*> objects) {
    clearObjects();
    _displayClients = objects;
    for(int i = 0; i < objects.size(); i++)
        addItem(objects[i]->toolTip());
    setCurrentRow(0);
    show();
    resize(sizeHint());
    raise();
    setFocus();
}

void ClientSelectionWidget::clearObjects() {
    clear();
    _displayClients.clear();
}

void ClientSelectionWidget::dialogForItem(QListWidgetItem *item) {
    foreach(MapObject *m, _displayClients) {
        if(item->text() == m->toolTip()) {
            m->showDetailsDialog();
            close();
            return;
        }
    }
}

void ClientSelectionWidget::focusOutEvent(QFocusEvent *event) {
    if(event->reason() != Qt::MouseFocusReason)
        close();
}

QSize ClientSelectionWidget::sizeHint() const {
    return contentsSize();
}
