#include "AirportDetailsAtcModel.h"

#include <src/Settings.h>

AirportDetailsAtcModel::AirportDetailsAtcModel(QObject* parent)
    : QAbstractItemModel(parent) {
    rootItem = new AirportDetailsAtcModelItem();

    m_atcExpandedByType = Settings::airportDialogAtcExpandedByType();
}

AirportDetailsAtcModel::~AirportDetailsAtcModel() {
    delete rootItem;
}

void AirportDetailsAtcModel::setClients(QList<Controller*> controllers) {
    beginResetModel();

    QMultiMap<QString, Controller*> _all;

    std::sort(
        controllers.begin(), controllers.end(), [](const Controller* a, const Controller* b)-> bool {
            return a->callsign > b->callsign;
        }
    );

    foreach (const auto c, controllers) {
        _all.insert(c->typeString(), c);
    }
    auto types = _all.uniqueKeys();

    rootItem->removeChildren();
    foreach (const auto type, types) {
        auto* typeItem = new AirportDetailsAtcModelItem({ type }, nullptr, rootItem);
        rootItem->m_childItems.append(typeItem);

        foreach (const auto c, _all.values(type)) {
            if (typeItem->m_controller == nullptr) {
                typeItem->m_controller = c;
            } else {
                auto* atcItem = new AirportDetailsAtcModelItem({}, c, typeItem);
                typeItem->m_childItems.append(atcItem);
            }
        }
    }

    endResetModel();
}

QModelIndex AirportDetailsAtcModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    AirportDetailsAtcModelItem* parentItem;

    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<AirportDetailsAtcModelItem*>(parent.internalPointer());
    }

    AirportDetailsAtcModelItem* childItem = parentItem->m_childItems.value(row, nullptr);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex AirportDetailsAtcModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }

    AirportDetailsAtcModelItem* childItem = static_cast<AirportDetailsAtcModelItem*>(index.internalPointer());
    AirportDetailsAtcModelItem* parentItem = childItem->m_parentItem;

    if (parentItem == rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int AirportDetailsAtcModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    AirportDetailsAtcModelItem* parentItem;
    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<AirportDetailsAtcModelItem*>(parent.internalPointer());
    }

    return parentItem->m_childItems.count();
}

int AirportDetailsAtcModel::columnCount(const QModelIndex&) const {
    return nColumns;
}

QVariant AirportDetailsAtcModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    AirportDetailsAtcModelItem* item = static_cast<AirportDetailsAtcModelItem*>(index.internalPointer());

    if (index.column() < 0 || index.column() >= AirportDetailsAtcModel::nColumns) {
        return QVariant();
    }

    if (index.column() == 0) {
        if (role == Qt::ForegroundRole) {
            return QGuiApplication::palette().window();
        } else if (role == Qt::BackgroundRole) {
            return QGuiApplication::palette().text();
        }
    }

    // group row
    if (!item->m_columnTexts.isEmpty() && index.column() == 0) {
        if (role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        } else if (role == Qt::DisplayRole) {
            return item->m_columnTexts.value(index.column(), QString());
        } else if (role == Qt::UserRole) { // for sorting
            const auto _value = item->m_columnTexts.value(index.column(), QString());
            const auto _index = typesSorted.indexOf(_value);
            return _index != -1? QVariant(_index): _value;
        }
        return QVariant();
    }

    // controller
    if (role == Qt::FontRole) {
        if (item->m_controller->isFriend()) {
            QFont result;
            result.setBold(true);
            return result;
        }
        return QFont();
    }

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case 1:
            case 6:
                return int(Qt::AlignRight | Qt::AlignVCenter);
            case 3:
            case 5:
                return Qt::AlignCenter;
        }
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 1: return isExpanded(item)? item->m_controller->callsign: "";
            case 2: {
                if (!isExpanded(item)) {
                    QStringList freqs { item->m_controller->frequency };
                    foreach (const auto* i, item->m_childItems) {
                        freqs << i->m_controller->frequency;
                    }
                    freqs.removeDuplicates();
                    freqs.sort();

                    return freqs.join(", ");
                }

                return item->m_controller->frequency;
            }
            case 3: return isExpanded(item)? item->m_controller->atisCode: "";
            case 4: return isExpanded(item)? item->m_controller->realName(): "";
            case 5: return isExpanded(item)? item->m_controller->rank(): "";
            case 6: return isExpanded(item)? item->m_controller->onlineTime(): "";
            case 7: return isExpanded(item)? item->m_controller->assumeOnlineUntil.time().toString("HHmm'z'"): "";
        }
    }

    if (role == Qt::UserRole) { // for sorting
        return item->m_controller->callsign;
    }

    return QVariant();
}

Qt::ItemFlags AirportDetailsAtcModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QVariant AirportDetailsAtcModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QString();
            case 1: return "Login";
            case 2: return "Freq";
            case 3: return "ATIS";
            case 4: return "Name";
            case 5: return "Rank";
            case 6: return "Online";
            case 7: return "Until";
        }
    }
    return QVariant();
}

void AirportDetailsAtcModel::modelSelected(const QModelIndex& index) const {
    if (!index.isValid()) {
        return;
    }
    auto* item = static_cast<AirportDetailsAtcModelItem*>(index.internalPointer());
    if (!isExpanded(item)) {
        return;
    }

    if (item->m_controller != nullptr && item->m_controller->hasPrimaryAction()) {
        item->m_controller->primaryAction();
    }
}

void AirportDetailsAtcModel::writeExpandedState(const QModelIndex &index, bool isExpanded) {
    if (!index.isValid()) {
        return;
    }
    auto* item = static_cast<AirportDetailsAtcModelItem*>(index.internalPointer());
    m_atcExpandedByType.insert(item->m_controller->typeString(), isExpanded);
    Settings::setAirportDialogAtcExpandedByType(m_atcExpandedByType);
}

bool AirportDetailsAtcModel::isExpanded(AirportDetailsAtcModelItem* item) const {
    return item->m_childItems.isEmpty() || m_atcExpandedByType.value(item->m_controller->typeString(), false).toBool();
}
