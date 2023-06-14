#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "Client.h"
#include "MapObject.h"
#include "Sector.h"
#include "WhazzupData.h"

#include <QJsonObject>

class Airport;

class Controller
    : public MapObject, public Client {
    Q_OBJECT
    public:
        static const QHash<QString, std::function<QString(Controller*)> > placeholders;
        static const QRegularExpression cpdlcRegExp;

        Controller(const QJsonObject& json, const WhazzupData* whazzup);
        virtual ~Controller();

        virtual QString rank() const override;
        virtual bool isFriend() const override;
        virtual QString toolTip() const override;
        virtual QString mapLabel() const override;
        virtual QString mapLabelHovered() const override;
        virtual QStringList mapLabelSecondaryLines() const override;
        virtual QStringList mapLabelSecondaryLinesHovered() const override;
        virtual QString livestreamString() const override;
        virtual bool matches(const QRegExp& regex) const override;
        virtual bool hasPrimaryAction() const override;
        virtual void primaryAction() override;

        void showDetailsDialog();

        QString toolTipShort() const;

        bool isObserver() const;
        bool isATC() const;

        QStringList atcLabelTokens() const;
        QString controllerSectorName() const;

        QString facilityString() const;
        QString typeString() const;

        bool isCtrFss() const;
        bool isAppDep() const;
        bool isTwr() const;
        bool isGnd() const;
        bool isDel() const;
        bool isAtis() const;

        QSet<Airport*> airports(bool withAdditionalMatches = true) const;
        QList<Airport*> airportsSorted() const;

        const QString cpdlcString() const;

        QString frequency, atisMessage, atisCode;
        int facilityType, visualRange;
        QDateTime assumeOnlineUntil;

        Sector* sector;
    protected:
        QString specialAirportWorkarounds(const QString& rawAirport) const;
    private:
};

#endif /*CONTROLLER_H_*/
