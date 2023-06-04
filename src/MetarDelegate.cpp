#include "MetarDelegate.h"

#include <QApplication>

void MetarDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (!index.data().isValid()) {
        return;
    }

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    Q_ASSERT(opt.widget);

    const QStyle* style = opt.widget->style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    const int hMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 2;
    const int vMargin = style->pixelMetric(QStyle::PM_FocusFrameVMargin) + 2;
    const QRect contentRect = QRect(
        opt.rect.x() + hMargin,
        opt.rect.y() + vMargin,
        opt.rect.width() - 2 * hMargin,
        opt.rect.height() - 2 * vMargin
    );

    painter->save();
    const QRect alignedRect = style->alignedRect(
        opt.direction,
        Qt::AlignTop,
        contentRect.size(),
        contentRect
    );
    painter->setFont(opt.font);
    style->drawItemText(
        painter,
        alignedRect,
        Qt::TextWordWrap,
        QApplication::palette(),
        opt.state & QStyle::State_Enabled,
        index.data().toString()
    );
    painter->restore();
}

QSize MetarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (!index.data().isValid()) {
        return QSize(0, 0);
    }

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    Q_ASSERT(opt.widget);

    const QStyle* style = opt.widget->style();

    const int hMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 2;
    const int vMargin = style->pixelMetric(QStyle::PM_FocusFrameVMargin) + 2;
    const int scrollBarWidth = style->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int width = opt.widget->width() - 2 * hMargin - scrollBarWidth;

    const QRect textArea = QRect(
        hMargin,
        vMargin,
        width,
        1000
    );

    const QRect textRect = style->itemTextRect(
        opt.fontMetrics,
        textArea,
        Qt::TextWordWrap,
        opt.state & QStyle::State_Enabled,
        index.data().toString()
    );

    return QSize(0, textRect.height() + 2 * vMargin);
}
