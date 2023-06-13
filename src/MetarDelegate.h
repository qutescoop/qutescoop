#ifndef METARDELEGATE_H
#define METARDELEGATE_H

#include <QPainter>
#include <QStyledItemDelegate>
#include <QtCore>

class MetarDelegate
    : public QStyledItemDelegate {
    Q_OBJECT
    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        void paint(
            QPainter* painter,
            const QStyleOptionViewItem &option,
            const QModelIndex &index
        ) const override;
        QSize sizeHint(
            const QStyleOptionViewItem &option,
            const QModelIndex &index
        ) const override;
};

#endif // METARDELEGATE_H
