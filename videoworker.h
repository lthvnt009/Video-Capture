// videoworker.h - Version 1.2 (Modernized with std::unique_ptr)
#ifndef VIDEOWORKER_H
#define VIDEOWORKER_H

#include <QObject>
#include <QTimer>
#include <memory> // Thêm thư viện để sử dụng std::unique_ptr
#include "videoprocessor.h"

class VideoWorker : public QObject
{
    Q_OBJECT

public:
    explicit VideoWorker(QObject *parent = nullptr);
    ~VideoWorker();

public slots:
    void processOpenFile(const QString &filePath);
    void processSeek(qint64 timestamp);
    void processPlayPause(bool play);
    void processNextFrame();
    void processPrevFrame();
    void stop();

signals:
    void fileOpened(bool success, VideoProcessor::AudioParams params, double frameRate, qint64 duration, AVRational timeBase);
    void frameReady(const FrameData &frameData);
    void finished();

private slots:
    void onPlaybackTimerTimeout();

private:
    // THAY ĐỔI: Sử dụng con trỏ thông minh để quản lý bộ nhớ tự động
    std::unique_ptr<VideoProcessor> m_processor;
    QTimer *m_playbackTimer;
    bool m_isPlaying = false;
    qint64 m_currentPts = 0;
};

#endif // VIDEOWORKER_H
