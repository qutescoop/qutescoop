#ifndef AIRPORTDETAILSATCMODELITEM_H_
#define AIRPORTDETAILSATCMODELITEM_H_

#include "../../Controller.h"

class AirportDetailsAtcModelItem {
    public:
        explicit AirportDetailsAtcModelItem(const QStringList &columnTexts = {}, Controller* controller = nullptr, AirportDetailsAtcModelItem* parentItem = nullptr);
        ~AirportDetailsAtcModelItem();

        void removeChildren();
        int row() const;

        AirportDetailsAtcModelItem* m_parentItem;
        QStringList m_columnTexts;
        Controller* m_controller;
        QVector<AirportDetailsAtcModelItem*> m_childItems;
};

#endif
