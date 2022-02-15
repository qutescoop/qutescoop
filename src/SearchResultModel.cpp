/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "SearchResultModel.h"

#include "Window.h"

Qt::ItemFlags SearchResultModel::flags(const QModelIndex &index) const
{
    MapObject* o = _content[index.row()];

    Client *c = dynamic_cast<Client*>(o);
    // allow adding an alias for a friend
    if(c != 0 && c->isFriend()) {
        return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractListModel::flags(index);
}

int SearchResultModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _content.count();
}

int SearchResultModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

bool SearchResultModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug() << "SearchResultModel::setData" << index.model();
    if (role == Qt::EditRole) {
        if (!checkIndex(index))
            return false;

        Client *c = dynamic_cast<Client*>(_content[index.row()]);
        if(c == 0)
            return false;

        Settings::setFriendAlias(c->userId, value.toString());

        return true;
    }
    return false;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || index.row() >= _content.size())
        return QVariant();

    if(role == Qt::DisplayRole) {
        MapObject* o = _content[index.row()];
        switch(index.column()) {
        case 0: return o->toolTip(); break;
        }
    }

    if(role == Qt::FontRole) {
        QFont result;
        //prefiled italic
        if(dynamic_cast<Pilot*>(_content[index.row()])) {
            Pilot *p = dynamic_cast<Pilot*>(_content[index.row()]);
            if(p->flightStatus() == Pilot::PREFILED) {
                result.setItalic(true);
            }
        }

        //friends bold
        Client *c = dynamic_cast<Client*>(_content[index.row()]);
        if(c == 0) return QVariant();

        if(c->isFriend()) {
            result.setBold(true);
        }
        return result;
    }

    return QVariant();
}

QVariant SearchResultModel::headerData(int section, enum Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return QVariant();

    if (section != 0)
        return QVariant();

    if (_content.isEmpty())
        return QString("No Results");

    return QString("%1 Result%2").arg(_content.size()).arg(_content.size() == 1? "": "s");
}

void SearchResultModel::modelClicked(const QModelIndex& index) {
    _content[index.row()]->showDetailsDialog();
}
