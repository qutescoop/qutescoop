#include "ControllerContext.h"

#include "src/Controller.h"

namespace MustacheQs::Controller {
    PartialResolver* PartialResolver::inst = nullptr;
    PartialResolver* PartialResolver::instance() {
        if (inst == nullptr) {
            inst = new PartialResolver();
        }
        return inst;
    }

    QString PartialResolver::getPartial(const QString &name) {
        return QString("[> %1]").arg(name);
    }

    PartialResolver::PartialResolver() {}

    Context::Context(const class Controller* a)
        : Mustache::Context(PartialResolver::instance()), m_o(a) {}

    Context::~Context() {}

    QString Context::stringValue(const QString &key) const {
        if (key == "sectorOrLogin") {
            if (m_o->sector != 0) {
                return m_o->controllerSectorName();
            }

            return m_o->callsign;
        }
        if (key == "sector") {
            if (m_o->sector != 0) {
                return m_o->sector->name;
            }

            return "";
        }
        if (key == "name") {
            return m_o->aliasOrName();
        }
        if (key == "nameIfFriend") {
            return m_o->isFriend()? m_o->aliasOrName(): "";
        }
        if (key == "rating") {
            return m_o->rank();
        }
        if (key == "frequency") {
            return m_o->frequency.length() > 1? m_o->frequency: "";
        }
        if (key == "cpdlc") {
            return m_o->cpdlcString();
        }
        if (key == "livestream") {
            return m_o->livestreamString();
        }

        return QString("{%1}").arg(key);
    }

    bool Context::isFalse(const QString &key) const {
        return stringValue(key).isEmpty();
    }

    int Context::listCount(const QString&) const {
        return 0;
    }

    void Context::push(const QString&, int) {}

    void Context::pop() {}
}
