// librarywidget.h - Version 1.4 (Drag & Drop Added)
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
    // THÊM MỚI: Signal cho chức năng kéo-thả
    void imagesDropped(const QList<QUrl> &urls);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    // THÊM MỚI: Ghi đè các sự kiện kéo-thả
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
};

#endif // LIBRARYWIDGET_H
