// viewpanel.cpp - Version 2.3 (Grid Columns & Size Signal)
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
    m_originalImages = images;
    processImages();
}

void ViewPanel::setLayoutType(LayoutType type)
{
    m_layoutType = type;
    processImages();
}

void ViewPanel::setSizingMode(SizingMode mode)
{
    m_sizingMode = mode;
    processImages();
}

void ViewPanel::setCustomSize(int width, int height)
{
    m_customWidth = width;
    m_customHeight = height;
    if (m_sizingMode == Custom) {
        processImages();
    }
}

void ViewPanel::setGridColumnCount(int count)
{
    m_gridColumnCount = count;
    if (m_layoutType == Grid) {
        processImages();
    }
}


void ViewPanel::processImages()
{
    m_processedImages.clear();
    if (m_originalImages.isEmpty()) {
        update();
        emit compositedImageSizeChanged(QSize(0,0));
        return;
    }

    switch (m_sizingMode) {
        case Original:
            m_processedImages = m_originalImages;
            break;

        case MatchFirst: {
            QImage firstImage = m_originalImages.first();
            m_processedImages.append(firstImage);
            for (int i = 1; i < m_originalImages.count(); ++i) {
                QImage scaledImage;
                if (m_layoutType == Horizontal) {
                    scaledImage = m_originalImages[i].scaledToHeight(firstImage.height(), Qt::SmoothTransformation);
                } else { 
                    scaledImage = m_originalImages[i].scaledToWidth(firstImage.width(), Qt::SmoothTransformation);
                }
                m_processedImages.append(scaledImage);
            }
            break;
        }

        case Custom: {
            for (const QImage &img : m_originalImages) {
                QImage scaledImage;
                if (m_layoutType == Horizontal && m_customHeight > 0) {
                    scaledImage = img.scaledToHeight(m_customHeight, Qt::SmoothTransformation);
                } else if (m_layoutType == Vertical && m_customWidth > 0) {
                    scaledImage = img.scaledToWidth(m_customWidth, Qt::SmoothTransformation);
                } else if (m_layoutType == Grid && m_customWidth > 0 && m_customHeight > 0) {
                    scaledImage = img.scaled(m_customWidth, m_customHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                } else {
                    scaledImage = img;
                }
                m_processedImages.append(scaledImage);
            }
            break;
        }
    }
    update();
    emit compositedImageSizeChanged(calculateTotalSize());
}


void ViewPanel::setSpacing(int spacing)
{
    m_spacing = spacing;
    update();
    emit compositedImageSizeChanged(calculateTotalSize());
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

    // THAY ĐỔI: Thêm 0.95 để tạo padding
    double scaleX = (this->width() * 0.95) / totalSize.width();
    double scaleY = (this->height() * 0.95) / totalSize.height();
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
    emit compositedImageSizeChanged(calculateTotalSize());
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
    if (m_processedImages.isEmpty()) {
        return QImage();
    }

    QSize totalSize = calculateTotalSize();
    if (!totalSize.isValid() || totalSize.isEmpty()) return QImage();

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
        for (const QImage &image : m_processedImages) {
            drawRoundedImage(image, currentX, currentY);
            currentX += image.width() + m_spacing;
        }
    } else if (m_layoutType == Vertical) {
        for (const QImage &image : m_processedImages) {
            drawRoundedImage(image, currentX, currentY);
            currentY += image.height() + m_spacing;
        }
    } else { // Grid
        // THAY ĐỔI: Sử dụng số cột tùy chỉnh hoặc tự động
        int cols = (m_gridColumnCount > 0) ? m_gridColumnCount : findBestColumnCount();
        if (cols == 0) return QImage();
        int current_col = 0;

        for (const QImage &image : m_processedImages) {
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

    if (m_processedImages.isEmpty()) {
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
    if (m_processedImages.isEmpty()) return 0;
    if (m_processedImages[0].height() == 0) return 1;
    double imageAspectRatio = (double)m_processedImages[0].width() / m_processedImages[0].height();

    const double targetAspectRatio = 16.0 / 9.0;
    double bestDiff = std::numeric_limits<double>::max();
    int bestCols = 1;
    int imageCount = m_processedImages.count();

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
    if (m_processedImages.isEmpty()) {
        return {0, 0};
    }

    int totalWidth = 0;
    int totalHeight = 0;
    int imageCount = m_processedImages.count();

    if (m_layoutType == Horizontal) {
        totalWidth = (imageCount > 1) ? (imageCount - 1) * m_spacing : 0;
        int maxHeight = 0;
        for (const QImage &img : m_processedImages) {
            totalWidth += img.width();
            maxHeight = qMax(maxHeight, img.height());
        }
        totalHeight = maxHeight;
    } else if (m_layoutType == Vertical) {
        totalHeight = (imageCount > 1) ? (imageCount - 1) * m_spacing : 0;
        int maxWidth = 0;
        for (const QImage &img : m_processedImages) {
            totalHeight += img.height();
            maxWidth = qMax(maxWidth, img.width());
        }
        totalWidth = maxWidth;
    } else { // Grid
        int cols = (m_gridColumnCount > 0) ? m_gridColumnCount : findBestColumnCount();
        if (cols == 0) return {0,0};
        int rows = qCeil((double)imageCount / cols);
        if (rows == 0) return {0,0};
        
        int current_col = 0;
        int max_row_width = 0;
        int current_row_width = 0;
        int current_row_height = 0;

        for(const QImage& img : m_processedImages) {
            current_row_width += img.width();
            max_row_width = qMax(max_row_width, current_row_width + (current_col * m_spacing));
            current_row_height = qMax(current_row_height, img.height());
            current_col++;
            if(current_col >= cols) {
                totalHeight += current_row_height;
                current_row_width = 0;
                current_row_height = 0;
                current_col = 0;
            }
        }
        if(current_col > 0) {
            totalHeight += current_row_height;
        }

        totalWidth = max_row_width;
        totalHeight += (rows > 1 ? (rows - 1) * m_spacing : 0);
    }
    
    totalWidth += 2 * m_border;
    totalHeight += 2 * m_border;

    return {totalWidth, totalHeight};
}
