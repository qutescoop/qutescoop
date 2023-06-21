#include "AirportContext.h"

#include "src/Airport.h"

namespace MustacheQs::Airport {
    PartialResolver* PartialResolver::inst = nullptr;
    PartialResolver* PartialResolver::instance() {
        if (inst == nullptr) {
            inst = new PartialResolver();
        }
        return inst;
    }

    QString PartialResolver::getPartial(const QString &name) {
        if (name == "traffic") {
            return "{#arrs}{arrs}{/arrs}{^arrs}-{/arrs}/{#deps}{deps}{/deps}{^deps}-{/deps}";
        }
        if (name == "trafficArrows") {
            return "{#arrs}{arrs}↘{/arrs}{#deps}↗{deps}{/deps}";
        }
        return QString("[> %1]").arg(name);
    }

    PartialResolver::PartialResolver() {}

    Context::Context(const class Airport* a)
        : Mustache::Context(PartialResolver::instance()), m_o(a) {}

    Context::~Context() {}

    QString Context::stringValue(const QString &key) const {
        if (key == "code") {
            return m_o->id;
        }
        if (key == "arrs") {
            return QString::number(m_o->nMaybeFilteredArrivals);
        }
        if (key == "deps") {
            return QString::number(m_o->nMaybeFilteredDepartures);
        }
        if (key == "allArrs") {
            return QString::number(m_o->arrivals.count());
        }
        if (key == "allDeps") {
            return QString::number(m_o->departures.count());
        }
        if (key == "controllers") {
            return m_o->controllersString();
        }
        if (key == "atis") {
            return m_o->atisCodeString();
        }
        if (key == "country") {
            return m_o->countryCode;
        }
        if (key == "prettyName") {
            return m_o->prettyName();
        }
        if (key == "name") {
            return m_o->name;
        }
        if (key == "city") {
            return m_o->city;
        }
        if (key == "frequencies") {
            return m_o->frequencyString();
        }
        if (key == "pdc") {
            return m_o->pdcString("");
        }
        if (key == "livestream") {
            return m_o->livestreamString();
        }

        return QString("{%1}").arg(key);
    }

    bool Context::isFalse(const QString &key) const {
        auto v = stringValue(key);
        return v.isEmpty() || v == "0";
    }

    int Context::listCount(const QString&) const {
        return 0;
    }

    void Context::push(const QString&, int) {}

    void Context::pop() {}
}
