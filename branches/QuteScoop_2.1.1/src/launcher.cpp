#include "launcher.h"

Launcher::Launcher(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
    map = QPixmap(":label/qs-logo.png").scaled(600,600);
    qDebug() << "isNull:" << map.isNull();
    qDebug() << "hasAlphaChannel:" << map.hasAlphaChannel();
    qDebug() << "w:" << map.width() << " h:" << map.height();

    image = new QLabel(this);
    text = new QLabel(this);

    image->setPixmap(map);
    image->resize(map.width(), map.height());

    resize(map.width(), map.height());

    text->setText("TEST -- loading  asdf asdf as on na noasndoin a noasndo nao oainsd nfoia sn onodn onsadon foiansoi nasdn oiansodi naslkn oiansd onfoasnd oiaosn oiansdl fnsodfn ");
    text->setAlignment(Qt::AlignCenter);
    text->setWordWrap(true);
    text->resize(440, 30);
    qDebug() << "text frames w:" << text->frameSize() << " text h:" << text->height();
    text->move((map.width()/2)-220, (map.height()/3)*2 +30);

    image->lower();
    text->raise();

    setMask(map.mask());
    qDebug() << "Widget w:" << this->width() << " Widget h:" << this->height();

}

Launcher::~Launcher()
{

}

void Launcher::keyReleaseEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Escape){
        event->accept();
        QApplication::instance()->quit();
    }
}

void Launcher::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
             dragPosition = event->globalPos() - frameGeometry().topLeft();
             event->accept();
         }
}

void Launcher::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
             move(event->globalPos() - dragPosition);
             event->accept();
         }
}
