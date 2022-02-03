#include "MapScreen.h"

static MapScreen *mapScreenInstance = 0;
MapScreen* MapScreen::instance(bool createIfNoInstance) {
    if(mapScreenInstance == 0 && createIfNoInstance)
        mapScreenInstance = new MapScreen;
    return mapScreenInstance;
}

MapScreen::MapScreen(QWidget *parent) :
        QWidget(parent) {

    //OpenGL config
    QSettings* settings = new QSettings();

    QGLFormat fmt;
    fmt.setDirectRendering(settings->value("gl/directrendering", fmt.defaultFormat().directRendering()).toBool());
    fmt.setDoubleBuffer(settings->value("gl/doublebuffer", fmt.defaultFormat().doubleBuffer()).toBool());
    fmt.setStencil(settings->value("gl/stencilbuffer", fmt.defaultFormat().stencil()).toBool());
    if (fmt.defaultFormat().stencilBufferSize() > 0)
        fmt.setStencilBufferSize(settings->value("gl/stencilsize", fmt.defaultFormat().stencilBufferSize()).toInt());
    fmt.setDepth(settings->value("gl/depthbuffer", fmt.defaultFormat().depth()).toBool());
    if (fmt.defaultFormat().depthBufferSize() > 0)
        fmt.setDepthBufferSize(settings->value("gl/depthsize", fmt.defaultFormat().depthBufferSize()).toInt());
    fmt.setAlpha(settings->value("gl/alphabuffer", fmt.defaultFormat().alpha()).toBool());
    if (fmt.defaultFormat().alphaBufferSize() > 0)
        fmt.setAlphaBufferSize(settings->value("gl/alphasize", fmt.defaultFormat().alphaBufferSize()).toInt());
    fmt.setSampleBuffers(settings->value("gl/samplebuffers", fmt.defaultFormat().sampleBuffers()).toBool());
    if (fmt.defaultFormat().samples() > 0)
        fmt.setSamples(settings->value("gl/samples", fmt.defaultFormat().samples()).toInt());
    fmt.setAccum(settings->value("gl/accumbuffer", fmt.defaultFormat().accum()).toBool());
    if (fmt.defaultFormat().accumBufferSize() > 0)
        fmt.setAccumBufferSize(settings->value("gl/accumsize", fmt.defaultFormat().accumBufferSize()).toInt());
    fmt.setRgba(true);

    qDebug() << "MapScreen::MapScreen() creating GLWidget";
    glWidget = new GLWidget(fmt, this);
    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMargin(0);
    layout->addWidget(glWidget);
    setLayout(layout);
    qDebug() << "MapScreen::MapScreen() creating GLWidget --finished";
}

void MapScreen::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    glWidget->resize(this->width(),this->height());
}

