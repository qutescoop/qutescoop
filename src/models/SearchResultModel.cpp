#include "SearchResultModel.h"

#include "../Pilot.h"

#include <QFont>

SearchResultModel::SearchResultModel(QObject* parent)
    : QAbstractListModel(parent) {}

int SearchResultModel::rowCount(const QModelIndex&) const {
    return _content.count();
}

int SearchResultModel::columnCount(const QModelIndex&) const {
    return 1;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= _content.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        MapObject* o = _content.value(index.row(), nullptr);
        if (o == nullptr) {
            return QVariant();
        }
        switch (index.column()) {
            case 0: return o->toolTip(); break;
        }

        return QVariant();
    }

    if (role == Qt::FontRole) {
        QFont result;
        MapObject* o = _content.value(index.row(), nullptr);
        if (o == nullptr) {
            return result;
        }
        // prefiled italic
        Pilot* p = dynamic_cast<Pilot*>(o);
        if (p != 0 && p->flightStatus() == Pilot::PREFILED) {
            result.setItalic(true);
        }

        // friends bold
        Client* c = dynamic_cast<Client*>(o);
        if (c != 0 && c->isFriend()) {
            result.setBold(true);
        }
        return result;
    }

    return QVariant();
}

QVariant SearchResultModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    if (section != 0) {
        return QVariant();
    }

    if (m_isSearching) {
        return "…";
    }

    if (_content.isEmpty()) {
        return "No Results";
    }

    return QString("%1 Result%2").arg(_content.size()).arg(_content.size() == 1? "": "s");
}

void SearchResultModel::setSearchResults(const QList<MapObject*> &searchResult) {
    beginResetModel();
    _content = searchResult;
    endResetModel();
}

void SearchResultModel::modelClicked(const QModelIndex& index) {
    MapObject* o = _content.value(index.row(), nullptr);
    if (o == nullptr) {
        return;
    }

    if (o->hasPrimaryAction()) {
        o->primaryAction();
    }
}
