#include "customdisplaydelegate.h"

void CustomDisplayDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem custom_options{option};
    custom_options.font.setWeight(QFont::Bold);
    custom_options.backgroundBrush.setColor(Qt::red);
    custom_options.backgroundBrush.setStyle(Qt::BrushStyle::SolidPattern);
    QStyledItemDelegate::paint(painter, custom_options, index);
}
