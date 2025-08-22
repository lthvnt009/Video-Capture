// mainwindow.h - Version 6.2 (Sửa lỗi connect)
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "videoprocessor.h" 
#include "helpers.h" 

// --- Forward declarations ---
class QSplitter;
class QKeyEvent;
class QAudioSink;
class QIODevice;
class QThread;
class VideoWorker;
class SidePanel; 
class PlayerPanel; 
class QListWidgetItem; 

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

    void playerStateChanged(bool isVideoLoaded);
    void newFrameReady(const FrameData& frameData, qint64 duration, double frameRate, const AVRational& timeBase);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public slots:
    void addImageToList(const QString &imagePath);

private slots:
    void onFileOpened(bool success, VideoProcessor::AudioParams params, double frameRate, qint64 duration, AVRational timeBase);
    void onFrameReady(const FrameData &frameData);

    void onOpenFile();
    void onPlayPause();
    void onCapture();
    void onCaptureAndExport();
    void onMuteClicked();
    void onVolumeChanged(int volume);
    void onToggleRightPanel();
    void onTimelineMoved(int position);
    // THÊM MỚI: Thêm lại slot đã thiếu
    void onTimelineReleased();

    void onExportImage(const QImage& image);
    void onAddImagesToLibrary();
    void onImagesDroppedOnLibrary(const QList<QUrl> &urls);

private:
    void setupUi();
    void setupVideoWorker();
    void openVideoFile(const QString& filePath);
    void setupTempDirectory();
    void cleanupTempDirectory();
    void saveSettings();
    void loadSettings();
    
    void cleanupAudio();
    QString generateUniqueFilename(const QString& baseName, const QString& extension);
    void ensureRightPanelVisible();
    
    // Layout & Modules
    QSplitter *mainSplitter;
    PlayerPanel *m_playerPanel;
    SidePanel *m_sidePanel; 

    // Worker Thread
    std::unique_ptr<VideoWorker> m_videoWorker;
    std::unique_ptr<QThread> m_videoThread;
    
    // Audio
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_audioDevice = nullptr;
    float m_lastVolume = 1.0f;

    // Data & State
    bool m_isPlaying = false;
    bool m_isScrubbing = false;
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
