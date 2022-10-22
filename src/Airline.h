/**************************************************************************
*  This file is part of QuteScoop. See README for license
**************************************************************************/

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
    virtual ~Airline() {
    }

    QString code, name, callsign, country;
};

#endif /* AIRLINE_H_ */
