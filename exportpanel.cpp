// exportpanel.cpp - Version 1.0
#include "exportpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QStyle>
#include <QFileDialog>
#include <QDesktopServices>

ExportPanel::ExportPanel(QWidget *parent) : QWidget(parent)
{
    m_titleFilter = new TitleEventFilter(this);
    setupUi();
}

void ExportPanel::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

    QGroupBox *exportBox = new QGroupBox("Xuất", this);
    exportBox->setToolTip("Lưu ảnh ghép thành file.\n"
                         "- Chọn thư mục lưu, định dạng và nhấn 'Xuất ảnh'.");
    exportBox->installEventFilter(m_titleFilter);
    QVBoxLayout *exportLayout = new QVBoxLayout(exportBox);
    m_savePathEdit = new QLineEdit();
    m_savePathEdit->setReadOnly(true);
    m_savePathEdit->setToolTip("Thư mục sẽ lưu ảnh xuất ra");
    QPushButton *changePathButton = new QPushButton("Thay đổi");
    changePathButton->setToolTip("Chọn thư mục để lưu ảnh");
    QPushButton *openFolderButton = new QPushButton();
    openFolderButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    openFolderButton->setToolTip("Mở thư mục lưu hiện tại");
    m_exportButton = new QPushButton("Xuất ảnh");
    m_exportButton->setToolTip("Lưu ảnh ghép trong panel 'Xem' thành file");
    m_exportButton->setStyleSheet("background-color: #e67e22; color: white; border: none; padding: 5px; border-radius: 3px;");
    m_formatComboBox = new QComboBox();
    m_formatComboBox->setToolTip("Chọn định dạng file ảnh để lưu");
    m_formatComboBox->addItems({"PNG", "JPG", "BMP", "TIFF", "WEBP"});

    QHBoxLayout *saveLineLayout = new QHBoxLayout();
    saveLineLayout->addWidget(new QLabel("Nơi lưu:"));
    saveLineLayout->addWidget(m_savePathEdit, 1);
    saveLineLayout->addWidget(changePathButton);
    saveLineLayout->addWidget(openFolderButton);

    QHBoxLayout *formatLineLayout = new QHBoxLayout();
    formatLineLayout->addWidget(new QLabel("Định dạng:"));
    formatLineLayout->addWidget(m_formatComboBox);
    formatLineLayout->addStretch();
    formatLineLayout->addWidget(m_exportButton);

    exportLayout->addLayout(saveLineLayout);
    exportLayout->addLayout(formatLineLayout);
    
    mainLayout->addWidget(exportBox);

    // --- Connections ---
    connect(m_exportButton, &QPushButton::clicked, this, &ExportPanel::exportClicked);
    connect(changePathButton, &QPushButton::clicked, this, [this](){
        QString dir = QFileDialog::getExistingDirectory(this, "Chọn thư mục lưu", m_savePathEdit->text());
        if (!dir.isEmpty()) {
            m_savePathEdit->setText(dir);
        }
    });
    connect(openFolderButton, &QPushButton::clicked, this, [this](){
        QString path = m_savePathEdit->text();
        if (!path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });
}

QString ExportPanel::getSavePath() const
{
    return m_savePathEdit->text();
}

QString ExportPanel::getSelectedFormat() const
{
    return m_formatComboBox->currentText();
}

void ExportPanel::setSavePath(const QString& path)
{
    m_savePathEdit->setText(path);
}
