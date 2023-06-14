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

    if (role == Qt::DisplayRole) {
        MapObject* o = _content[index.row()];
        switch (index.column()) {
            case 0: return o->toolTip(); break;
        }
    } else if (role == Qt::ToolTipRole) {
        MapObject* o = _content[index.row()];
        switch (index.column()) {
            case 0: return o->toolTip(); break;
        }
    } else if (role == Qt::FontRole) {
        QFont result;
        // prefiled italic
        if (dynamic_cast<Pilot*>(_content[index.row()])) {
            Pilot* p = dynamic_cast<Pilot*>(_content[index.row()]);
            if (p->flightStatus() == Pilot::PREFILED) {
                result.setItalic(true);
            }
        }

        // friends bold
        Client* c = dynamic_cast<Client*>(_content[index.row()]);
        if (c == 0) {
            return QVariant();
        }

        if (c->isFriend()) {
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
    if (_content[index.row()]->hasPrimaryAction()) {
        _content[index.row()]->primaryAction();
    }
}
