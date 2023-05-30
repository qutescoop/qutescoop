#ifndef METAR_H_
#define METAR_H_

#include <QDateTime>
#include <QString>

class Metar {
    public:
        Metar(const QString& encodedString = QString(), const QString& airportLabel = QString());

        bool isNull() const;
        bool isValid() const;
        bool doesNotExist() const;
        bool needsRefresh() const;

        QString encoded, airportLabel;
        QDateTime downloaded;
        QString humanHtml() const;

    private:
        QString decodeDate(QStringList& tokens) const;
        QString decodeWind(QStringList& tokens) const;
        QString decodeVisibility(QStringList& tokens) const;
        QString decodeSigWX(QStringList& tokens) const;
        QString decodeClouds(QStringList& tokens) const;
        QString decodeTemp(QStringList& tokens) const;
        QString decodeQNH(QStringList& tokens) const;
        QString decodePrediction(QStringList& tokens) const;
};

#endif /*METAR_H_*/
