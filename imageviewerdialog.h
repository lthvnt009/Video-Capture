// imageviewerdialog.h - Version 1.1 (Thêm QTimer)
#ifndef IMAGEVIEWERDIALOG_H
#define IMAGEVIEWERDIALOG_H

#include <QDialog>
#include <QImage>
#include <QTimer> // THÊM MỚI

class QLabel;
class QScrollArea;
class QWheelEvent;

class ImageViewerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewerDialog(const QImage &image, QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void fitToWindow();
    void oneToOne();

private:
    void setupUi();
    void updateScale(double newScale);

    QScrollArea *m_scrollArea;
    QLabel *m_imageLabel;
    QImage m_sourceImage;
    double m_scale = 1.0;
};

#endif // IMAGEVIEWERDIALOG_H
