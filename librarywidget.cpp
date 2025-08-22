// librarywidget.cpp - Version 1.5 (Drag & Drop Added)
#include "librarywidget.h"
#include <QApplication>
#include <QDropEvent>
#include <QListView>
#include <QMouseEvent>
#include <QMimeData>
#include <QFileInfo>

LibraryWidget::LibraryWidget(QWidget *parent) : QListWidget(parent)
{
    setDragDropMode(QAbstractItemView::NoDragDrop); // Sẽ xử lý drop thủ công
    setAcceptDrops(true); // Bật chức năng nhận drop
    setMovement(QListView::Static);
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void LibraryWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QListWidgetItem *item = itemAt(event->pos());
        if (item) {
            emit itemQuickExportRequested(item);
            event->accept();
            return;
        }
    }
    QListWidget::mouseDoubleClickEvent(event);
}

// THÊM MỚI: Xử lý khi có đối tượng được kéo vào widget
void LibraryWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            QFileInfo fileInfo(url.toLocalFile());
            QString suffix = fileInfo.suffix().toLower();
            // Chỉ chấp nhận các định dạng ảnh phổ biến
            if (suffix == "png" || suffix == "jpg" || suffix == "jpeg" || suffix == "bmp") {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

// THÊM MỚI: Xử lý khi có đối tượng được di chuyển bên trong widget
void LibraryWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

// THÊM MỚI: Xử lý khi đối tượng được thả vào widget
void LibraryWidget::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    QList<QUrl> validImageUrls;
    for (const QUrl &url : urls) {
        QFileInfo fileInfo(url.toLocalFile());
        QString suffix = fileInfo.suffix().toLower();
        if (suffix == "png" || suffix == "jpg" || suffix == "jpeg" || suffix == "bmp") {
            validImageUrls.append(url);
        }
    }
    if (!validImageUrls.isEmpty()) {
        emit imagesDropped(validImageUrls);
    }
    event->acceptProposedAction();
}
