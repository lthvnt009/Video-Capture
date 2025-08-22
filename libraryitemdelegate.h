// libraryitemdelegate.h - Version 1.5 (UI Tweaks)
#ifndef LIBRARYITEMDELEGATE_H
#define LIBRARYITEMDELEGATE_H

#include <QStyledItemDelegate>

class LibraryItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit LibraryItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    QRect getCheckBoxRect(const QStyleOptionViewItem &option) const;

    // TINH CHỈNH: Giảm kích thước checkbox
    static constexpr int CHECKBOX_SIZE = 12;
    static constexpr int CHECKBOX_MARGIN = 3;
    // Đã loại bỏ các hằng số cho background
};

#endif // LIBRARYITEMDELEGATE_H
