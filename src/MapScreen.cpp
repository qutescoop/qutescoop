#include "MapScreen.h"

static MapScreen* mapScreenInstance = 0;
MapScreen* MapScreen::instance(bool createIfNoInstance) {
    if (mapScreenInstance == 0 && createIfNoInstance) {
        mapScreenInstance = new MapScreen;
    }
    return mapScreenInstance;
}

MapScreen::MapScreen(QWidget* parent)
    : QWidget(parent) {

    //OpenGL config
    QSettings* settings = new QSettings();

    QGLFormat fmt;
    fmt.setDirectRendering(settings->value("gl/directrendering", fmt.defaultFormat().directRendering()).toBool());

    fmt.setDoubleBuffer(settings->value("gl/doublebuffer", fmt.defaultFormat().doubleBuffer()).toBool());

    fmt.setStencil(settings->value("gl/stencilbuffer", fmt.defaultFormat().stencil()).toBool());

    if (fmt.defaultFormat().stencilBufferSize() > 0) {
        fmt.setStencilBufferSize(settings->value("gl/stencilsize", fmt.defaultFormat().stencilBufferSize()).toInt());
    }

    fmt.setDepth(settings->value("gl/depthbuffer", fmt.defaultFormat().depth()).toBool());

    if (fmt.defaultFormat().depthBufferSize() > 0) {
        fmt.setDepthBufferSize(settings->value("gl/depthsize", fmt.defaultFormat().depthBufferSize()).toInt());
    }

    fmt.setAlpha(settings->value("gl/alphabuffer", fmt.defaultFormat().alpha()).toBool());

    if (fmt.defaultFormat().alphaBufferSize() > 0) {
        fmt.setAlphaBufferSize(settings->value("gl/alphasize", fmt.defaultFormat().alphaBufferSize()).toInt());
    }

    fmt.setAccum(settings->value("gl/accumbuffer", fmt.defaultFormat().accum()).toBool());
    if (fmt.defaultFormat().accumBufferSize() > 0) {
        fmt.setAccumBufferSize(settings->value("gl/accumsize", fmt.defaultFormat().accumBufferSize()).toInt());
    }

    fmt.setRgba(true);

    // Anti-aliasing (4xMSAA by default):
    const auto sampleBuffers = settings->value("gl/samplebuffers", true).toBool();
    if (sampleBuffers) {
        const auto nSamples = settings->value("gl/samples", 4).toInt();
        fmt.setSamples(nSamples);

        qDebug() << "GLFormat: MSAA: Requesting multi-sample anti-aliasing with" << nSamples << "sample buffers";
    }
    fmt.setSampleBuffers(sampleBuffers);


    qDebug() << "creating GLWidget";
    glWidget = new GLWidget(fmt, this);
    QGridLayout* layout = new QGridLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMargin(0);
    layout->addWidget(glWidget);
    setLayout(layout);
    qDebug() << "creating GLWidget --finished";
}

void MapScreen::resizeEvent(QResizeEvent*) {
    glWidget->resize(this->width(), this->height());
}
