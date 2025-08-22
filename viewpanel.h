// viewpanel.h - Version 2.3 (Grid Columns & Size Signal)
#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include <QWidget>
#include <QList>
#include <QImage>
#include <QSize>
#include <QWheelEvent>
#include <QColor>

class ViewPanel : public QWidget
{
    Q_OBJECT

public:
    enum LayoutType { Grid, Vertical, Horizontal };
    enum SizingMode { Original, MatchFirst, Custom };

    explicit ViewPanel(QWidget *parent = nullptr);

    QImage getCompositedImage() const;

signals:
    void scaleChanged(double newScale);
    // THÊM MỚI: Signal để gửi kích thước ảnh ghép
    void compositedImageSizeChanged(const QSize &size);

public slots:
    void setImages(const QList<QImage> &images);
    void setLayoutType(LayoutType type);
    void setSpacing(int spacing);
    void setScale(double newScale);
    void fitToWindow();
    void setOneToOne();
    void setBorder(int border);
    void setCornerRadius(int radius);
    void setBackgroundColor(const QColor &color);
    void setSizingMode(SizingMode mode);
    void setCustomSize(int width, int height);
    // THÊM MỚI: Slot để đặt số cột cho chế độ Lưới
    void setGridColumnCount(int count);


protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void processImages(); 
    QSize calculateTotalSize() const;
    int findBestColumnCount() const;

    QList<QImage> m_originalImages;
    QList<QImage> m_processedImages;
    LayoutType m_layoutType = Horizontal;
    int m_spacing = 5;
    double m_scale = 1.0;

    int m_border = 0;
    int m_cornerRadius = 0;
    QColor m_backgroundColor = QColor("#333333");

    SizingMode m_sizingMode = Original;
    int m_customWidth = 0;
    int m_customHeight = 0;
    // THÊM MỚI: Biến lưu số cột, 0 = tự động
    int m_gridColumnCount = 0; 
};

#endif // VIEWPANEL_H
