#ifndef MUSTACHEQS_RENDERER_H
#define MUSTACHEQS_RENDERER_H

#include "src/mustache/external/qt-mustache/mustache.h"

namespace MustacheQs {
    class Renderer {
        public:
            static Renderer* instance();
            // when using this, call teardownContext() to release memory
            static QString render(const QString& _template, QObject* _o);
            static void teardownContext(QObject* _o);
        private:
            Renderer();

            QString m_render(const QString& _template, QObject* _o);
            void m_teardownContext(QObject* _o);
            Mustache::Context* m_context(QObject* o);

            Mustache::Renderer m_renderer;
            QHash<QObject*, Mustache::Context*> m_contexts;
    };
}

#endif
