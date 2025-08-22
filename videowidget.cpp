// videowidget.cpp - Version 1.1
#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background-color: black;");
}

QImage VideoWidget::getCurrentImage() const
{
    return m_image;
}

void VideoWidget::setImage(const QImage &image)
{
    m_image = image;
    update();
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (m_image.isNull()) {
        return;
    }

    QPainter painter(this);
    QImage scaledImage = m_image.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    int x = (this->width() - scaledImage.width()) / 2;
    int y = (this->height() - scaledImage.height()) / 2;
    painter.drawImage(x, y, scaledImage);
}
