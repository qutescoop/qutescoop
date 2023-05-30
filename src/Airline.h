#ifndef AIRLINE_H_
#define AIRLINE_H_

#include <QtCore>

class Airline {
public:
    Airline(const QString& code, const QString& name, const QString& callsign, const QString& country) :
        code(code),
        name(name),
        callsign(callsign),
        country(country){
    }

    QString label() const {
        return QString("%1 \"%2\"").arg(code, callsign);
    }

    QString toolTip() const {
        return QString("%1 \"%2\", %3, %4").arg(code, callsign, name, country);
    }

    virtual ~Airline() {
    }

    QString code, name, callsign, country;
};

#endif /* AIRLINE_H_ */
