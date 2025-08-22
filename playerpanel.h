// playerpanel.h - Version 1.1 (Thêm Time Label Update)
// Module chuyên trách cho panel trình phát video (panel bên trái)
#ifndef PLAYERPANEL_H
#define PLAYERPANEL_H

#include <QWidget>
#include "videoprocessor.h" // Cần cho FrameData

// --- Forward declarations ---
class VideoWidget;
class QPushButton;
class QSlider;
class QLabel;
class QAction;
class QGroupBox;
class QKeyEvent;
class TitleEventFilter;

class PlayerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPanel(QWidget *parent = nullptr);
    VideoWidget* getVideoWidget() const;

signals:
    // Tín hiệu phát ra khi người dùng tương tác với UI
    void openFileClicked();
    void playPauseClicked();
    void nextFrameClicked();
    void prevFrameClicked();
    void captureClicked();
    void captureAndExportClicked();
    void toggleRightPanelClicked();
    void timelinePressed();
    void timelineReleased();
    void timelineMoved(int position);
    void muteClicked();
    void volumeChanged(int volume);
    void seekRequested(qint64 timestamp); // Tín hiệu khi scrubbing trên timeline

public slots:
    // Khe cắm để MainWindow cập nhật trạng thái cho panel này
    void updatePlayerState(bool isVideoLoaded);
    void updateUIWithFrame(const FrameData& frameData, qint64 duration, double frameRate, const AVRational& timeBase);
    void setPlayPauseButtonIcon(bool isPlaying);
    // THÊM MỚI: Chỉ cập nhật label thời gian để tua video mượt hơn
    void updateTimeLabelOnly(qint64 currentTimeUs, qint64 totalTimeUs, double frameRate);
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QString formatTime(int64_t timeUs);
    void setupUi();

    // UI Components
    TitleEventFilter *m_titleFilter;
    VideoWidget *m_videoWidget;
    QPushButton *m_openButton;
    QPushButton *m_playPauseButton;
    QPushButton *m_nextFrameButton;
    QPushButton *m_prevFrameButton;
    QPushButton *m_captureButton;
    QPushButton *m_captureAndExportButton;
    QPushButton *m_toggleRightPanelButton;
    QAction *m_captureExportAction;
    QSlider *m_timelineSlider;
    QLabel *m_timeLabel;
    QPushButton *m_muteButton;
    QSlider *m_volumeSlider;

    // Data
    qint64 m_duration = 0;
};

#endif // PLAYERPANEL_H
