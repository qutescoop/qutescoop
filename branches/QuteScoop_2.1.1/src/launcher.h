#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "_pch.h"

class Launcher : public QWidget
{
    Q_OBJECT

public:
    Launcher(QWidget *parent = 0);
    ~Launcher();

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void keyReleaseEvent(QKeyEvent *event);

private:
    QPixmap map;
    QLabel *image;
    QLabel *text;
    //required to move the widget
    QPoint dragPosition;
};

#endif // LAUNCHER_H
