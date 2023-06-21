#include "Renderer.h"

#include "src/Airport.h"

namespace MustacheQs {
    Renderer* renderer = nullptr;
    Renderer* Renderer::instance() {
        if (renderer == nullptr) {
            renderer = new Renderer();
        }
        return renderer;
    }

    QString Renderer::render(const QString &_template, QObject* _o) {
        return instance()->m_render(_template, _o);
    }

    void Renderer::teardownContext(QObject* _o) {
        instance()->m_teardownContext(_o);
    }

    Renderer::Renderer() {
        m_renderer = Mustache::Renderer();
        // we only use single braces
        m_renderer.setTagMarkers("{", "}");
    }

    QString Renderer::m_render(const QString &_template, QObject* _o) {
        return m_renderer.render(_template, m_context(_o));
    }

    void Renderer::m_teardownContext(QObject* _o) {
        if (!m_contexts.contains(_o)) {
            return;
        }

        auto context = m_contexts.take(_o);
        delete context;
    }

    Mustache::Context* Renderer::m_context(QObject* _o) {
        if (m_contexts.contains(_o)) {
            return m_contexts.value(_o);
        }

        Mustache::Context* context;

        ::Airport* a = qobject_cast<::Airport*>(_o);
        if (a != 0) {
            context = new MustacheQs::Airport::Context(a);
        } else {
            ::Controller* c = qobject_cast<::Controller*>(_o);
            if (c != 0) {
                context = new MustacheQs::Controller::Context(c);
            } else {
                ::Pilot* p = qobject_cast<::Pilot*>(_o);
                if (p != 0) {
                    context = new MustacheQs::Pilot::Context(p);
                } else {
                    context = new Mustache::QtVariantContext((QVariantHash()));
                }
            }
        }
        m_contexts.insert(_o, context);
        return context;
    }
}
