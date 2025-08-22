// librarywidget.h - Version 1.7 (Thêm Double Click)
#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

class LibraryWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit LibraryWidget(QWidget *parent = nullptr);

signals:
    void itemQuickExportRequested(QListWidgetItem *item);
    void imagesDropped(const QList<QUrl> &urls);
    // THÊM MỚI: Tín hiệu cho double-click chuột trái
    void itemDoubleClicked(QListWidgetItem* item);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
};

#endif // LIBRARYWIDGET_H
