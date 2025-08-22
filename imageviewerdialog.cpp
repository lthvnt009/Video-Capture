// imageviewerdialog.cpp - Version 1.0
#include "imageviewerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QWheelEvent>
#include <QShowEvent>
#include <QPixmap>

ImageViewerDialog::ImageViewerDialog(const QImage &image, QWidget *parent)
    : QDialog(parent), m_sourceImage(image)
{
    setupUi();
    setWindowTitle("Xem ảnh chi tiết");
    resize(800, 600);
    m_imageLabel->setPixmap(QPixmap::fromImage(m_sourceImage));
}

void ImageViewerDialog::wheelEvent(QWheelEvent *event)
{
    double newScale = m_scale + (event->angleDelta().y() > 0 ? 0.1 : -0.1);
    updateScale(qBound(0.1, newScale, 5.0));
}

void ImageViewerDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    fitToWindow();
}

void ImageViewerDialog::fitToWindow()
{
    double w_ratio = (double)m_scrollArea->width() / (m_sourceImage.width() + 10);
    double h_ratio = (double)m_scrollArea->height() / (m_sourceImage.height() + 10);
    updateScale(qMin(w_ratio, h_ratio));
}

void ImageViewerDialog::oneToOne()
{
    updateScale(1.0);
}

void ImageViewerDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_scrollArea = new QScrollArea(this);
    m_imageLabel = new QLabel(this);
    m_imageLabel->setBackgroundRole(QPalette::Dark);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidget(m_imageLabel);
    m_scrollArea->setBackgroundRole(QPalette::Dark);
    mainLayout->addWidget(m_scrollArea, 1);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    QPushButton *fitButton = new QPushButton("Phóng");
    QPushButton *oneToOneButton = new QPushButton("1:1");
    QPushButton *closeButton = new QPushButton("Đóng");

    controlsLayout->addStretch();
    controlsLayout->addWidget(fitButton);
    controlsLayout->addWidget(oneToOneButton);
    controlsLayout->addWidget(closeButton);
    mainLayout->addLayout(controlsLayout);

    connect(fitButton, &QPushButton::clicked, this, &ImageViewerDialog::fitToWindow);
    connect(oneToOneButton, &QPushButton::clicked, this, &ImageViewerDialog::oneToOne);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void ImageViewerDialog::updateScale(double newScale)
{
    m_scale = newScale;
    m_imageLabel->setFixedSize(m_sourceImage.size() * m_scale);
}
