// stylepanel.cpp - Version 1.2 (Sửa lỗi biên dịch)
#include "stylepanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QSlider>
#include <QToolButton>
#include <QColorDialog>
#include <QPushButton> 

StylePanel::StylePanel(QWidget *parent) 
    : QWidget(parent), m_backgroundColor("#333333")
{
    m_titleFilter = new TitleEventFilter(this);
    setupUi();
}

void StylePanel::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

    QGroupBox *styleBox = new QGroupBox("Kiểu", this);
    styleBox->setToolTip("Tùy chỉnh bố cục và giao diện của ảnh ghép.\n"
                         "- Bố cục: Chọn cách sắp xếp các ảnh (Ngang, Dọc, Lưới).\n"
                         "- Kích thước ảnh: Đồng bộ kích thước các ảnh.\n"
                         "- Trang trí: Thêm viền, bo góc, khoảng cách và màu nền.");
    styleBox->installEventFilter(m_titleFilter);
    QVBoxLayout *mainStyleLayout = new QVBoxLayout(styleBox);

    // -- Bố cục --
    QGroupBox *layoutTypeBox = new QGroupBox("Bố cục");
    layoutTypeBox->installEventFilter(m_titleFilter);
    QGridLayout *radioLayout = new QGridLayout(layoutTypeBox);
    m_radioHorizontal = new QRadioButton("Ngang");
    m_radioVertical = new QRadioButton("Dọc");
    m_radioGrid = new QRadioButton("Lưới");
    m_radioHorizontal->setChecked(true);
    
    m_gridModeGroup = new QButtonGroup(this);
    m_gridAutoRadio = new QRadioButton("Tự động");
    m_gridColumnRadio = new QRadioButton("Cột:");
    m_gridModeGroup->addButton(m_gridAutoRadio);
    m_gridModeGroup->addButton(m_gridColumnRadio);
    m_gridAutoRadio->setChecked(true);

    m_gridColumnCountCombo = new QComboBox();
    for(int i = 1; i <= 20; ++i) m_gridColumnCountCombo->addItem(QString::number(i));
    
    radioLayout->addWidget(m_radioHorizontal, 0, 0);
    radioLayout->addWidget(m_radioVertical, 0, 1);
    radioLayout->addWidget(m_radioGrid, 0, 2);
    radioLayout->setColumnStretch(3, 1); 
    radioLayout->addWidget(m_gridAutoRadio, 0, 4);
    radioLayout->addWidget(m_gridColumnRadio, 0, 5);
    radioLayout->addWidget(m_gridColumnCountCombo, 0, 6);
    mainStyleLayout->addWidget(layoutTypeBox);

    // -- Tùy chỉnh chi tiết --
    QHBoxLayout* detailLayout = new QHBoxLayout();
    
    QGroupBox *sizingBox = new QGroupBox("Kích thước ảnh");
    sizingBox->installEventFilter(m_titleFilter);
    QVBoxLayout *sizingLayout = new QVBoxLayout(sizingBox);
    m_sizingGroup = new QButtonGroup(this);
    m_sizeOriginalRadio = new QRadioButton("Gốc");
    m_sizeMatchFirstRadio = new QRadioButton("Đầu tiên");
    m_sizeCustomRadio = new QRadioButton("Tuỳ chỉnh");
    m_sizingGroup->addButton(m_sizeOriginalRadio);
    m_sizingGroup->addButton(m_sizeMatchFirstRadio);
    m_sizingGroup->addButton(m_sizeCustomRadio);
    m_sizeOriginalRadio->setChecked(true);
    sizingLayout->addWidget(m_sizeOriginalRadio);
    sizingLayout->addWidget(m_sizeMatchFirstRadio);
    sizingLayout->addWidget(m_sizeCustomRadio);

    m_customSizeContainer = new QWidget();
    QHBoxLayout *customSizeLayout = new QHBoxLayout(m_customSizeContainer);
    customSizeLayout->setContentsMargins(15, 0, 0, 0);
    m_customWidthSpinBox = new QSpinBox();
    m_customWidthSpinBox->setRange(10, 8000); m_customWidthSpinBox->setValue(1280);
    m_customHeightSpinBox = new QSpinBox();
    m_customHeightSpinBox->setRange(10, 8000); m_customHeightSpinBox->setValue(720);
    m_customSizeLabelW = new QLabel("Ngang:");
    m_customSizeLabelH = new QLabel("Cao:");
    
    QHBoxLayout* customWLayout = new QHBoxLayout();
    customWLayout->addWidget(m_customSizeLabelW);
    customWLayout->addWidget(createVerticalSpinBox(m_customWidthSpinBox));
    
    QHBoxLayout* customHLayout = new QHBoxLayout();
    customHLayout->addWidget(m_customSizeLabelH);
    customHLayout->addWidget(createVerticalSpinBox(m_customHeightSpinBox));

    customSizeLayout->addLayout(customWLayout);
    customSizeLayout->addLayout(customHLayout);
    customSizeLayout->addStretch();
    sizingLayout->addWidget(m_customSizeContainer);
    sizingLayout->addStretch();
    detailLayout->addWidget(sizingBox);

    QGroupBox *decorationBox = new QGroupBox("Trang trí");
    decorationBox->installEventFilter(m_titleFilter);
    QGridLayout *decorationLayout = new QGridLayout(decorationBox);
    
    m_borderSpinBox = new QSpinBox();
    m_borderSpinBox->setRange(0, 100); m_borderSpinBox->setValue(0); m_borderSpinBox->setFixedWidth(40);
    m_borderSlider = new QSlider(Qt::Horizontal);
    m_borderSlider->setRange(0, 100);
    decorationLayout->addWidget(new QLabel("Viền (px):"), 0, 0);
    decorationLayout->addWidget(createVerticalSpinBox(m_borderSpinBox), 0, 1);
    decorationLayout->addWidget(m_borderSlider, 0, 2);

    m_cornerRadiusSpinBox = new QSpinBox();
    m_cornerRadiusSpinBox->setRange(0, 100); m_cornerRadiusSpinBox->setValue(0); m_cornerRadiusSpinBox->setFixedWidth(40);
    m_cornerRadiusSlider = new QSlider(Qt::Horizontal);
    m_cornerRadiusSlider->setRange(0, 100);
    decorationLayout->addWidget(new QLabel("Bo góc (%):"), 1, 0);
    decorationLayout->addWidget(createVerticalSpinBox(m_cornerRadiusSpinBox), 1, 1);
    decorationLayout->addWidget(m_cornerRadiusSlider, 1, 2);

    m_spacingSpinBox = new QSpinBox();
    m_spacingSpinBox->setRange(0, 100); m_spacingSpinBox->setValue(0); m_spacingSpinBox->setFixedWidth(40);
    m_spacingSlider = new QSlider(Qt::Horizontal);
    m_spacingSlider->setRange(0, 100);
    decorationLayout->addWidget(new QLabel("Cách (px):"), 2, 0);
    decorationLayout->addWidget(createVerticalSpinBox(m_spacingSpinBox), 2, 1);
    decorationLayout->addWidget(m_spacingSlider, 2, 2);

    QHBoxLayout *bgColorLayout = new QHBoxLayout();
    m_colorSwatch = new ClickableFrame();
    m_colorSwatch->setFrameShape(QFrame::Box); m_colorSwatch->setFrameShadow(QFrame::Sunken); m_colorSwatch->setAutoFillBackground(true); m_colorSwatch->setFixedSize(22, 22);
    QPalette pal = m_colorSwatch->palette(); pal.setColor(QPalette::Window, m_backgroundColor); m_colorSwatch->setPalette(pal);
    QPushButton* changeColorButton = new QPushButton("Thay đổi");
    changeColorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bgColorLayout->addWidget(m_colorSwatch);
    bgColorLayout->addWidget(changeColorButton);
    bgColorLayout->addStretch();
    decorationLayout->addWidget(new QLabel("Nền:"), 3, 0);
    decorationLayout->addLayout(bgColorLayout, 3, 1, 1, 2);
    detailLayout->addWidget(decorationBox);

    mainStyleLayout->addLayout(detailLayout);
    mainLayout->addWidget(styleBox);

    // --- Connections ---
    connect(m_radioHorizontal, &QRadioButton::toggled, this, &StylePanel::onStyleOptionChanged);
    connect(m_radioVertical, &QRadioButton::toggled, this, &StylePanel::onStyleOptionChanged);
    connect(m_radioGrid, &QRadioButton::toggled, this, &StylePanel::onStyleOptionChanged);
    connect(m_gridModeGroup, &QButtonGroup::buttonClicked, this, &StylePanel::onStyleOptionChanged);
    connect(m_gridColumnCountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StylePanel::onStyleOptionChanged);
    connect(m_sizingGroup, &QButtonGroup::buttonClicked, this, &StylePanel::onStyleOptionChanged);
    connect(m_customWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &StylePanel::onStyleOptionChanged);
    connect(m_customHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &StylePanel::onStyleOptionChanged);
    
    connect(m_spacingSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_spacingSlider, &QSlider::setValue);
    connect(m_spacingSlider, &QSlider::valueChanged, m_spacingSpinBox, &QSpinBox::setValue);
    connect(m_spacingSlider, &QSlider::valueChanged, this, &StylePanel::onStyleOptionChanged);
    
    connect(m_borderSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_borderSlider, &QSlider::setValue);
    connect(m_borderSlider, &QSlider::valueChanged, m_borderSpinBox, &QSpinBox::setValue);
    connect(m_borderSlider, &QSlider::valueChanged, this, &StylePanel::onStyleOptionChanged);
    
    connect(m_cornerRadiusSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_cornerRadiusSlider, &QSlider::setValue);
    connect(m_cornerRadiusSlider, &QSlider::valueChanged, m_cornerRadiusSpinBox, &QSpinBox::setValue);
    connect(m_cornerRadiusSlider, &QSlider::valueChanged, this, &StylePanel::onStyleOptionChanged);

    connect(changeColorButton, &QPushButton::clicked, this, [this](){
        QColor color = QColorDialog::getColor(m_backgroundColor, this, "Chọn màu nền");
        if (color.isValid()) {
            m_backgroundColor = color;
            QPalette pal = m_colorSwatch->palette();
            pal.setColor(QPalette::Window, m_backgroundColor);
            m_colorSwatch->setPalette(pal);
            onStyleOptionChanged();
        }
    });
    connect(m_colorSwatch, &ClickableFrame::clicked, changeColorButton, &QPushButton::click);

    onStyleOptionChanged(); // Gửi tín hiệu ban đầu
}

void StylePanel::onStyleOptionChanged()
{
    // Cập nhật UI nội bộ
    bool isGrid = m_radioGrid->isChecked();
    m_gridAutoRadio->setEnabled(isGrid);
    m_gridColumnRadio->setEnabled(isGrid);
    m_gridColumnCountCombo->setEnabled(isGrid && m_gridColumnRadio->isChecked());

    bool isCustom = m_sizeCustomRadio->isChecked();
    m_customSizeContainer->setVisible(isCustom);

    if (m_radioHorizontal->isChecked()) {
        m_sizeCustomRadio->setText("Cao");
        m_customSizeLabelW->hide(); m_customWidthSpinBox->hide();
        m_customSizeLabelH->show(); m_customHeightSpinBox->show();
    } else if (m_radioVertical->isChecked()) {
        m_sizeCustomRadio->setText("Ngang");
        m_customSizeLabelW->show(); m_customWidthSpinBox->show();
        m_customSizeLabelH->hide(); m_customHeightSpinBox->hide();
    } else { // Grid
        m_sizeCustomRadio->setText("Cỡ");
        m_customSizeLabelW->show(); m_customWidthSpinBox->show();
        m_customSizeLabelH->show(); m_customHeightSpinBox->show();
    }

    // Đóng gói và gửi dữ liệu
    StyleOptions opts;
    if(m_radioHorizontal->isChecked()) opts.layoutType = ViewPanel::Horizontal;
    else if(m_radioVertical->isChecked()) opts.layoutType = ViewPanel::Vertical;
    else opts.layoutType = ViewPanel::Grid;

    opts.gridColumnCount = m_gridColumnRadio->isChecked() ? m_gridColumnCountCombo->currentText().toInt() : 0;

    if(m_sizeOriginalRadio->isChecked()) opts.sizingMode = ViewPanel::Original;
    else if(m_sizeMatchFirstRadio->isChecked()) opts.sizingMode = ViewPanel::MatchFirst;
    else opts.sizingMode = ViewPanel::Custom;

    opts.customSize = QSize(m_customWidthSpinBox->value(), m_customHeightSpinBox->value());
    opts.border = m_borderSpinBox->value();
    opts.cornerRadius = m_cornerRadiusSpinBox->value();
    opts.spacing = m_spacingSpinBox->value();
    opts.backgroundColor = m_backgroundColor;

    emit styleChanged(opts);
}

QWidget* StylePanel::createVerticalSpinBox(QSpinBox* spinbox) {
    QWidget *container = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(2);
    spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(0);
    QToolButton *upButton = new QToolButton();
    upButton->setArrowType(Qt::UpArrow);
    upButton->setFixedSize(12, 11);
    QToolButton *downButton = new QToolButton();
    downButton->setArrowType(Qt::DownArrow);
    downButton->setFixedSize(12, 11);
    buttonLayout->addWidget(upButton);
    buttonLayout->addWidget(downButton);
    layout->addWidget(spinbox);
    layout->addLayout(buttonLayout);
    connect(upButton, &QToolButton::clicked, spinbox, &QSpinBox::stepUp);
    connect(downButton, &QToolButton::clicked, spinbox, &QSpinBox::stepDown);
    return container;
}
