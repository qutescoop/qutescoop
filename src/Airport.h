#ifndef AIRPORT_H_
#define AIRPORT_H_

#include "Controller.h"
#include "MapObject.h"
#include "Metar.h"
#include "Pilot.h"

class Pilot;

class Airport: public MapObject {
    public:
        const static int symbologyAppRadius_nm = 28;
        const static int symbologyTwrRadius_nm = 16;
        const static int symbologyGndRadius_nm = 13;
        const static int symbologyDelRadius_nm = 10;

        Airport(const QStringList &list, unsigned int debugLineNumber = 0);
        ~Airport();

        virtual void showDetailsDialog();

        virtual QString toolTip() const;
        virtual QString mapLabel() const;
        virtual bool matches(const QRegExp &regex) const;
        virtual QString prettyName() const;

        void resetWhazzupStatus();

        bool active;

        QSet<Controller*> approaches, towers, grounds, deliveries, atises;
        QSet<Pilot*> arrivals, departures;
        QSet<Controller*> allControllers() const;

        void addArrival(Pilot* client);
        void addDeparture(Pilot* client);
        int numFilteredArrivals, numFilteredDepartures;

        //void addCenter(Controller *client);
        void addApproach(Controller* client);
        void addTower(Controller* client);
        void addGround(Controller* client);
        void addDelivery(Controller* client);
        void addAtis(Controller* client);

        QString name, city, countryCode;

        bool showRoutes;

        const GLuint &appDisplayList();
        const GLuint &twrDisplayList();
        const GLuint &gndDisplayList();
        const GLuint &delDisplayList();

        Metar metar;

    private:
        GLuint _appDisplayList, _twrDisplayList, _gndDisplayList, _delDisplayList;
        void appGl(const QColor &middleColor, const QColor &marginColor, const QColor &borderColor, const GLfloat &borderLineWidth) const;
        void twrGl(const QColor &middleColor, const QColor &marginColor, const QColor &borderColor, const GLfloat &borderLineWidth) const;
};

#endif /*AIRPORT_H_*/
