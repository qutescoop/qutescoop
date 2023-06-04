#ifndef CLIENTSELECTIONWIDGET_H_
#define CLIENTSELECTIONWIDGET_H_

#include "MapObject.h"

#include <QFocusEvent>
#include <QListWidget>

class ClientSelectionWidget
    : public QListWidget {
    Q_OBJECT
    public:
        explicit ClientSelectionWidget(QWidget* parent = 0);
        virtual ~ClientSelectionWidget();

        void setObjects(QList<MapObject*> objects);
        void clearObjects();

        virtual QSize sizeHint() const override;

    public slots:
        void dialogForItem(QListWidgetItem* item);

    protected:
        void focusOutEvent(QFocusEvent* event) override;

    private:
        QList<MapObject*> _displayClients;
};

#endif
