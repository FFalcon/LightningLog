#ifndef CUSTOMDISPLAYDELEGATE_H
#define CUSTOMDISPLAYDELEGATE_H

#include <QStyledItemDelegate>

class CustomDisplayDelegate : public QStyledItemDelegate
{
    Q_OBJECT;

public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // CUSTOMDISPLAYDELEGATE_H
