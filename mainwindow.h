// mainwindow.h - Version 4.7 (Major UI Refactor)
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QImage>
#include <QFrame>
#include <QMouseEvent>
#include <memory>
#include "videoprocessor.h"
#include "viewpanel.h"
#include <QObject>
#include <QEvent>
#include <QTimer>
#include <QToolTip>
#include <QGroupBox>

// --- Helper class cho ô màu có thể click ---
class ClickableFrame : public QFrame {
    Q_OBJECT
public:
    explicit ClickableFrame(QWidget *parent = nullptr) : QFrame(parent) {
        setCursor(Qt::PointingHandCursor);
    }
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
        }
        QFrame::mousePressEvent(event);
    }
};

// --- Helper class cho tooltip của GroupBox ---
class TitleEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit TitleEventFilter(QObject *parent = nullptr) : QObject(parent) {}
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Enter) {
            QGroupBox *groupBox = qobject_cast<QGroupBox*>(obj);
            if (groupBox) {
                QTimer::singleShot(500, [=]() {
                    if (groupBox->underMouse()) {
                        QToolTip::showText(QCursor::pos(), groupBox->toolTip(), groupBox);
                    }
                });
            }
        } else if (event->type() == QEvent::Leave) {
            QToolTip::hideText();
        }
        return QObject::eventFilter(obj, event);
    }
};


// --- Forward declarations ---
class QSplitter;
class VideoWidget;
class QPushButton;
class QSlider;
class QLabel;
class QKeyEvent;
class LibraryWidget;
class QListWidgetItem;
class LibraryItemDelegate;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QAudioSink;
class QIODevice;
class QAction;
class QThread;
class VideoWorker;
class QButtonGroup;
class QRadioButton;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void requestOpenFile(const QString &filePath);
    void requestSeek(qint64 timestamp);
    void requestPlayPause(bool play);
    void requestNextFrame();
    void requestPrevFrame();
    void requestStop();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onFileOpened(bool success, VideoProcessor::AudioParams params, double frameRate, qint64 duration, AVRational timeBase);
    void onFrameReady(const FrameData &frameData);

    void onOpenFile();
    void onPlayPause();
    void onNextFrame();
    void onPrevFrame();
    void onTimelinePressed();
    void onTimelineReleased();
    void onTimelineMoved(int position);
    void onCapture();
    void onCaptureAndExport();
    void onMuteClicked();
    void onVolumeChanged(int volume);
    void onToggleRightPanel();

    void onLibrarySelectionChanged();
    void onViewAndCropSelected();
    void onDeleteSelected();
    void onLibraryItemQuickExport(QListWidgetItem *item);
    void onAddImagesToLibrary();
    void onImagesDroppedOnLibrary(const QList<QUrl> &urls);

    void onLibraryItemChanged(QListWidgetItem *item);
    void onViewPanelCrop();
    void onStyleChanged();
    void updateViewPanelScaleLabel(double scale);
    void updateViewPanelSizeLabel(const QSize &size);

    void onExport();
    void onExportImage(const QImage& image);
    void onChooseSavePath();
    void onOpenSaveFolder();

    void onChooseBackgroundColor();
    void onCornerRadiusSliderChanged(int value);
    void onBorderSliderChanged(int value);
    void onSizingModeChanged();
    void onGridModeChanged();

    void onFileDeletionFinished(bool success, const QString& filePath, QListWidgetItem* item);
    void onCroppedImageSaveFinished(bool success, const QString& filePath, const QImage& savedImage);


private:
    void setupUi();
    void setupLeftPanel(QWidget *parent);
    void setupRightPanel(QWidget *parent);
    void setupVideoWorker();
    void openVideoFile(const QString& filePath);
    void setupTempDirectory();
    void cleanupTempDirectory();
    void saveSettings();
    void loadSettings();
    QWidget* createVerticalSpinBox(QSpinBox* spinbox);
    void updateTimeLabel(int64_t currentTimeUs, int64_t totalTimeUs);
    void updateUIWithFrame(const FrameData& frameData);
    void cleanupAudio();
    QString generateUniqueFilename(const QString& baseName, const QString& extension);
    void ensureRightPanelVisible();
    QString formatTime(int64_t timeUs);
    void updatePlayerState(bool isVideoLoaded);
    void addImageToList(const QString& imagePath);

    // Layout
    QSplitter *mainSplitter;
    TitleEventFilter *m_titleFilter;

    std::unique_ptr<VideoWorker> m_videoWorker;
    std::unique_ptr<QThread> m_videoThread;

    // Left Panel Components
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
    
    // Audio
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_audioDevice = nullptr;
    float m_lastVolume = 1.0f;

    // Right Panel Components
    LibraryWidget *m_libraryWidget;
    LibraryItemDelegate *m_libraryDelegate;
    QPushButton *m_viewAndCropButton;
    QPushButton *m_deleteButton;
    QPushButton *m_addImagesButton;

    ViewPanel *m_viewPanel;
    QLineEdit *m_viewScaleLabel;
    QLineEdit *m_viewSizeLabel;
    
    // -- Style Panel --
    // Layout
    QRadioButton *m_radioGrid, *m_radioVertical, *m_radioHorizontal;
    QButtonGroup *m_gridModeGroup;
    QRadioButton *m_gridAutoRadio, *m_gridColumnRadio;
    QComboBox *m_gridColumnCountCombo;
    // Sizing
    QButtonGroup *m_sizingGroup;
    QRadioButton *m_sizeOriginalRadio, *m_sizeMatchFirstRadio, *m_sizeCustomRadio;
    QWidget *m_customSizeContainer;
    QSpinBox *m_customWidthSpinBox, *m_customHeightSpinBox;
    QLabel *m_customSizeLabelW, *m_customSizeLabelH;
    // Decoration
    QSpinBox *m_borderSpinBox;
    QSlider *m_borderSlider;
    QSpinBox *m_cornerRadiusSpinBox;
    QSlider *m_cornerRadiusSlider;
    QSpinBox *m_spacingSpinBox;
    ClickableFrame *m_colorSwatch;

    QComboBox *m_formatComboBox;
    QLineEdit *m_savePathEdit;
    QPushButton *m_exportButton;

    // Data
    bool m_isPlaying = false;
    bool m_isScrubbing = false;
    int64_t m_currentPts = 0;
    QList<QString> m_capturedFramePaths;
    QString m_currentVideoPath;
    QString m_tempPath;
    QString m_lastUsedDir;

    // Video Info
    double m_frameRate = 0.0;
    qint64 m_duration = 0;
    AVRational m_timeBase = {0, 1};
};
#endif // MAINWINDOW_H
