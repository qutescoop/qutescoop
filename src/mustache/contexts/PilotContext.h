#ifndef MUSTACHEQS_PILOT_CONTEXT_H
#define MUSTACHEQS_PILOT_CONTEXT_H

#include "src/mustache/external/qt-mustache/mustache.h"

class Pilot;
namespace MustacheQs::Pilot {
    class PartialResolver
        : public Mustache::PartialResolver {
        public:
            static PartialResolver* instance();
            virtual QString getPartial(const QString& name) override;
        private:
            PartialResolver();
            static PartialResolver* inst;
    };

    class Context
        : public Mustache::Context {
        public:
            Context(const class Pilot*);
            virtual ~Context();

            virtual QString stringValue(const QString &key) const override;
            virtual bool isFalse(const QString &key) const override;
            virtual int listCount(const QString &key) const override;
            virtual void push(const QString &key, int index) override;
            virtual void pop() override;
        private:
            const class Pilot* m_o;
    };
}

#endif
