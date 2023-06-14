#include "AirportDetailsAtcModelItem.h"

#include <src/models/AirportDetailsAtcModel.h>

AirportDetailsAtcModelItem::AirportDetailsAtcModelItem(const QStringList &columnTexts, Controller* controller, AirportDetailsAtcModelItem* parent)
    : m_parentItem(parent), m_columnTexts(columnTexts), m_controller(controller) {}

AirportDetailsAtcModelItem::~AirportDetailsAtcModelItem() {
    qDeleteAll(m_childItems);
}

void AirportDetailsAtcModelItem::removeChildren() {
    qDeleteAll(m_childItems);
    m_childItems.clear();
}

int AirportDetailsAtcModelItem::row() const {
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<AirportDetailsAtcModelItem*>(this));
    }

    return 0;
}
