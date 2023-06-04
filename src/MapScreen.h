#ifndef MAPSCREEN_H
#define MAPSCREEN_H

#include "GLWidget.h"

class MapScreen
    : public QWidget {
    Q_OBJECT

    public:
        MapScreen(QWidget* parent = 0);
        GLWidget* glWidget;
        static MapScreen* instance(bool createIfNoInstance = true);
    protected:
        void resizeEvent(QResizeEvent* event);
};

#endif // MAPSCREEN_H
