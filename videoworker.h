// videoworker.h - Version 1.3 (Thêm cờ isSeeking)
#ifndef VIDEOWORKER_H
#define VIDEOWORKER_H

#include <QObject>
#include <QTimer>
#include <memory> 
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
    std::unique_ptr<VideoProcessor> m_processor;
    QTimer *m_playbackTimer;
    bool m_isPlaying = false;
    qint64 m_currentPts = 0;
    // THÊM MỚI: Cờ để ngăn xung đột khi đang tua video
    std::atomic<bool> m_isSeeking = false;
};

#endif // VIDEOWORKER_H
