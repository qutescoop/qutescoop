#include "AirportDetailsAtcModel.h"

AirportDetailsAtcModel::AirportDetailsAtcModel(QObject* parent)
    : QAbstractItemModel(parent) {
    rootItem = new AirportDetailsAtcModelItem();
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
        rootItem->appendChild(typeItem);

        foreach (const auto c, _all.values(type)) {
            if (typeItem->controller() == nullptr) {
                typeItem->setController(c);
            } else {
                auto* atcItem = new AirportDetailsAtcModelItem({}, c, typeItem);
                typeItem->appendChild(atcItem);
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

    AirportDetailsAtcModelItem* childItem = parentItem->child(row);
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
    AirportDetailsAtcModelItem* parentItem = childItem->parentItem();

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

    return parentItem->childCount();
}

int AirportDetailsAtcModel::columnCount(const QModelIndex&) const {
    return nColumns;
}

QVariant AirportDetailsAtcModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    AirportDetailsAtcModelItem* item = static_cast<AirportDetailsAtcModelItem*>(index.internalPointer());

    return item->data(index.column(), role);
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
    if (!item->isExpanded) {
        return;
    }

    if (item->controller() != nullptr && item->controller()->hasPrimaryAction()) {
        item->controller()->primaryAction();
    }
}

void AirportDetailsAtcModel::writeExpandedState(const QModelIndex &index, bool isExpanded) const {
    if (!index.isValid()) {
        return;
    }
    auto* item = static_cast<AirportDetailsAtcModelItem*>(index.internalPointer());
    item->isExpanded = isExpanded;
}
