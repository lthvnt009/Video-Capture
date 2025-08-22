// cropdialog.h - Version 3.5 (Image Size Display)
#ifndef CROPDIALOG_H
#define CROPDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QImage>
#include <QRectF>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainterPath>
#include <QUndoStack>

class QTimer;
class QLineEdit;
class QStackedWidget;

class CropArea : public QWidget
{
    Q_OBJECT

public:
    explicit CropArea(QWidget *parent = nullptr);

    void setImage(const QImage &image);
    QRect getSelection() const;
    void clearSelection();
    void setAspectRatio(double ratio);
    void setScale(double newScale);
    void moveSelection(int dx, int dy);

signals:
    void scaleChanged(double newScale);
    // THÊM MỚI: Signal gửi kích thước vùng chọn
    void selectionSizeChanged(const QSize &size);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void animateSelectionBorder();

private:
    enum Handle { None, TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, Move };

    void updateCursor(const QPointF &pos);
    void resizeSelection(const QPointF &pos);
    Handle getHandleAt(const QPointF &pos) const;
    QRectF getHandleRect(Handle handle) const;
    void emitSelectionSize(); // Helper

    QImage m_image;
    QRectF m_selectionRect;
    double m_aspectRatio = 0.0;
    double m_scale = 1.0;
    Handle m_currentHandle = None;
    QPointF m_dragStartPos;
    
    QTimer *m_animationTimer;
    double m_dashOffset = 0;
};

class QScrollArea;
class QButtonGroup;
class QPushButton;
class QAction;
class QSpinBox;
class QRadioButton;

class CropDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CropDialog(const QImage &image, QWidget *parent = nullptr);
    QImage getFinalImage() const;

signals:
    void exportImageRequested(const QImage &image);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override; // THÊM MỚI

private slots:
    void onAspectRatioChanged(int id, bool checked);
    void updateCustomValues();
    void fitToWindow();
    void oneToOne();
    void applyCrop();
    void exportImage();
    void updateScaleLabel(double scale);
    // THÊM MỚI: Slot cập nhật kích thước
    void updateSizeLabel(const QSize &size);

private:
    void setupUi();

    CropArea *m_cropArea;
    QScrollArea *m_scrollArea;
    QButtonGroup *m_ratioGroup;
    QImage m_currentImage;
    QLineEdit *m_scaleLabel;
    QLineEdit *m_sizeLabel; // THÊM MỚI

    QWidget *m_customContainer;
    QRadioButton *m_customRadio;
    QRadioButton *m_ratioSubRadio;
    QRadioButton *m_sizeSubRadio;
    QStackedWidget *m_customStackedWidget;
    QSpinBox *m_customWidthSpinBox;
    QSpinBox *m_customHeightSpinBox;
    QLineEdit *m_ratioWEdit;
    QLineEdit *m_ratioHEdit;

    QUndoStack *m_undoStack;
    QAction *m_undoAction;
    QAction *m_redoAction;
};

#endif // CROPDIALOG_H
