#ifndef AIRPORT_H_
#define AIRPORT_H_

#include "Controller.h"
#include "MapObject.h"
#include "Metar.h"
#include "Pilot.h"

// that was to use the QJSEngine for the labels. But basically it was way too slow for us.
//class AirportApi: public QObject {
//    Q_OBJECT
//    public:
//        AirportApi(Airport* _a);
//        virtual ~AirportApi();
//        Q_PROPERTY(QString traffic READ _traffic CONSTANT);
//        Q_PROPERTY(QString controllers READ _controllers CONSTANT);
//    private:
//        QString _traffic();
//        QString _controllers();
//        Airport* m_airport;
//};

class Airport
    : public MapObject {
    Q_OBJECT
    public:
        const static int symbologyAppRadius_nm = 28;
        const static int symbologyTwrRadius_nm = 16;
        const static int symbologyGndRadius_nm = 13;
        const static int symbologyDelRadius_nm = 10;
        static const QHash<QString, std::function<QString(Airport*)> > placeholders;
        static const QRegularExpression pdcRegExp;

        Airport(const QStringList &list, unsigned int debugLineNumber = 0);
        virtual ~Airport();

        virtual bool matches(const QRegExp& regex) const override;
        virtual QString mapLabel() const override;
        virtual QString mapLabelHovered() const override;
        virtual QStringList mapLabelSecondaryLines() const override;
        virtual QStringList mapLabelSecondaryLinesHovered() const override;
        virtual QString toolTip() const override;
        virtual bool hasPrimaryAction() const override;
        virtual void primaryAction() override;

        void showDetailsDialog();

        const QString shortLabel() const;
        const QString longLabel() const;
        const QString prettyName() const;
        const QString trafficString() const;
        const QString trafficFilteredString() const;
        const QString trafficUnfilteredString() const;
        const QString controllersString() const;
        const QString atisCodeString() const;
        const QString frequencyString() const;
        const QString pdcString() const;

        void resetWhazzupStatus();

        QSet<Controller*> allControllers() const;
        QSet<Controller*> appDeps, twrs, gnds, dels, atiss;
        QSet<Pilot*> arrivals, departures;

        void addArrival(Pilot* client);
        void addDeparture(Pilot* client);
        uint nMaybeFilteredArrivals, nMaybeFilteredDepartures;

        uint congestion() const;

        void addController(Controller* c);

        const GLuint &appDisplayList();
        const GLuint &twrDisplayList();
        const GLuint &gndDisplayList();
        const GLuint &delDisplayList();

        Metar metar;
        QString id, name, city, countryCode;
        bool showRoutes;
        bool active;
    private:
        GLuint _appDisplayList, _twrDisplayList, _gndDisplayList, _delDisplayList;
        void appGl(const QColor &middleColor, const QColor &marginColor, const QColor &borderColor, const GLfloat &borderLineWidth) const;
        void twrGl(const QColor &middleColor, const QColor &marginColor, const QColor &borderColor, const GLfloat &borderLineWidth) const;
};

#endif /*AIRPORT_H_*/
