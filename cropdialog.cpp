// cropdialog.cpp - Version 5.2 (Sửa lỗi vùng chọn mặc định và áp dụng tỉ lệ ban đầu)
// Change-log:
// - Version 5.2:
//   - Xóa bỏ việc tự động tạo vùng chọn khi mở dialog.
//   - Đảm bảo tỉ lệ được chọn ban đầu (16:9) được áp dụng ngay lập tức.
// - Version 5.1: Tái cấu trúc hoàn toàn hàm CropArea::resizeSelection để đảm bảo
//   việc thay đổi kích thước vùng chọn luôn tuân thủ tỉ lệ đã chọn một cách chính xác.
#include "cropdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QApplication>
#include <QScrollArea>
#include <QButtonGroup>
#include <QShowEvent>
#include <QMessageBox>
#include <QKeyEvent>
#include <QToolBar>
#include <QAction>
#include <QUndoCommand>
#include <QIcon>
#include <QStyle>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QTimer>
#include <QGridLayout>
#include <QStackedWidget>

// --- Triển khai các phương thức của CropArea ---

CropArea::CropArea(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    m_selectionRect = QRectF();
    
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &CropArea::animateSelectionBorder);
    m_animationTimer->start(40);
}

void CropArea::setImage(const QImage &image)
{
    m_image = image;
    setFixedSize(m_image.size());
    emit selectionSizeChanged(m_image.size());
    update();
}

QRect CropArea::getSelection() const
{
    return m_selectionRect.isValid() ? m_selectionRect.normalized().toRect() : QRect();
}

void CropArea::clearSelection()
{
    m_selectionRect = QRectF();
    emitSelectionSize();
    update();
}

void CropArea::setAspectRatio(double ratio)
{
    m_aspectRatio = ratio;
    if (m_aspectRatio > 0 && m_selectionRect.isValid()) {
        // Khi đổi tỉ lệ, điều chỉnh vùng chọn hiện tại
        QRectF rect = m_selectionRect.normalized();
        double newHeight = rect.width() / m_aspectRatio;
        rect.setHeight(newHeight);
        // Cần đảm bảo nó không vượt ra ngoài ảnh
        if (rect.bottom() > m_image.rect().bottom()) {
            rect.moveBottom(m_image.rect().bottom());
            rect.setWidth(rect.height() * m_aspectRatio);
        }
        if (rect.right() > m_image.rect().right()) {
            rect.moveRight(m_image.rect().right());
            rect.setHeight(rect.width() / m_aspectRatio);
        }
        m_selectionRect = rect;
    }
    emitSelectionSize();
    update();
}

void CropArea::setScale(double newScale)
{
    m_scale = qBound(0.01, newScale, 5.0);
    if(!m_image.isNull()) {
        setFixedSize(m_image.size() * m_scale);
    }
    emit scaleChanged(m_scale);
    update();
}

void CropArea::moveSelection(int dx, int dy)
{
    if (m_selectionRect.isValid()) {
        m_selectionRect.translate(dx, dy);

        QRectF imageBounds = m_image.rect();
        if (m_selectionRect.left() < imageBounds.left()) m_selectionRect.moveLeft(imageBounds.left());
        if (m_selectionRect.right() > imageBounds.right()) m_selectionRect.moveRight(imageBounds.right());
        if (m_selectionRect.top() < imageBounds.top()) m_selectionRect.moveTop(imageBounds.top());
        if (m_selectionRect.bottom() > imageBounds.bottom()) m_selectionRect.moveBottom(imageBounds.bottom());
        
        emitSelectionSize();
        update();
    }
}

void CropArea::setSelection(const QRect& rect)
{
    m_selectionRect = QRectF(rect);
    emitSelectionSize();
    update();
}

void CropArea::emitSelectionSize()
{
    if (m_selectionRect.isValid() && !m_selectionRect.isEmpty()) {
        emit selectionSizeChanged(m_selectionRect.normalized().size().toSize());
    } else {
        emit selectionSizeChanged(m_image.size());
    }
}


void CropArea::animateSelectionBorder()
{
    m_dashOffset -= 0.5;
    if (m_dashOffset < -8.0) {
        m_dashOffset = 0;
    }
    if (m_selectionRect.isValid()) {
        update();
    }
}

void CropArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if(m_image.isNull()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.scale(m_scale, m_scale);
    painter.drawImage(0, 0, m_image);

    if (m_selectionRect.isValid()) {
        QPainterPath path;
        path.addRect(m_image.rect());
        path.addRect(m_selectionRect);
        painter.setBrush(QColor(0, 0, 0, 128));
        painter.drawPath(path);

        QPen pen(Qt::white, 1 / m_scale, Qt::CustomDashLine);
        pen.setDashPattern({4.0, 4.0});
        pen.setDashOffset(m_dashOffset);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(m_selectionRect);

        painter.setPen(QPen(Qt::white, 1 / m_scale));
        painter.setBrush(Qt::white);
        for (int i = TopLeft; i <= Left; ++i) {
            painter.drawRect(getHandleRect(static_cast<Handle>(i)));
        }
    }
}

void CropArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->pos() / m_scale;
        m_currentHandle = getHandleAt(pos);
        m_dragStartPos = pos;
        if (m_currentHandle == None) {
            m_selectionRect.setTopLeft(pos);
            m_selectionRect.setSize({0,0});
            m_currentHandle = BottomRight;
        }
    }
}

void CropArea::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = event->pos() / m_scale;
    if (m_currentHandle != None) {
        resizeSelection(pos);
    } else {
        updateCursor(pos);
    }
}

void CropArea::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_currentHandle = None;
}

void CropArea::wheelEvent(QWheelEvent *event)
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

void CropArea::updateCursor(const QPointF &pos)
{
    Handle handle = getHandleAt(pos);
    switch (handle) {
    case TopLeft: case BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case TopRight: case BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
    case Top: case Bottom: setCursor(Qt::SizeVerCursor); break;
    case Left: case Right: setCursor(Qt::SizeHorCursor); break;
    case Move: setCursor(Qt::SizeAllCursor); break;
    default: setCursor(Qt::CrossCursor); break;
    }
}

void CropArea::resizeSelection(const QPointF &pos)
{
    if (m_aspectRatio > 0 && m_currentHandle != Move) {
        QPointF anchor; 
        if (m_currentHandle == TopLeft)         anchor = m_selectionRect.bottomRight();
        else if (m_currentHandle == TopRight)   anchor = m_selectionRect.bottomLeft();
        else if (m_currentHandle == BottomLeft) anchor = m_selectionRect.topRight();
        else if (m_currentHandle == BottomRight)anchor = m_selectionRect.topLeft();

        if (!anchor.isNull()) { 
            QPointF adjustedPos = pos;
            double dx = pos.x() - anchor.x();
            double dy = pos.y() - anchor.y();

            if (qAbs(dx) / m_aspectRatio > qAbs(dy)) {
                adjustedPos.setY(anchor.y() + (dx / m_aspectRatio) * (dy >= 0 ? 1.0 : -1.0));
            } else {
                adjustedPos.setX(anchor.x() + (dy * m_aspectRatio) * (dx >= 0 ? 1.0 : -1.0));
            }
            m_selectionRect = QRectF(anchor, adjustedPos);

        } else { 
            QRectF currentRect = m_selectionRect;
            if (m_currentHandle == Top)         currentRect.setTop(pos.y());
            else if (m_currentHandle == Bottom) currentRect.setBottom(pos.y());
            else if (m_currentHandle == Left)   currentRect.setLeft(pos.x());
            else if (m_currentHandle == Right)  currentRect.setRight(pos.x());

            if (m_currentHandle == Top || m_currentHandle == Bottom) {
                double newWidth = qAbs(currentRect.height()) * m_aspectRatio;
                currentRect.setWidth(newWidth);
                currentRect.moveLeft(m_selectionRect.center().x() - newWidth / 2.0);
            } else { 
                double newHeight = qAbs(currentRect.width()) / m_aspectRatio;
                currentRect.setHeight(newHeight);
                currentRect.moveTop(m_selectionRect.center().y() - newHeight / 2.0);
            }
            m_selectionRect = currentRect;
        }
    } else {
        QPointF delta = pos - m_dragStartPos;
        QRectF originalRect = m_selectionRect;
        switch (m_currentHandle) {
            case TopLeft:     m_selectionRect.setTopLeft(originalRect.topLeft() + delta); break;
            case Top:         m_selectionRect.setTop(originalRect.top() + delta.y()); break;
            case TopRight:    m_selectionRect.setTopRight(originalRect.topRight() + delta); break;
            case Right:       m_selectionRect.setRight(originalRect.right() + delta.x()); break;
            case BottomRight: m_selectionRect.setBottomRight(originalRect.bottomRight() + delta); break;
            case Bottom:      m_selectionRect.setBottom(originalRect.bottom() + delta.y()); break;
            case BottomLeft:  m_selectionRect.setBottomLeft(originalRect.bottomLeft() + delta); break;
            case Left:        m_selectionRect.setLeft(originalRect.left() + delta.x()); break;
            case Move:        m_selectionRect.translate(delta); break;
            default: break;
        }
        m_dragStartPos = pos;
    }

    emitSelectionSize();
    update();
}


CropArea::Handle CropArea::getHandleAt(const QPointF &pos) const
{
    if (!m_selectionRect.isValid()) return None;
    for (int i = TopLeft; i <= Left; ++i) {
        if (getHandleRect(static_cast<Handle>(i)).contains(pos)) {
            return static_cast<Handle>(i);
        }
    }
    if (m_selectionRect.contains(pos)) {
        return Move;
    }
    return None;
}

QRectF CropArea::getHandleRect(Handle handle) const
{
    double size = 8 / m_scale;
    QPointF center;
    switch (handle) {
        case TopLeft: center = m_selectionRect.topLeft(); break;
        case Top: center = {m_selectionRect.center().x(), m_selectionRect.top()}; break;
        case TopRight: center = m_selectionRect.topRight(); break;
        case Right: center = {m_selectionRect.right(), m_selectionRect.center().y()}; break;
        case BottomRight: center = m_selectionRect.bottomRight(); break;
        case Bottom: center = {m_selectionRect.center().x(), m_selectionRect.bottom()}; break;
        case BottomLeft: center = m_selectionRect.bottomLeft(); break;
        case Left: center = {m_selectionRect.left(), m_selectionRect.center().y()}; break;
        default: return QRectF();
    }
    return QRectF(center.x() - size/2, center.y() - size/2, size, size);
}


// --- Lớp Command cho Undo/Redo thao tác cắt ảnh ---
class ApplyCropCommand : public QUndoCommand
{
public:
    ApplyCropCommand(QImage *imageContainer, CropArea *cropArea, const QImage &oldImage, const QImage &newImage, QUndoCommand *parent = nullptr)
        : QUndoCommand(parent), m_imageContainer(imageContainer), m_cropArea(cropArea), m_oldImage(oldImage), m_newImage(newImage)
    {
        setText("Cắt ảnh");
    }

    void undo() override {
        *m_imageContainer = m_oldImage;
        m_cropArea->setImage(m_oldImage);
        m_cropArea->clearSelection();
    }

    void redo() override {
        *m_imageContainer = m_newImage;
        m_cropArea->setImage(m_newImage);
        m_cropArea->clearSelection();
    }

private:
    QImage *m_imageContainer;
    CropArea *m_cropArea;
    QImage m_oldImage;
    QImage m_newImage;
};

// --- Triển khai CropDialog ---
CropDialog::CropDialog(const QImage &image, QWidget *parent)
    : QDialog(parent), m_currentImage(image)
{
    setupUi();
    m_cropArea->setImage(m_currentImage);
    setWindowTitle("Xem & Cắt ảnh"); 
    resize(800, 600);
    this->installEventFilter(this);
}

QImage CropDialog::getFinalImage() const
{
    return m_currentImage;
}

bool CropDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        keyPressEvent(keyEvent);
        return true; 
    }
    return QDialog::eventFilter(watched, event);
}


void CropDialog::keyPressEvent(QKeyEvent *event)
{
    int step = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;

    switch (event->key()) {
    case Qt::Key_Up:
        m_cropArea->moveSelection(0, -step);
        break;
    case Qt::Key_Down:
        m_cropArea->moveSelection(0, step);
        break;
    case Qt::Key_Left:
        m_cropArea->moveSelection(-step, 0);
        break;
    case Qt::Key_Right:
        m_cropArea->moveSelection(step, 0);
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        applyCrop();
        break;
    default:
        event->ignore();
        break;
    }
}


void CropDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    m_cropArea->setFocus();
    fitToWindow();
    
    // SỬA LỖI: Đồng bộ hóa trạng thái tỉ lệ khi dialog hiển thị
    // thay vì tự động tạo vùng chọn.
    onAspectRatioChanged(m_ratioGroup->checkedId(), true);
    
    // SỬA LỖI: Đã xóa dòng QTimer::singleShot để không tạo vùng chọn mặc định.
    // QTimer::singleShot(0, this, &CropDialog::createDefaultSelection);
}

void CropDialog::createDefaultSelection()
{
    // Chức năng này không còn được sử dụng nhưng vẫn giữ lại mã nguồn để tham khảo.
    if (m_currentImage.isNull()) return;

    if (m_cropArea->getSelection().isValid() && !m_cropArea->getSelection().isEmpty()) {
        return;
    }

    double aspectRatio = 16.0/9.0;
    int imgWidth = m_currentImage.width();
    int imgHeight = m_currentImage.height();

    int selWidth, selHeight;
    if (imgWidth / (double)imgHeight > aspectRatio) {
        selHeight = imgHeight;
        selWidth = static_cast<int>(selHeight * aspectRatio);
    } else {
        selWidth = imgWidth;
        selHeight = static_cast<int>(selWidth / aspectRatio);
    }

    int x = (imgWidth - selWidth) / 2;
    int y = (imgHeight - selHeight) / 2;

    m_cropArea->setSelection(QRect(x, y, selWidth, selHeight));
}


void CropDialog::setupUi()
{
    m_undoStack = new QUndoStack(this);
    m_undoAction = m_undoStack->createUndoAction(this, "Hoàn tác cắt");
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_redoAction = m_undoStack->createRedoAction(this, "Làm lại cắt");
    m_redoAction->setShortcut(QKeySequence::Redo);
    this->addActions({m_undoAction, m_redoAction});

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QToolBar *toolBar = new QToolBar(this);
    toolBar->addAction(m_undoAction);
    toolBar->addAction(m_redoAction);
    mainLayout->addWidget(toolBar);

    m_cropArea = new CropArea(this);
    connect(m_cropArea, &CropArea::scaleChanged, this, &CropDialog::updateScaleLabel);
    connect(m_cropArea, &CropArea::selectionSizeChanged, this, &CropDialog::updateSizeLabel);
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidget(m_cropArea);
    mainLayout->addWidget(m_scrollArea, 1);

    QGroupBox *ratioBox = new QGroupBox("Tỉ lệ");
    ratioBox->setToolTip("Chọn tỉ lệ khung hình cho vùng cắt");
    QVBoxLayout *ratioVLayout = new QVBoxLayout(ratioBox);
    QHBoxLayout *ratioHLayout = new QHBoxLayout();
    m_ratioGroup = new QButtonGroup(this);
    m_ratioGroup->setExclusive(true);
    QRadioButton *freeformRadio = new QRadioButton("Tự do");
    QRadioButton *ratio11 = new QRadioButton("1:1");
    QRadioButton *ratio43 = new QRadioButton("4:3");
    QRadioButton *ratio169 = new QRadioButton("16:9");
    m_customRadio = new QRadioButton("Tuỳ chỉnh");

    m_ratioGroup->addButton(freeformRadio, 0);
    m_ratioGroup->addButton(ratio11, 1);
    m_ratioGroup->addButton(ratio43, 2);
    m_ratioGroup->addButton(ratio169, 3);
    m_ratioGroup->addButton(m_customRadio, 4);

    ratioHLayout->addWidget(freeformRadio);
    ratioHLayout->addWidget(ratio11);
    ratioHLayout->addWidget(ratio43);
    ratioHLayout->addWidget(ratio169);
    ratioHLayout->addWidget(m_customRadio);
    ratioVLayout->addLayout(ratioHLayout);

    m_customContainer = new QWidget();
    QVBoxLayout *customContainerLayout = new QVBoxLayout(m_customContainer);
    customContainerLayout->setContentsMargins(10, 5, 0, 0);

    m_customStackedWidget = new QStackedWidget();
    m_ratioSubRadio = new QRadioButton("Theo tỉ lệ");
    m_sizeSubRadio = new QRadioButton("Theo kích thước");
    
    QWidget* ratioWidgetPage = new QWidget();
    QHBoxLayout *customRatioLayout = new QHBoxLayout(ratioWidgetPage);
    customRatioLayout->setContentsMargins(0, 0, 0, 0);
    m_ratioWEdit = new QLineEdit("16");
    m_ratioWEdit->setValidator(new QDoubleValidator(0.1, 1000, 2, this));
    m_ratioWEdit->setFixedWidth(50);
    m_ratioHEdit = new QLineEdit("9");
    m_ratioHEdit->setValidator(new QDoubleValidator(0.1, 1000, 2, this));
    m_ratioHEdit->setFixedWidth(50);
    customRatioLayout->addWidget(m_ratioWEdit);
    customRatioLayout->addWidget(new QLabel(":"));
    customRatioLayout->addWidget(m_ratioHEdit);
    customRatioLayout->addStretch();
    m_customStackedWidget->addWidget(ratioWidgetPage);
    
    QWidget* sizeWidgetPage = new QWidget();
    QHBoxLayout *customSizeLayout = new QHBoxLayout(sizeWidgetPage);
    customSizeLayout->setContentsMargins(0, 0, 0, 0);
    m_customWidthSpinBox = new QSpinBox();
    m_customWidthSpinBox->setRange(1, 9999); m_customWidthSpinBox->setValue(1920);
    m_customWidthSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_customWidthSpinBox->setFixedWidth(50);
    m_customHeightSpinBox = new QSpinBox();
    m_customHeightSpinBox->setRange(1, 9999); m_customHeightSpinBox->setValue(1080);
    m_customHeightSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_customHeightSpinBox->setFixedWidth(50);
    customSizeLayout->addWidget(m_customWidthSpinBox);
    customSizeLayout->addWidget(new QLabel("x"));
    customSizeLayout->addWidget(m_customHeightSpinBox);
    customSizeLayout->addStretch();
    m_customStackedWidget->addWidget(sizeWidgetPage);

    QGridLayout *customGridLayout = new QGridLayout();
    customGridLayout->setColumnStretch(1, 1);
    customGridLayout->addWidget(m_ratioSubRadio, 0, 0);
    customGridLayout->addWidget(m_sizeSubRadio, 1, 0);
    customGridLayout->addWidget(m_customStackedWidget, 0, 1, 2, 1);
    customContainerLayout->addLayout(customGridLayout);
    
    m_ratioSubRadio->setChecked(true);
    
    ratioVLayout->addWidget(m_customContainer);
    m_customContainer->setVisible(false);
    
    ratio169->setChecked(true);

    connect(m_ratioSubRadio, &QRadioButton::toggled, this, &CropDialog::updateCustomValues);
    connect(m_sizeSubRadio, &QRadioButton::toggled, this, &CropDialog::updateCustomValues);

    QGroupBox *zoomBox = new QGroupBox("Thu phóng");
    zoomBox->setToolTip("Các công cụ để điều chỉnh góc nhìn");
    QHBoxLayout *zoomLayout = new QHBoxLayout(zoomBox);
    QPushButton *fitButton = new QPushButton("Vừa khung");
    fitButton->setToolTip("Thu phóng ảnh vừa với khung xem (95%)");
    QPushButton *oneToOneButton = new QPushButton("1:1");
    oneToOneButton->setToolTip("Xem ảnh với kích thước thật (100%)");
    m_scaleLabel = new QLineEdit("100%");
    m_scaleLabel->setReadOnly(true);
    m_scaleLabel->setAlignment(Qt::AlignCenter);
    m_scaleLabel->setFixedSize(50, 22);
    m_scaleLabel->setToolTip("Tỉ lệ phóng hiện tại");
    m_scaleLabel->setStyleSheet("background-color: #2c3e50; color: white; border: 1px solid #606060; selection-background-color: transparent;");
    
    m_sizeLabel = new QLineEdit("1920x1080");
    m_sizeLabel->setReadOnly(true);
    m_sizeLabel->setAlignment(Qt::AlignCenter);
    m_sizeLabel->setFixedWidth(80);
    m_sizeLabel->setToolTip("Kích thước vùng chọn (pixel)");
    m_sizeLabel->setStyleSheet("background-color: #2c3e50; color: white; border: 1px solid #606060; selection-background-color: transparent;");

    zoomLayout->addWidget(fitButton);
    zoomLayout->addWidget(oneToOneButton);
    zoomLayout->addWidget(m_scaleLabel);
    zoomLayout->addWidget(m_sizeLabel);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(ratioBox);
    controlsLayout->addStretch();
    controlsLayout->addWidget(zoomBox);
    mainLayout->addLayout(controlsLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(); 

    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Huỷ");
    QPushButton *exportButton = new QPushButton("Xuất ảnh");
    exportButton->setToolTip("Lưu ảnh hiện tại ra file và đóng cửa sổ");
    exportButton->setStyleSheet("background-color: #e67e22; color: white; border: none; padding: 5px; border-radius: 3px;");

    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(exportButton);
    
    connect(m_ratioGroup, &QButtonGroup::idToggled, this, &CropDialog::onAspectRatioChanged);
    connect(m_customWidthSpinBox, &QSpinBox::valueChanged, this, &CropDialog::updateCustomValues);
    connect(m_customHeightSpinBox, &QSpinBox::valueChanged, this, &CropDialog::updateCustomValues);
    connect(m_ratioWEdit, &QLineEdit::textChanged, this, &CropDialog::updateCustomValues);
    connect(m_ratioHEdit, &QLineEdit::textChanged, this, &CropDialog::updateCustomValues);
    connect(fitButton, &QPushButton::clicked, this, &CropDialog::fitToWindow);
    connect(oneToOneButton, &QPushButton::clicked, this, &CropDialog::oneToOne);
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(exportButton, &QPushButton::clicked, this, &CropDialog::exportImage);
    
    mainLayout->addLayout(buttonLayout);
}

void CropDialog::onAspectRatioChanged(int id, bool checked)
{
    if (!checked) return;
    
    m_customContainer->setVisible(id == 4);

    switch(id) {
        case 0: m_cropArea->setAspectRatio(0.0); break;
        case 1: m_cropArea->setAspectRatio(1.0); break;
        case 2: m_cropArea->setAspectRatio(4.0/3.0); break;
        case 3: m_cropArea->setAspectRatio(16.0/9.0); break;
        case 4: updateCustomValues(); break;
    }
}

void CropDialog::updateCustomValues()
{
    if (m_customRadio->isChecked()) {
        m_customStackedWidget->setCurrentIndex(m_sizeSubRadio->isChecked() ? 1 : 0);

        if (m_sizeSubRadio->isChecked()) {
            double w = m_customWidthSpinBox->value();
            double h = m_customHeightSpinBox->value();
            if (h > 0) {
                m_cropArea->setAspectRatio(w / h);
            }
        } else if (m_ratioSubRadio->isChecked()) {
            double w = m_ratioWEdit->text().toDouble();
            double h = m_ratioHEdit->text().toDouble();
            if (h > 0) {
                m_cropArea->setAspectRatio(w / h);
            }
        }
    }
}

void CropDialog::updateScaleLabel(double scale)
{
    m_scaleLabel->setText(QString::number(qRound(scale * 100)) + "%");
}

void CropDialog::updateSizeLabel(const QSize &size)
{
    m_sizeLabel->setText(QString("%1x%2").arg(size.width()).arg(size.height()));
}


void CropDialog::fitToWindow()
{
    if(m_currentImage.isNull()) return;
    double w_ratio = (double)(m_scrollArea->width() * 0.95) / (m_currentImage.width());
    double h_ratio = (double)(m_scrollArea->height() * 0.95) / (m_currentImage.height());
    double scale = qMin(w_ratio, h_ratio);
    m_cropArea->setScale(scale);
}

void CropDialog::oneToOne()
{
    m_cropArea->setScale(1.0);
}

void CropDialog::applyCrop()
{
    QRect selection = m_cropArea->getSelection();
    if (!selection.isValid() || selection.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng vẽ một vùng chọn trước khi nhấn Enter.");
        return;
    }
    QImage oldImage = m_currentImage;
    QImage newImage = oldImage.copy(selection);
    m_undoStack->push(new ApplyCropCommand(&m_currentImage, m_cropArea, oldImage, newImage));

    fitToWindow();
}

void CropDialog::exportImage()
{
    emit exportImageRequested(m_currentImage);
    accept();
}
