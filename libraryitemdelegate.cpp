// libraryitemdelegate.cpp - Version 1.6 (UI Tweaks)
#include "libraryitemdelegate.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QStyleOptionViewItem>
#include <QIcon>
#include <QListView>

LibraryItemDelegate::LibraryItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QRect LibraryItemDelegate::getCheckBoxRect(const QStyleOptionViewItem &option) const
{
    const QListView *view = qobject_cast<const QListView*>(option.widget);
    QSize iconSize = view ? view->iconSize() : QSize(128, 72);

    int pixmapX = option.rect.x() + (option.rect.width() - iconSize.width()) / 2;
    int pixmapY = option.rect.y() + (option.rect.height() - iconSize.height()) / 2;
    QRect pixmapRect(pixmapX, pixmapY, iconSize.width(), iconSize.height());
    
    int size = LibraryItemDelegate::CHECKBOX_SIZE;
    int margin = LibraryItemDelegate::CHECKBOX_MARGIN;
    return QRect(pixmapRect.left() + margin, pixmapRect.bottom() - size - margin, size, size);
}


void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    const QListView *view = qobject_cast<const QListView*>(option.widget);
    QSize iconSize = view ? view->iconSize() : QSize(128, 72);

    QPixmap pixmap = icon.pixmap(iconSize);

    int x = option.rect.x() + (option.rect.width() - pixmap.width()) / 2;
    int y = option.rect.y() + (option.rect.height() - pixmap.height()) / 2;
    
    if (!pixmap.isNull()) {
        painter->drawPixmap(x, y, pixmap);
    }
    
    Qt::CheckState checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
    QRect checkBoxRect = getCheckBoxRect(option);
    
    // TINH CHỈNH: Đã loại bỏ phần vẽ nền cho checkbox
    // painter->setBrush(QColor(0, 0, 0, 100));
    // painter->setPen(Qt::NoPen);
    // painter->drawRoundedRect(...);

    QStyleOptionButton checkBoxOpt;
    checkBoxOpt.rect = checkBoxRect;
    checkBoxOpt.state = QStyle::State_Enabled;
    if (checkState == Qt::Checked) {
        checkBoxOpt.state |= QStyle::State_On;
    } else {
        checkBoxOpt.state |= QStyle::State_Off;
    }
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOpt, painter);

    painter->restore();
}

bool LibraryItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QRect checkBoxRect = getCheckBoxRect(option);

        if (checkBoxRect.contains(mouseEvent->pos())) {
            Qt::CheckState currentState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
            Qt::CheckState newState = (currentState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
            model->setData(index, newState, Qt::CheckStateRole);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
