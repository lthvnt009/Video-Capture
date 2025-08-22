// librarywidget.cpp - Version 1.7 (Thêm Double Click)
#include "librarywidget.h"
#include <QApplication>
#include <QDropEvent>
#include <QListView>
#include <QMouseEvent>
#include <QMimeData>
#include <QFileInfo>

LibraryWidget::LibraryWidget(QWidget *parent) : QListWidget(parent)
{
    setDragDropMode(QAbstractItemView::NoDragDrop); 
    setAcceptDrops(true); 
    setMovement(QListView::Static);
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void LibraryWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QListWidgetItem *item = itemAt(event->pos());
    if (!item) {
        QListWidget::mouseDoubleClickEvent(event);
        return;
    }

    // THÊM MỚI: Xử lý double-click chuột trái
    if (event->button() == Qt::LeftButton) {
        emit itemDoubleClicked(item);
        event->accept();
        return;
    }
    
    if (event->button() == Qt::RightButton) {
        emit itemQuickExportRequested(item);
        event->accept();
        return;
    }

    QListWidget::mouseDoubleClickEvent(event);
}

void LibraryWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            QFileInfo fileInfo(url.toLocalFile());
            QString suffix = fileInfo.suffix().toLower();
            if (suffix == "png" || suffix == "jpg" || suffix == "jpeg" || suffix == "bmp") {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void LibraryWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

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
