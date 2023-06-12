#include "AirportDetailsAtcModelItem.h"

#include <src/models/AirportDetailsAtcModel.h>

AirportDetailsAtcModelItem::AirportDetailsAtcModelItem(const QStringList &columnTexts, Controller* controller, AirportDetailsAtcModelItem* parent)
    : m_parentItem(parent), m_columnTexts(columnTexts), m_controller(controller) {}

AirportDetailsAtcModelItem::~AirportDetailsAtcModelItem() {
    qDeleteAll(m_childItems);
}

void AirportDetailsAtcModelItem::appendChild(AirportDetailsAtcModelItem* item) {
    m_childItems.append(item);
}

void AirportDetailsAtcModelItem::removeChildren() {
    qDeleteAll(m_childItems);
    m_childItems.clear();
}

void AirportDetailsAtcModelItem::setController(Controller* c) {
    m_controller = c;
}

AirportDetailsAtcModelItem* AirportDetailsAtcModelItem::child(int row) {
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }
    return m_childItems.at(row);
}

int AirportDetailsAtcModelItem::childCount() const {
    return m_childItems.count();
}

QVariant AirportDetailsAtcModelItem::data(int column, int role) const {
    if (column < 0 || column >= AirportDetailsAtcModel::nColumns) {
        return QVariant();
    }

    if (column == 0) {
        if (role == Qt::ForegroundRole) {
            return QGuiApplication::palette().window();
        } else if (role == Qt::BackgroundRole) {
            return QGuiApplication::palette().text();
        }
    }

    // group row
    if (!m_columnTexts.isEmpty() && column == 0) {
        if (role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        } else if (role == Qt::DisplayRole) {
            return m_columnTexts.value(column, QString());
        } else if (role == Qt::UserRole) { // for sorting
            const auto _value = m_columnTexts.value(column, QString());
            const auto _index = typesSorted.indexOf(_value);
            return _index != -1? QVariant(_index): _value;
        }
        return QVariant();
    }

    // controller
    if (role == Qt::FontRole) {
        if (m_controller->isFriend()) {
            QFont result;
            result.setBold(true);
            return result;
        }
        return QFont();
    } else if (role == Qt::TextAlignmentRole) {
        switch (column) {
            case 1:
            case 6:
                return int(Qt::AlignRight | Qt::AlignVCenter);
            case 3:
            case 5:
                return Qt::AlignCenter;
        }
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::DisplayRole) {
        switch (column) {
            case 1: return isExpanded? m_controller->callsign: "";
            case 2: {
                if (!isExpanded) {
                    QStringList freqs { m_controller->frequency };
                    foreach (const auto* i, m_childItems) {
                        freqs << i->controller()->frequency;
                    }
                    freqs.sort();

                    return freqs.join(", ");
                }

                return m_controller->frequency;
            }
            case 3: return isExpanded? m_controller->atisCode: "";
            case 4: return isExpanded? m_controller->realName(): "";
            case 5: return isExpanded? m_controller->rank(): "";
            case 6: return isExpanded? m_controller->onlineTime(): "";
            case 7: return isExpanded? m_controller->assumeOnlineUntil.time().toString("HHmm'z'"): "";
        }
    } else if (role == Qt::UserRole) { // for sorting
        return m_controller->callsign;
    }

    return QVariant();
}

Controller* AirportDetailsAtcModelItem::controller() const {
    return m_controller;
}

int AirportDetailsAtcModelItem::row() const {
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<AirportDetailsAtcModelItem*>(this));
    }

    return 0;
}

AirportDetailsAtcModelItem* AirportDetailsAtcModelItem::parentItem() {
    return m_parentItem;
}
