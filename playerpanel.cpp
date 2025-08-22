// playerpanel.cpp - Version 1.1 (Thêm Time Label Update)
#include "playerpanel.h"
#include "videowidget.h"
#include "helpers.h" // Đổi sang dùng helpers.h

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QAction>
#include <QStyle>
#include <QKeyEvent>
#include <QToolTip>
#include <QMouseEvent>

PlayerPanel::PlayerPanel(QWidget *parent) : QWidget(parent)
{
    m_titleFilter = new TitleEventFilter(this);
    setupUi();
}

VideoWidget* PlayerPanel::getVideoWidget() const
{
    return m_videoWidget;
}

void PlayerPanel::setupUi()
{
    QGroupBox *playerBox = new QGroupBox("Trình Phát", this);
    playerBox->setToolTip("Khu vực hiển thị và điều khiển video.\nKéo thả file video vào đây để mở.");
    playerBox->installEventFilter(m_titleFilter);
    QVBoxLayout *leftLayout = new QVBoxLayout(playerBox);
    leftLayout->setContentsMargins(5,5,5,5);
    leftLayout->setSpacing(5);

    m_videoWidget = new VideoWidget();
    leftLayout->addWidget(m_videoWidget, 1);

    QHBoxLayout *timelineLayout = new QHBoxLayout();
    m_timelineSlider = new QSlider(Qt::Horizontal);
    m_timelineSlider->setRange(0, 1000);
    m_timelineSlider->setFocusPolicy(Qt::NoFocus);
    m_timelineSlider->setMouseTracking(true);
    m_timelineSlider->installEventFilter(this);

    m_timeLabel = new QLabel("00:00.000 / 00:00.000");
    timelineLayout->addWidget(m_timelineSlider);
    timelineLayout->addWidget(m_timeLabel);
    leftLayout->addLayout(timelineLayout);

    QHBoxLayout *controlLayout = new QHBoxLayout();
    m_prevFrameButton = new QPushButton();
    m_prevFrameButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    m_prevFrameButton->setToolTip("Frame trước (Phím ←)");
    m_playPauseButton = new QPushButton();
    m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playPauseButton->setToolTip("Phát/Dừng (Phím Space)");
    m_nextFrameButton = new QPushButton();
    m_nextFrameButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    m_nextFrameButton->setToolTip("Frame kế tiếp (Phím →)");
    
    QSize buttonIconSize(36, 36);
    m_prevFrameButton->setIconSize(buttonIconSize);
    m_playPauseButton->setIconSize(buttonIconSize);
    m_nextFrameButton->setIconSize(buttonIconSize);

    m_muteButton = new QPushButton();
    m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    m_muteButton->setToolTip("Tắt/Mở tiếng");
    m_muteButton->setCheckable(true);
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setMaximumWidth(80);

    m_captureAndExportButton = new QPushButton("Chụp và Xuất");
    m_captureAndExportButton->setToolTip("Chụp frame hiện tại và xuất trực tiếp ra file (Ctrl+Space)");
    m_captureAndExportButton->setStyleSheet("background-color: #27ae60; color: white; border: none; padding: 5px; border-radius: 3px;");
    
    m_captureExportAction = new QAction(this);
    m_captureExportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space));
    connect(m_captureExportAction, &QAction::triggered, this, &PlayerPanel::captureAndExportClicked);
    this->addAction(m_captureExportAction);

    m_captureButton = new QPushButton("Chụp");
    m_captureButton->setToolTip("Chụp frame hiện tại vào thư viện");
    m_captureButton->setStyleSheet("background-color: #3498db; color: white; border: none; padding: 5px; border-radius: 3px;");

    m_openButton = new QPushButton("Mở Video");
    m_openButton->setToolTip("Mở một file video mới");
    m_openButton->setStyleSheet("background-color: #9b59b6; color: white; border: none; padding: 5px; border-radius: 3px;");

    m_toggleRightPanelButton = new QPushButton();
    m_toggleRightPanelButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    m_toggleRightPanelButton->setToolTip("Mở rộng / Thu gọn panel bên phải");
    m_toggleRightPanelButton->setCheckable(true);
    m_toggleRightPanelButton->setChecked(false);

    controlLayout->addWidget(m_prevFrameButton);
    controlLayout->addWidget(m_playPauseButton);
    controlLayout->addWidget(m_nextFrameButton);
    controlLayout->addStretch();
    controlLayout->addWidget(m_muteButton);
    controlLayout->addWidget(m_volumeSlider);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(m_captureAndExportButton);
    controlLayout->addWidget(m_captureButton);
    controlLayout->addWidget(m_openButton);
    controlLayout->addWidget(m_toggleRightPanelButton);
    leftLayout->addLayout(controlLayout);
    
    QVBoxLayout* mainContainerLayout = new QVBoxLayout(this);
    mainContainerLayout->addWidget(playerBox);

    connect(m_openButton, &QPushButton::clicked, this, &PlayerPanel::openFileClicked);
    connect(m_captureButton, &QPushButton::clicked, this, &PlayerPanel::captureClicked);
    connect(m_captureAndExportButton, &QPushButton::clicked, this, &PlayerPanel::captureAndExportClicked);
    connect(m_playPauseButton, &QPushButton::clicked, this, &PlayerPanel::playPauseClicked);
    connect(m_nextFrameButton, &QPushButton::clicked, this, &PlayerPanel::nextFrameClicked);
    connect(m_prevFrameButton, &QPushButton::clicked, this, &PlayerPanel::prevFrameClicked);
    connect(m_timelineSlider, &QSlider::sliderPressed, this, &PlayerPanel::timelinePressed);
    connect(m_timelineSlider, &QSlider::sliderReleased, this, &PlayerPanel::timelineReleased);
    connect(m_timelineSlider, &QSlider::sliderMoved, this, &PlayerPanel::timelineMoved);
    connect(m_muteButton, &QPushButton::clicked, this, &PlayerPanel::muteClicked);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &PlayerPanel::volumeChanged);
    connect(m_toggleRightPanelButton, &QPushButton::clicked, this, &PlayerPanel::toggleRightPanelClicked);

    // Kết nối nội bộ để cập nhật icon của nút Mute
    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int volume){
        bool isMutedNow = (volume == 0);
        m_muteButton->setChecked(isMutedNow);
        m_muteButton->setIcon(style()->standardIcon(isMutedNow ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
    });
}

void PlayerPanel::updatePlayerState(bool isVideoLoaded)
{
    m_playPauseButton->setEnabled(isVideoLoaded);
    m_nextFrameButton->setEnabled(isVideoLoaded);
    m_prevFrameButton->setEnabled(isVideoLoaded);
    m_timelineSlider->setEnabled(isVideoLoaded);
    m_captureButton->setEnabled(isVideoLoaded);
    m_captureAndExportButton->setEnabled(isVideoLoaded);
    m_captureExportAction->setEnabled(isVideoLoaded);
}

void PlayerPanel::updateUIWithFrame(const FrameData& frameData, qint64 duration, double frameRate, const AVRational& timeBase)
{
    if (!frameData.image.isNull()) {
        m_videoWidget->setImage(frameData.image);
        if (timeBase.den == 0) return;

        m_duration = duration; // Cập nhật duration
        int64_t currentTimeUs = frameData.pts * 1000000 * timeBase.num / timeBase.den;
        
        updateTimeLabelOnly(currentTimeUs, duration, frameRate);

        // Cập nhật Timeline Slider
        m_timelineSlider->blockSignals(true);
        if (duration > 0) {
            m_timelineSlider->setValue((double)currentTimeUs / duration * 1000);
        }
        m_timelineSlider->blockSignals(false);
    }
}

// THÊM MỚI: Hàm chỉ cập nhật label
void PlayerPanel::updateTimeLabelOnly(qint64 currentTimeUs, qint64 totalTimeUs, double frameRate)
{
    QString timeStr = formatTime(currentTimeUs);
    QString durationStr = formatTime(totalTimeUs);
    if (frameRate > 0) {
        long long currentFrame = (currentTimeUs / 1000000.0) * frameRate;
        long long totalFrames = (totalTimeUs / 1000000.0) * frameRate;
        m_timeLabel->setText(QString("%1 (Frame %2) / %3 (Frame %4)").arg(timeStr).arg(currentFrame).arg(durationStr).arg(totalFrames));
    } else {
        m_timeLabel->setText(QString("%1 / %2").arg(timeStr).arg(durationStr));
    }
}


void PlayerPanel::setPlayPauseButtonIcon(bool isPlaying)
{
    m_playPauseButton->setIcon(style()->standardIcon(isPlaying ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
}

bool PlayerPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_timelineSlider && event->type() == QEvent::MouseMove) {
        if (m_duration > 0) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            double pos = mouseEvent->pos().x() / (double)m_timelineSlider->width();
            int value = m_timelineSlider->minimum() + ((m_timelineSlider->maximum() - m_timelineSlider->minimum()) * pos);
            
            value = qBound(m_timelineSlider->minimum(), value, m_timelineSlider->maximum());

            qint64 hoverTimeUs = m_duration * (double)value / 1000.0;
            QString timeStr = formatTime(hoverTimeUs);
            QToolTip::showText(mouseEvent->globalPos(), timeStr, m_timelineSlider);
        }
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

QString PlayerPanel::formatTime(int64_t timeUs)
{
    int totalMilliseconds = timeUs / 1000;
    int seconds = (totalMilliseconds / 1000) % 60;
    int minutes = (totalMilliseconds / (1000 * 60)) % 60;
    int hours = (totalMilliseconds / (1000 * 60 * 60));
    int milliseconds = totalMilliseconds % 1000;
    if (hours > 0)
        return QString("%1:%2:%3.%4").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')).arg(milliseconds, 3, 10, QChar('0'));
    else
        return QString("%1:%2.%3").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')).arg(milliseconds, 3, 10, QChar('0'));
}
