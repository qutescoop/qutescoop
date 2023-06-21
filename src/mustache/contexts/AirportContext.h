#ifndef MUSTACHEQS_AIRPORT_CONTEXT_H
#define MUSTACHEQS_AIRPORT_CONTEXT_H

#include "src/mustache/external/qt-mustache/mustache.h"

class Airport;
namespace MustacheQs::Airport {
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
            Context(const class Airport*);
            virtual ~Context();

            virtual QString stringValue(const QString &key) const override;
            virtual bool isFalse(const QString &key) const override;
            virtual int listCount(const QString &key) const override;
            virtual void push(const QString &key, int index) override;
            virtual void pop() override;
        private:
            const class Airport* m_o;
    };
}

#endif
