#include "PilotContext.h"

#include "src/Pilot.h"

namespace MustacheQs::Pilot {
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

    Context::Context(const class Pilot* a)
        : Mustache::Context(PartialResolver::instance()), m_o(a) {}

    Context::~Context() {}

    QString Context::stringValue(const QString &key) const {
        if (key == "debug:nextWp") {
            QList<Waypoint*> waypoints = ((class Pilot*) m_o)->routeWaypointsWithDepDest();
            int next = m_o->nextPointOnRoute(waypoints);
            Waypoint* w = waypoints.value(next, nullptr);
            if (w == nullptr) {
                return "";
            }
            return w->id;
        }

        if (key == "login") {
            return m_o->callsign;
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
        if (key == "dep") {
            return m_o->planDep;
        }
        if (key == "dest") {
            return m_o->planDest;
        }
        if (key == "FL") {
            return m_o->flOrEmpty();
        }
        if (key == "GS") {
            auto _gs = m_o->groundspeed;
            if (_gs == 0) {
                return "";
            }
            return QString("N%1").arg(m_o->groundspeed);
        }
        if (key == "GS10") {
            auto _gs = m_o->groundspeed;
            if (_gs == 0) {
                return "";
            }
            return QString("N%1").arg(round(m_o->groundspeed / 10.));
        }
        if (key == "rules") {
            return m_o->planFlighttype;
        }
        if (key == "rulesIfNotIfr") {
            return m_o->planFlighttype != "I"? m_o->planFlighttype: "";
        }
        if (key == "type") {
            return m_o->aircraftType();
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
