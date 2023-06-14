#include "NavAid.h"

const QHash<NavAid::Type, QString> NavAid::typeStrings = {
    { NDB, "NDB" },
    { VOR, "VOR" },
    { ILS_LOC, "ILS" },
    { LOC, "LOC" },
    { GS, "GS" },
    { OM, "OM" },
    { MM, "OM" },
    { IM, "IM" },
    { DME_NO_FREQ, "DME (no freq)" },
    { DME, "DME" },
    { FAP_GBAS, "FAP alignment point" },
    { GBAS_GND, "GBAS Ground station" },
    { GBAS_THR, "GBAS Threshold point" },
    { CUSTOM_VORDME, "VOR/DME" }
};

NavAid::NavAid(const QStringList&stringList) {
    if (stringList.size() < 12) {
        QMessageLogger("earth_nav.dat", 0, QT_MESSAGELOG_FUNC).critical()
            << "could not parse" << stringList << "as Navaid. Expected more than 12 fields.";
        return;
    }

    bool ok;

    _type = (Type) stringList[0].toInt(&ok);
    if (!ok) {
        QMessageLogger("earth_nav.dat", 0, QT_MESSAGELOG_FUNC).critical()
            << "unable to parse waypointtype (int):" << stringList;
        return;
    }
    lat = stringList[1].toDouble(&ok);
    if (!ok) {
        QMessageLogger("earth_nav.dat", 0, QT_MESSAGELOG_FUNC).critical()
            << "unable to parse lat (double):" << stringList;
        return;
    }
    lon = stringList[2].toDouble(&ok);
    if (!ok) {
        QMessageLogger("earth_nav.dat", 0, QT_MESSAGELOG_FUNC).critical()
            << "unable to parse lon (double):" << stringList;
        return;
    }

    _freq = stringList[4].toInt(&ok);
    if (!ok) {
        QMessageLogger("earth_nav.dat", 0, QT_MESSAGELOG_FUNC).critical()
            << "unable to parse freq (int):" << stringList;
        return;
    }

    id = stringList[7];

    regionCode = stringList[9];

    _name = "";
    for (int i = 10; i < stringList.size(); i++) {
        _name += stringList[i] + (i > 9? " ": "");
    }
    _name = _name.trimmed();
}

QString NavAid::typeStr(Type type) {
    return typeStrings.value(type, QString());
}

QString NavAid::toolTip() const {
    QString ret = id + " (" + _name + ")";

    if (_type == NDB) {
        ret.append(QString(" %1 kHz").arg(_freq));
    } else if (
        _type == VOR || _type == DME || _type == DME_NO_FREQ
        || _type == ILS_LOC || _type == LOC || _type == GS
        || _type == CUSTOM_VORDME
    ) {
        ret.append(QString(" %1 MHz").arg(_freq / 100., 0, 'f', 2));
    } else if (_freq != 0) {
        ret.append(QString(" %1?").arg(_freq));
    }
    if ((_type == ILS_LOC || _type == LOC) && _hdg != 0) {
        ret.append(QString(" %1").arg((double) _hdg, 0, 'f', 0));
    }
    if (!NavAid::typeStr(_type).isEmpty()) {
        ret.append(QString(" [%1]").arg(typeStr(_type)));
    }
    return ret;
}

QString NavAid::mapLabelHovered() const {
    return (id + " " + typeStr(_type)).trimmed();
}

QStringList NavAid::mapLabelSecondaryLinesHovered() const {
    return { freqString() }; // + "\n" + airwaysString()).trimmed().split("\n");
}

QString NavAid::freqString() const {
    if (_type == NDB) {
        return QString("%1 kHz").arg(_freq);
    } else if (
        _type == VOR || _type == DME || _type == DME_NO_FREQ
        || _type == ILS_LOC || _type == LOC || _type == GS
        || _type == CUSTOM_VORDME
    ) {
        return QString("%1 MHz").arg(_freq / 100., 0, 'f', 2);
    }

    return "";
}

int NavAid::type() {
    return _type;
}

void NavAid::upgradeToVorDme() {
    if (_type == VOR) {
        _type = CUSTOM_VORDME;
    }
}
