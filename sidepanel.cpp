// sidepanel.cpp - Version 2.8 (Thêm phím Delete)
// Change-log:
// - Version 2.8:
//   - Thêm logic xử lý xóa ảnh bằng phím Delete.
// - Version 2.7: Cải tiến logic toàn diện.

#include "sidepanel.h"
#include "librarypanel.h"
#include "viewpanel.h"
#include "stylepanel.h"
#include "exportpanel.h"
#include "librarywidget.h"
#include "cropdialog.h"
#include "imageviewerdialog.h" 

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QThreadPool>
#include <QListWidget>

SidePanel::SidePanel(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void SidePanel::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_libraryPanel = new LibraryPanel(this);
    m_stylePanel = new StylePanel(this);
    m_exportPanel = new ExportPanel(this);

    m_viewPanel = new ViewPanel();
    TitleEventFilter* viewTitleFilter = new TitleEventFilter(this);
    QGroupBox *viewBox = new QGroupBox("Xem", this);
    viewBox->setToolTip("Hiển thị ảnh ghép từ các ảnh đã chọn trong thư viện.\n"
                        "- Lăn chuột để phóng to/thu nhỏ.\n"
                        "- Nhấn 'Xem' để mở cửa sổ cắt ảnh ghép này.");
    viewBox->installEventFilter(viewTitleFilter);
    QVBoxLayout *viewLayout = new QVBoxLayout(viewBox);
    
    QPushButton *cropButton = new QPushButton("Xem");
    cropButton->setToolTip("Mở cửa sổ xem và cắt ảnh ghép");
    cropButton->setStyleSheet("background-color: #2980b9; color: white; border: none; padding: 5px; border-radius: 3px;");
    
    QPushButton *fitButton = new QPushButton("Vừa khung");
    fitButton->setToolTip("Thu phóng ảnh vừa với khung xem (95%)");
    QPushButton *oneToOneButton = new QPushButton("1:1");
    oneToOneButton->setToolTip("Xem ảnh với kích thước thật (100%)");

    QLineEdit* viewScaleLabel = new QLineEdit("100%");
    viewScaleLabel->setReadOnly(true);
    viewScaleLabel->setAlignment(Qt::AlignCenter);
    viewScaleLabel->setFixedSize(50, 22);
    viewScaleLabel->setToolTip("Tỉ lệ phóng hiện tại");
    viewScaleLabel->setStyleSheet("background-color: #2c3e50; color: white; border: 1px solid #606060; selection-background-color: transparent;");

    QLineEdit* viewSizeLabel = new QLineEdit("0x0");
    viewSizeLabel->setReadOnly(true);
    viewSizeLabel->setAlignment(Qt::AlignCenter);
    viewSizeLabel->setFixedWidth(80);
    viewSizeLabel->setToolTip("Kích thước ảnh ghép (pixel)");
    viewSizeLabel->setStyleSheet("background-color: #2c3e50; color: white; border: 1px solid #606060; selection-background-color: transparent;");

    QHBoxLayout *viewControlsLayout = new QHBoxLayout();
    viewControlsLayout->addWidget(fitButton);
    viewControlsLayout->addWidget(oneToOneButton);
    viewControlsLayout->addWidget(viewScaleLabel);
    viewControlsLayout->addWidget(viewSizeLabel);
    viewControlsLayout->addStretch();
    viewControlsLayout->addWidget(cropButton);

    viewLayout->addLayout(viewControlsLayout);
    viewLayout->addWidget(m_viewPanel);

    connect(fitButton, &QPushButton::clicked, m_viewPanel, &ViewPanel::fitToWindow);
    connect(oneToOneButton, &QPushButton::clicked, m_viewPanel, &ViewPanel::setOneToOne);
    connect(cropButton, &QPushButton::clicked, this, &SidePanel::onViewPanelCrop);
    connect(m_viewPanel, &ViewPanel::scaleChanged, this, [viewScaleLabel](double scale){
        viewScaleLabel->setText(QString::number(qRound(scale * 100)) + "%");
    });
    connect(m_viewPanel, &ViewPanel::compositedImageSizeChanged, this, [viewSizeLabel](const QSize &size){
        viewSizeLabel->setText(QString("%1x%2").arg(size.width()).arg(size.height()));
    });

    mainLayout->addWidget(m_libraryPanel, 2); 
    mainLayout->addWidget(viewBox, 4);   
    mainLayout->addWidget(m_stylePanel, 0);
    mainLayout->addWidget(m_exportPanel, 0);

    // --- Connections ---
    connect(m_libraryPanel, &LibraryPanel::viewAndCropClicked, this, &SidePanel::onViewAndCropItem);
    connect(m_libraryPanel, &LibraryPanel::deleteCheckedClicked, this, &SidePanel::onDeleteChecked);
    connect(m_libraryPanel->getLibraryWidget(), &LibraryWidget::deleteRequested, this, &SidePanel::onDeleteSelection); // Kết nối signal mới
    connect(m_libraryPanel, &LibraryPanel::quickExportRequested, this, &SidePanel::onQuickExportItem);
    connect(m_libraryPanel, &LibraryPanel::itemsChanged, this, &SidePanel::onLibraryItemsChanged);
    connect(m_libraryPanel, &LibraryPanel::itemDoubleClicked, this, &SidePanel::onItemDoubleClicked);
    
    connect(m_libraryPanel, &LibraryPanel::addImagesClicked, this, &SidePanel::addImagesToLibraryRequested);
    connect(m_libraryPanel, &LibraryPanel::imagesDropped, this, &SidePanel::newImagesDropped);

    connect(m_stylePanel, &StylePanel::styleChanged, this, &SidePanel::applyStylesToViewPanel);

    connect(m_exportPanel, &ExportPanel::exportClicked, this, &SidePanel::onExportClicked);
}

// === GIẢI PHÁP: Thêm phím Delete ===
void SidePanel::onDeleteSelection()
{
    LibraryWidget* lw = m_libraryPanel->getLibraryWidget();
    QList<QListWidgetItem*> itemsToDelete = lw->selectedItems();

    if (itemsToDelete.isEmpty()) return;

    int ret = QMessageBox::question(this, "Xác nhận xoá", 
        QString("Bạn có chắc muốn xoá %1 ảnh đã chọn?").arg(itemsToDelete.count()), 
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        for (QListWidgetItem* item : itemsToDelete) {
            QString filePath = item->data(Qt::UserRole).toString();
            emit fileDeleted(filePath); 
            delete lw->takeItem(lw->row(item));
        }
        onLibraryItemsChanged(nullptr);
    }
}

// ... (Các hàm còn lại không thay đổi) ...
void SidePanel::onItemDoubleClicked(QListWidgetItem* item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    QImage image(filePath);
    if (!image.isNull()) {
        ImageViewerDialog dialog(image, this);
        dialog.exec();
    }
}

void SidePanel::onViewPanelCrop()
{
    QImage imageToCrop = m_viewPanel->getCompositedImage();
    if (imageToCrop.isNull()) {
        QMessageBox::information(this, "Thông báo", "Không có ảnh nào trong vùng xem để cắt.");
        return;
    }
    CropDialog dialog(imageToCrop, this);
    connect(&dialog, &CropDialog::exportImageRequested, this, &SidePanel::exportImageRequested);

    if (dialog.exec() == QDialog::Accepted) {
        QImage finalImage = dialog.getFinalImage();
        if(!finalImage.isNull()) {
            m_viewPanel->setImages({finalImage});
            m_viewPanel->fitToWindow();
        }
    }
}

void SidePanel::onViewAndCropItem(QListWidgetItem* item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    QImage imageToCrop(filePath);

    if (!imageToCrop.isNull()) {
        CropDialog dialog(imageToCrop, this);
        connect(&dialog, &CropDialog::exportImageRequested, this, &SidePanel::exportImageRequested);
        
        if (dialog.exec() == QDialog::Accepted) {
            QImage finalImage = dialog.getFinalImage();
            if (!finalImage.isNull()) {
                QThreadPool::globalInstance()->start([this, filePath, finalImage]() {
                    bool success = finalImage.save(filePath, "PNG");
                    QMetaObject::invokeMethod(this, "onCroppedImageSaveFinished", Qt::QueuedConnection,
                                              Q_ARG(bool, success),
                                              Q_ARG(QString, filePath),
                                              Q_ARG(QImage, finalImage));
                });
            }
        }
    }
}

void SidePanel::onDeleteChecked()
{
    LibraryWidget* lw = m_libraryPanel->getLibraryWidget();
    QList<QListWidgetItem*> itemsToDelete;
    for (int i = 0; i < lw->count(); ++i) {
        if (lw->item(i)->checkState() == Qt::Checked) {
            itemsToDelete.append(lw->item(i));
        }
    }

    if (itemsToDelete.isEmpty()) return;

    int ret = QMessageBox::question(this, "Xác nhận xoá", 
        QString("Bạn có chắc muốn xoá %1 ảnh đã đánh dấu?").arg(itemsToDelete.count()), 
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        for (QListWidgetItem* item : itemsToDelete) {
            QString filePath = item->data(Qt::UserRole).toString();
            emit fileDeleted(filePath); 
            delete lw->takeItem(lw->row(item));
        }
        onLibraryItemsChanged(nullptr);
    }
}

void SidePanel::onQuickExportItem(QListWidgetItem* item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    QImage imageToExport(filePath);
    if (!imageToExport.isNull()) {
        emit exportImageRequested(imageToExport);
    }
}

void SidePanel::onLibraryItemsChanged(QListWidgetItem* item)
{
    Q_UNUSED(item);
    QList<QImage> checkedImages;
    LibraryWidget* lw = m_libraryPanel->getLibraryWidget();
    for (int i = 0; i < lw->count(); ++i) {
        QListWidgetItem* currentItem = lw->item(i);
        if (currentItem->checkState() == Qt::Checked) {
            QString filePath = currentItem->data(Qt::UserRole).toString();
            checkedImages.append(QImage(filePath));
        }
    }
    m_viewPanel->setImages(checkedImages);
    if (!checkedImages.isEmpty()) {
        m_viewPanel->fitToWindow();
    }
}

void SidePanel::applyStylesToViewPanel(const StyleOptions& options)
{
    m_viewPanel->setLayoutType(options.layoutType);
    m_viewPanel->setGridColumnCount(options.gridColumnCount);
    m_viewPanel->setSizingMode(options.sizingMode);
    m_viewPanel->setCustomSize(options.customSize.width(), options.customSize.height());
    m_viewPanel->setBorder(options.border);
    m_viewPanel->setCornerRadius(options.cornerRadius);
    m_viewPanel->setSpacing(options.spacing);
    m_viewPanel->setBackgroundColor(options.backgroundColor);
    
    m_viewPanel->fitToWindow();
}

void SidePanel::onExportClicked()
{
    QImage finalImage = m_viewPanel->getCompositedImage();
    if (!finalImage.isNull()) {
        emit exportImageRequested(finalImage);
    } else {
        QMessageBox::warning(this, "Lỗi", "Không có ảnh để xuất.");
    }
}

void SidePanel::onCroppedImageSaveFinished(bool success, const QString& filePath, const QImage& savedImage)
{
    if (success) {
        LibraryWidget* lw = m_libraryPanel->getLibraryWidget();
        for (int i = 0; i < lw->count(); ++i) {
            QListWidgetItem* item = lw->item(i);
            if (item->data(Qt::UserRole).toString() == filePath) {
                QPixmap thumbnail = QPixmap::fromImage(savedImage.scaled(lw->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                item->setIcon(QIcon(thumbnail));
                onLibraryItemsChanged(item);
                break;
            }
        }
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể lưu ảnh đã cắt.");
    }
}

LibraryWidget* SidePanel::getLibraryWidget() const { return m_libraryPanel->getLibraryWidget(); }
ViewPanel* SidePanel::getViewPanel() const { return m_viewPanel; }
ExportPanel* SidePanel::getExportPanel() const { return m_exportPanel; }
