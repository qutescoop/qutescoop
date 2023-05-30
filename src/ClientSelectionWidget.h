/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef CLIENTSELECTIONWIDGET_H_
#define CLIENTSELECTIONWIDGET_H_

#include "MapObject.h"

#include <QFocusEvent>
#include <QListWidget>

class ClientSelectionWidget : public QListWidget {
        Q_OBJECT
    public:
        explicit ClientSelectionWidget(QWidget *parent = 0);
        ~ClientSelectionWidget() {}

        void setObjects(QList<MapObject*> objects);
        void clearObjects();

        virtual QSize sizeHint () const;

    public slots:
        void dialogForItem(QListWidgetItem *item);

    protected:
        void focusOutEvent(QFocusEvent *event);

    private:
        QList<MapObject*> _displayClients;
};

#endif
