// librarypanel.cpp - Version 1.2 (Đổi tên nút)
#include "librarypanel.h"
#include "librarywidget.h"
#include "libraryitemdelegate.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QListWidget>

LibraryPanel::LibraryPanel(QWidget *parent) : QWidget(parent)
{
    m_titleFilter = new TitleEventFilter(this);
    setupUi();
}

LibraryWidget* LibraryPanel::getLibraryWidget() const
{
    return m_libraryWidget;
}

void LibraryPanel::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

    QGroupBox *libraryBox = new QGroupBox("Thư viện", this);
    libraryBox->setToolTip("Quản lý các ảnh đã chụp hoặc thêm từ bên ngoài.\n"
                           "- Double-click: Xem ảnh chi tiết.\n"
                           "- Double-click chuột phải: Xuất nhanh ảnh.\n"
                           "- Kéo thả file ảnh vào đây để thêm.");
    libraryBox->installEventFilter(m_titleFilter);
    QVBoxLayout *libraryLayout = new QVBoxLayout(libraryBox);
    
    m_libraryWidget = new LibraryWidget(this);
    m_libraryDelegate = new LibraryItemDelegate(this);
    m_libraryWidget->setItemDelegate(m_libraryDelegate);
    m_libraryWidget->setViewMode(QListWidget::IconMode);
    m_libraryWidget->setIconSize(QSize(128, 72));
    m_libraryWidget->setWordWrap(true);
    m_libraryWidget->setSpacing(2); 
    m_libraryWidget->setGridSize(QSize(m_libraryWidget->iconSize().width() + 4, m_libraryWidget->iconSize().height() + 4));
    m_libraryWidget->setStyleSheet("QListWidget::item { padding: 1px; margin: 0px; border: 0px; }");
    
    m_addImagesButton = new QPushButton("Thêm");
    m_addImagesButton->setToolTip("Thêm ảnh từ máy tính vào thư viện");
    m_addImagesButton->setStyleSheet("background-color: #16a085; color: white; border: none; padding: 5px; border-radius: 3px;");

    // SỬA LỖI: Đổi tên nút
    m_viewAndCropButton = new QPushButton("Xem");
    m_viewAndCropButton->setToolTip("Mở cửa sổ Cắt ảnh cho ảnh đã chọn");
    m_viewAndCropButton->setStyleSheet("background-color: #2980b9; color: white; border: none; padding: 5px; border-radius: 3px;");
    m_deleteButton = new QPushButton("Xoá");
    m_deleteButton->setToolTip("Xoá ảnh đã chọn khỏi thư viện");
    m_deleteButton->setStyleSheet("background-color: #c0392b; color: white; border: none; padding: 5px; border-radius: 3px;");
    m_viewAndCropButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    QHBoxLayout *libraryButtonsLayout = new QHBoxLayout();
    libraryButtonsLayout->addStretch();
    libraryButtonsLayout->addWidget(m_viewAndCropButton);
    libraryButtonsLayout->addWidget(m_addImagesButton);
    libraryButtonsLayout->addWidget(m_deleteButton);
    libraryLayout->addLayout(libraryButtonsLayout);
    libraryLayout->addWidget(m_libraryWidget);

    mainLayout->addWidget(libraryBox);

    // --- Connections ---
    connect(m_addImagesButton, &QPushButton::clicked, this, &LibraryPanel::addImagesClicked);
    connect(m_viewAndCropButton, &QPushButton::clicked, this, [this](){
        if (!m_libraryWidget->selectedItems().isEmpty()) {
            emit viewAndCropClicked(m_libraryWidget->selectedItems().first());
        }
    });
    connect(m_deleteButton, &QPushButton::clicked, this, [this](){
        if (!m_libraryWidget->selectedItems().isEmpty()) {
            emit deleteClicked(m_libraryWidget->selectedItems().first());
        }
    });

    connect(m_libraryWidget, &LibraryWidget::itemQuickExportRequested, this, &LibraryPanel::quickExportRequested);
    connect(m_libraryWidget, &LibraryWidget::imagesDropped, this, &LibraryPanel::imagesDropped);
    connect(m_libraryWidget, &QListWidget::itemChanged, this, &LibraryPanel::itemsChanged);
    connect(m_libraryWidget, &QListWidget::itemSelectionChanged, this, [this](){
        bool hasSelection = !m_libraryWidget->selectedItems().isEmpty();
        m_viewAndCropButton->setEnabled(hasSelection);
        m_deleteButton->setEnabled(hasSelection);
        emit selectionChanged();
    });
    connect(m_libraryWidget, &LibraryWidget::itemDoubleClicked, this, &LibraryPanel::itemDoubleClicked);
}
