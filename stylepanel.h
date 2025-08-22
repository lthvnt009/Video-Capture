// stylepanel.h - Version 1.0
#ifndef STYLEPANEL_H
#define STYLEPANEL_H

#include <QWidget>
#include "viewpanel.h" // for enums
#include "helpers.h"

class QRadioButton;
class QButtonGroup;
class QComboBox;
class QSpinBox;
class QLabel;
class QSlider;

// Struct để đóng gói tất cả các tùy chọn style
struct StyleOptions {
    ViewPanel::LayoutType layoutType;
    int gridColumnCount;
    ViewPanel::SizingMode sizingMode;
    QSize customSize;
    int border;
    int cornerRadius;
    int spacing;
    QColor backgroundColor;
};

class StylePanel : public QWidget
{
    Q_OBJECT

public:
    explicit StylePanel(QWidget *parent = nullptr);

signals:
    void styleChanged(const StyleOptions& options);

private slots:
    void onStyleOptionChanged();

private:
    void setupUi();
    QWidget* createVerticalSpinBox(QSpinBox* spinbox);

    TitleEventFilter* m_titleFilter;
    
    // Layout
    QRadioButton *m_radioGrid, *m_radioVertical, *m_radioHorizontal;
    QButtonGroup *m_gridModeGroup;
    QRadioButton *m_gridAutoRadio, *m_gridColumnRadio;
    QComboBox *m_gridColumnCountCombo;
    
    // Sizing
    QButtonGroup *m_sizingGroup;
    QRadioButton *m_sizeOriginalRadio, *m_sizeMatchFirstRadio, *m_sizeCustomRadio;
    QWidget *m_customSizeContainer;
    QSpinBox *m_customWidthSpinBox, *m_customHeightSpinBox;
    QLabel *m_customSizeLabelW, *m_customSizeLabelH;
    
    // Decoration
    QSpinBox *m_borderSpinBox;
    QSlider *m_borderSlider;
    QSpinBox *m_cornerRadiusSpinBox;
    QSlider *m_cornerRadiusSlider;
    QSpinBox *m_spacingSpinBox;
    QSlider* m_spacingSlider;
    ClickableFrame *m_colorSwatch;
    QColor m_backgroundColor;
};

#endif // STYLEPANEL_H
