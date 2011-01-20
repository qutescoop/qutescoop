/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Metar.h"
#include "Settings.h"
#include "NavData.h"

#include <QRegExp>

Metar::Metar(const QString& encStr)
{
    if(!encStr.isEmpty()) {
        encoded = encStr;
        downloaded = QDateTime::currentDateTime();
    }
}

bool Metar::isNull() const {
    return encoded.isNull();
}

bool Metar::isValid() const {
    return encoded.contains(QRegExp("^[A-Z]{4} ")); // Metar must start with 4 capital letters, followed by a blank
}

bool Metar::doesNotExist() const {
    return !isNull() && !isValid();
}

bool Metar::needsRefresh() const {
    return isNull() || downloaded.secsTo(QDateTime::currentDateTime()) > Settings::metarDownloadInterval() * 60;
}

/***********************************************************
 * WARNING - CODE BELOW WILL MAKE YOUR EYES BLEED
 **********************************************************/

QString Metar::decodeDate(QStringList& tokens) const {
    QString date = tokens.first();
    tokens.removeFirst();
    QString result;

    // 012020Z
    int day = date.left(2).toInt();
    if(day == 1) result += "1st";
    else if(day == 2) result += "2nd";
    else result += QString("%1th").arg(day);

    date = date.right(5);
    result += " on " + date.left(2) + date.right(3).toLower();

    return result;
}

QString Metar::decodeWind(QStringList& tokens) const {
    QString wind = tokens.first();
    QString result = QString();

    // 00000KT
    if(wind.contains(QRegExp("^00000"))) {
        tokens.removeFirst();
        return "No Wind";
    }

    //35006KT
    if(wind.contains(QRegExp("^[0-9]{5}KT"))) {
        tokens.removeFirst();
        result = "Wind from " + wind.left(3) + " at ";
        wind = wind.right(wind.length() - 3);
        result += wind;
    }

    //13022G25KT
    if(wind.contains(QRegExp("^[0-9]{5}G[0-9]{2}KT"))) {
        tokens.removeFirst();
        result = "Wind from " + wind.left(3) + " at ";
        wind.remove(0, 3);
        result += wind.left(2) + " ";
        wind.remove(0, 3);
        result += "Gusts " + wind;
    }

    // VRB02KT VRB01KT
    if(wind.contains(QRegExp("^VRB[0-9]"))) {
        tokens.removeFirst();
        result = "Wind Calm";
    }

    // 320V040
    QString varying = tokens.first();
    if(varying.contains(QRegExp("^[0-9]{3}V[0-9]{3}$"))) {
        tokens.removeFirst();
        result += "<br>Varying Between " + varying.left(3) + " And " + varying.right(3);
    }

    return result;
}

QString Metar::decodeVisibility(QStringList& tokens) const {
    if(tokens.isEmpty()) return QString();
    QString vis = tokens.first();

    if(vis == "CAVOK") {
        tokens.removeFirst();
        return "Clouds and Visibility OK";
    }

    // 2 1/2SM
    if(vis.length() == 1) {
        QString result = "Visibility " + tokens.first();
        tokens.removeFirst();
        result += " " + tokens.first();
        tokens.removeFirst();
        return result;
    }

    // 5SM
    if(vis.contains(QRegExp("^[0-9]+SM"))) {
        tokens.removeFirst();
        return "Visibility " + vis;
    }

    // xKM
    if(vis.contains(QRegExp("^[0-9]+KM"))) {
        tokens.removeFirst();
        return "Visibility " + vis;
    }

    bool ok;
    int i = vis.toInt(&ok);
    if(!ok) return QString();

    tokens.removeFirst();

    if(i == 9999)
        return "Visibility 10KM or more";

    if(i < 50)
        return "Visibility less than 50m";

    if(i % 1000 == 0)
        return QString("Visibility %1KM").arg(i / 1000);

    return QString("Visibility %1m").arg(i);
}

#define SIG(xy, descr) if(sig.startsWith(xy)) { \
    if(!QString(descr).isEmpty()) { result += QString(descr) + " "; } \
    sig.remove(0, QString(xy).length()); }

#define SIG2(xy, descr) if(sig.startsWith(xy)) { \
    if(!QString(descr).isEmpty()) { result += QString(descr) + " "; } \
    sig.remove(0, QString(xy).length()); loop = true; }

QString Metar::decodeSigWX(QStringList& tokens) const {
    if(tokens.isEmpty()) return QString();
    QRegExp regex = QRegExp(QString("^(VC)?")                    // Proximity
                            + "(-|\\+)?"                         // Intensity
                            + "(MI|PR|BC|DR|BL|SH|TS|FZ)?"       // Descriptor
                            + "(((DZ|RA|SN|SG|IC|PL|GR|GS|UP)+)|"  // Precipitation
                            + "(BR|FG|FU|VA|DU|SA|HZ|PY))"       // Obscuration
                            + "(PO|SQ|FC|SS)?");                 // Other

    QString sig = tokens.first();
    if(!sig.contains(regex)) return QString();
    tokens.removeFirst();
    QString result;

    SIG("VC", "Vincinity");
    if(sig.startsWith("+")) { result += "Heavy "; sig.remove(0, 1); }
    if(sig.startsWith("-")) { result += "Light "; sig.remove(0, 1); }

    SIG("MI", "Shallow");
    SIG("PR", "Partial");
    SIG("BC", "Patches");
    SIG("DR", "Drifting");
    SIG("BL", "Blowing");
    SIG("SH", "Showers");
    SIG("TS", "Thunderstorm");
    SIG("FZ", "Freezing");

    bool loop;
    do {
        loop = false;
        SIG2("DZ", "Drizzle,");
        SIG2("RA", "Rain,");
        SIG2("SN", "Snow,");
        SIG2("SG", "Snow Grains,");
        SIG2("IC", "Ice,");
        SIG2("PL", "Ice Pellets,");
        SIG2("GR", "Hail,");
        SIG2("GS", "Snow Pellets,");
        SIG2("UP", "Unknown Precipitation,");
    } while(loop);

    SIG("BR", "Mist,");
    SIG("FG", "Fog,");
    SIG("FU", "Fume,");
    SIG("VA", "Volcanic Ash,");
    SIG("DU", "Dust,");
    SIG("SA", "Sand,");
    SIG("HZ", "Haze,");
    SIG("PY", "Spray,");

    // remove last "," from the string
    result = result.trimmed().left(result.length() - 2);
    return result;
}

QString Metar::decodeClouds(QStringList& tokens) const {
    if(tokens.isEmpty()) return QString();
    QRegExp regex = QRegExp("^(VV|FEW|SCT|BKN|OVC)([0-9]{3}|///)(CB|TCU)?");
    QString sig = tokens.first();

    if(!sig.contains(regex)) return QString();
    tokens.removeFirst();
    QString result;

    if(sig.startsWith("VV")) return result;

    SIG("FEW", "Few");
    SIG("SCT", "Scattered");
    SIG("BKN", "Broken");
    SIG("OVC", "Overcast");

    if(sig.contains(QRegExp("^[0-9]{3}"))) {
        int feet = sig.left(3).toInt();
        result += QString("%1").arg(feet) + "00ft";
        sig.remove(0, 3);
    }

    SIG("CB", "Cumulonimbus");
    SIG("TCU", "Towering Cumulus");

    return result;
}

QString Metar::decodeTemp(QStringList& tokens) const {
    if(tokens.isEmpty()) return QString();
    QRegExp regex = QRegExp("^(M?[0-9]{2})/(M?[0-9]{2}|//)");
    QString sig = tokens.first();

    if(!sig.contains(regex)) return QString();
    tokens.removeFirst();
    QString result = "Temperature ";

    SIG("M", "Minus");

    int i = sig.left(2).toInt();
    result += QString("%1").arg(i);
    sig.remove(0, 3);
    result += ", Dew Point ";
    SIG("M", "Minus");

    i = sig.toInt();
    result += QString("%1").arg(i);

    return result;
}

QString Metar::decodeQNH(QStringList& tokens) const {
    if(tokens.isEmpty()) return QString();
    QString sig = tokens.first();
    if(sig.contains(QRegExp("^Q[0-9]{4}"))) {
        tokens.removeFirst();
        sig.remove(0, 1);
        return "QNH " + sig;
    }
    if(sig.contains(QRegExp("^A[0-9]{4}"))) {
        tokens.removeFirst();
        sig.remove(0, 1);
        QString result = "Altimeter " + sig.left(2);
        sig.remove(0, 2);
        result += "." + sig;
        return result;
    }
    return QString();
}

QString Metar::decodePrediction(QStringList& tokens) const {
    if(tokens.isEmpty()) return QString();
    QString sig = tokens.first();

    if(sig.contains(QRegExp("^BECMG"))) {
        tokens.removeFirst();
        return "Becoming";
    }
    if(sig.contains(QRegExp("^NOSIG"))) {
        tokens.removeFirst();
        return "No Significant Changes expected";
    }
    if(sig.contains(QRegExp("^TEMPO"))) {
        tokens.removeFirst();
        return "Temporary";
    }

    return QString();
}

QString Metar::decodedHtml() const {
    QString result = "";
    QStringList tokens = encoded.split(" ", QString::SkipEmptyParts);

    // LOWW 012020Z 35006KT 320V040 9999 -RA FEW060 SCT070 BKN080 08/05 Q1014 NOSIG
    Airport *a = NavData::getInstance()->airports()[tokens.first()];
    if(a == 0) result = tokens.first();
    else result = a->city;
    result += " Weather Information, Recorded ";
    tokens.removeFirst();
    result += decodeDate(tokens);

    while(!tokens.isEmpty()) {
        QString part = decodeWind(tokens);
        if(!part.isEmpty()) result += "<br>" + part;

        part = decodeVisibility(tokens);
        if(!part.isEmpty()) result += "<br>" + part;

        part = "x";
        bool br = false;
        while(!part.isEmpty()) {
            part = decodeSigWX(tokens);
            if(!part.isEmpty()) {
                if(!br) { result += "<br>"; br = true; }
                else result += ", ";
                result += part;
            }
        }

        part = "x";
        br = false;
        while(!part.isEmpty()) {
            part = decodeClouds(tokens);
            if(!part.isEmpty()) {
                if(!br) { result += "<br>Clouds "; br = true;}
                else result += ", ";
                result += part;
            }
        }

        part = decodeTemp(tokens);
        if(!part.isEmpty()) result += "<br>" + part;

        part = decodeQNH(tokens);
        if(!part.isEmpty()) result += "<br>" + part;

        part = decodePrediction(tokens);
        if(part.isEmpty()) {
            if(!tokens.isEmpty()) {
                if(tokens.first().startsWith("RMK"))
                    tokens.clear();
                else
                    tokens.removeFirst();
            }
        } else {
            result += "<br>" + part + " ";
        }
    }

    return result;
}
