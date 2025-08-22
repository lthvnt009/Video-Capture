// viewpanel.h - Version 1.6
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

    explicit ViewPanel(QWidget *parent = nullptr);

    QImage getCompositedImage() const;

signals:
    // SỬA LỖI: Bổ sung signal còn thiếu
    void scaleChanged(double newScale);

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

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QSize calculateTotalSize() const;
    int findBestColumnCount() const;

    QList<QImage> m_images;
    LayoutType m_layoutType = Horizontal;
    int m_spacing = 5;
    double m_scale = 1.0;

    int m_border = 0;
    int m_cornerRadius = 0;
    QColor m_backgroundColor = QColor("#333333");
};

#endif // VIEWPANEL_H
