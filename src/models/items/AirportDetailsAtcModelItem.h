#ifndef AIRPORTDETAILSATCMODELITEM_H_
#define AIRPORTDETAILSATCMODELITEM_H_

#include "../../Controller.h"

class AirportDetailsAtcModelItem {
public:
    explicit AirportDetailsAtcModelItem(const QStringList &columnTexts = {}, Controller* controller = nullptr, AirportDetailsAtcModelItem* parentItem = nullptr);
    ~AirportDetailsAtcModelItem();

    void appendChild(AirportDetailsAtcModelItem* child);
    void removeChildren();
    void setController(Controller* controller);

    AirportDetailsAtcModelItem* child(int row);
    int childCount() const;
    QVariant data(int column, int role) const;
    Controller* controller() const;
    int row() const;
    AirportDetailsAtcModelItem* parentItem();
    bool isExpanded = false;

private:
    const QStringList typesSorted { "FSS", "CTR", "DEP", "APP", "TWR", "GND", "DEL", "ATIS", "OBS" };

    AirportDetailsAtcModelItem* m_parentItem;
    QVector<AirportDetailsAtcModelItem*> m_childItems;

    QStringList m_columnTexts;
    Controller* m_controller;
};

#endif
