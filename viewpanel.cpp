// viewpanel.cpp - Version 2.1
#include "viewpanel.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <limits>

ViewPanel::ViewPanel(QWidget *parent) : QWidget(parent)
{
    setBackgroundColor(m_backgroundColor);
}

void ViewPanel::setImages(const QList<QImage> &images)
{
    m_images = images;
    update();
}

void ViewPanel::setLayoutType(LayoutType type)
{
    m_layoutType = type;
    update();
}

void ViewPanel::setSpacing(int spacing)
{
    m_spacing = spacing;
    update();
}

void ViewPanel::setScale(double newScale)
{
    m_scale = qBound(0.01, newScale, 5.0);
    emit scaleChanged(m_scale);
    update();
}

void ViewPanel::fitToWindow()
{
    QSize totalSize = calculateTotalSize();
    if (!totalSize.isValid() || totalSize.isEmpty()) {
        setScale(1.0);
        return;
    }

    double scaleX = (double)this->width() / totalSize.width();
    double scaleY = (double)this->height() / totalSize.height();
    double fitScale = qMin(scaleX, scaleY);
    setScale(fitScale);
}

void ViewPanel::setOneToOne()
{
    setScale(1.0);
}

void ViewPanel::setBorder(int border)
{
    m_border = qMax(0, border);
    update();
}

void ViewPanel::setCornerRadius(int radius)
{
    m_cornerRadius = qMax(0, radius);
    update();
}

void ViewPanel::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    setStyleSheet(QString("background-color: %1;").arg(m_backgroundColor.name()));
    update();
}

QImage ViewPanel::getCompositedImage() const
{
    if (m_images.isEmpty()) {
        return QImage();
    }

    QSize totalSize = calculateTotalSize();
    if (!totalSize.isValid()) return QImage();

    QImage resultImage(totalSize, QImage::Format_ARGB32_Premultiplied);
    resultImage.fill(m_backgroundColor);

    QPainter painter(&resultImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int currentX = m_border;
    int currentY = m_border;
    int maxRowHeight = 0;

    auto drawRoundedImage = [&](const QImage& img, int x, int y) {
        int radius = m_cornerRadius > 0 ? (qMin(img.width(), img.height()) * m_cornerRadius / 100) : 0;
        if (radius > 0) {
            QPainterPath path;
            path.addRoundedRect(QRect(x, y, img.width(), img.height()), radius, radius);
            painter.setClipPath(path);
            painter.drawImage(x, y, img);
            painter.setClipping(false);
        } else {
            painter.drawImage(x, y, img);
        }
    };

    if (m_layoutType == Horizontal) {
        for (const QImage &image : m_images) {
            drawRoundedImage(image, currentX, currentY);
            currentX += image.width() + m_spacing;
        }
    } else if (m_layoutType == Vertical) {
        for (const QImage &image : m_images) {
            drawRoundedImage(image, currentX, currentY);
            currentY += image.height() + m_spacing;
        }
    } else { // Grid
        int cols = findBestColumnCount();
        if (cols == 0) return QImage();
        int current_col = 0;

        for (const QImage &image : m_images) {
            drawRoundedImage(image, currentX, currentY);
            currentX += image.width() + m_spacing;
            maxRowHeight = qMax(maxRowHeight, image.height());
            current_col++;
            if (current_col >= cols) {
                current_col = 0;
                currentX = m_border;
                currentY += maxRowHeight + m_spacing;
                maxRowHeight = 0;
            }
        }
    }

    return resultImage;
}

void ViewPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (m_images.isEmpty()) {
        return;
    }

    QImage compositedImage = getCompositedImage();
    if (compositedImage.isNull()) return;

    QSize scaledSize = compositedImage.size() * m_scale;
    int x = (this->width() - scaledSize.width()) / 2;
    int y = (this->height() - scaledSize.height()) / 2;

    painter.drawImage(QRect(x, y, scaledSize.width(), scaledSize.height()), compositedImage);
}

void ViewPanel::wheelEvent(QWheelEvent *event)
{
    double zoomFactor = 1.15;
    double newScale;
    if (event->angleDelta().y() > 0) {
        newScale = m_scale * zoomFactor;
    } else {
        newScale = m_scale / zoomFactor;
    }
    setScale(newScale);
}

int ViewPanel::findBestColumnCount() const
{
    if (m_images.isEmpty()) return 0;
    if (m_images[0].height() == 0) return 1;
    double imageAspectRatio = (double)m_images[0].width() / m_images[0].height();

    const double targetAspectRatio = 16.0 / 9.0;
    double bestDiff = std::numeric_limits<double>::max();
    int bestCols = 1;
    int imageCount = m_images.count();

    for (int cols = 1; cols <= imageCount; ++cols) {
        int rows = qCeil((double)imageCount / cols);
        double gridAspectRatio = (cols * imageAspectRatio) / rows;
        double diff = qAbs(gridAspectRatio - targetAspectRatio);

        if (diff < bestDiff) {
            bestDiff = diff;
            bestCols = cols;
        }
    }
    return bestCols;
}

QSize ViewPanel::calculateTotalSize() const
{
    if (m_images.isEmpty()) {
        return {0, 0};
    }

    int totalWidth = 0;
    int totalHeight = 0;
    int imageCount = m_images.count();

    // THAY ĐỔI: Sửa logic tính toán để không có viền thừa
    int effectiveSpacing = (imageCount > 1) ? m_spacing : 0;

    if (m_layoutType == Horizontal) {
        totalWidth = (imageCount - 1) * effectiveSpacing;
        int maxHeight = 0;
        for (const QImage &img : m_images) {
            totalWidth += img.width();
            maxHeight = qMax(maxHeight, img.height());
        }
        totalHeight = maxHeight;
    } else if (m_layoutType == Vertical) {
        totalHeight = (imageCount - 1) * effectiveSpacing;
        int maxWidth = 0;
        for (const QImage &img : m_images) {
            totalHeight += img.height();
            maxWidth = qMax(maxWidth, img.width());
        }
        totalWidth = maxWidth;
    } else { // Grid
        int cols = findBestColumnCount();
        if (cols == 0) return {0,0};
        int rows = qCeil((double)imageCount / cols);
        if (rows == 0) return {0,0};

        int imageWidth = m_images[0].width();
        int imageHeight = m_images[0].height();

        totalWidth = cols * imageWidth + (cols > 1 ? (cols - 1) * m_spacing : 0);
        totalHeight = rows * imageHeight + (rows > 1 ? (rows - 1) * m_spacing : 0);
    }
    
    totalWidth += 2 * m_border;
    totalHeight += 2 * m_border;

    return {totalWidth, totalHeight};
}
