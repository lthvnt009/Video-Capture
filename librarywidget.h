// librarywidget.h - Version 1.8 (Thêm phím Delete)
#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QKeyEvent> // Thêm vào

class LibraryWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit LibraryWidget(QWidget *parent = nullptr);

signals:
    void itemQuickExportRequested(QListWidgetItem *item);
    void imagesDropped(const QList<QUrl> &urls);
    void itemDoubleClicked(QListWidgetItem* item);
    void deleteRequested(); // Signal mới cho phím Delete

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override; // Override sự kiện nhấn phím
};

#endif // LIBRARYWIDGET_H
