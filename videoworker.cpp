// videoworker.cpp - Version 1.2 (Modernized with std::unique_ptr)
#include "videoworker.h"
#include <QDebug>
#include <QThread>

// THAY ĐỔI: Không cần include <memory> ở đây vì đã có trong file .h

VideoWorker::VideoWorker(QObject *parent) : QObject(parent)
{
    // THAY ĐỔI: Khởi tạo m_processor bằng std::make_unique
    m_processor = std::make_unique<VideoProcessor>();
    m_playbackTimer = new QTimer(this);
    connect(m_playbackTimer, &QTimer::timeout, this, &VideoWorker::onPlaybackTimerTimeout);
}

VideoWorker::~VideoWorker()
{
    m_playbackTimer->stop();
    // THAY ĐỔI: Không cần "delete m_processor;" nữa, std::unique_ptr sẽ tự động giải phóng bộ nhớ.
}

void VideoWorker::processOpenFile(const QString &filePath)
{
    bool success = m_processor->openFile(filePath);
    if (success) {
        VideoProcessor::AudioParams params = m_processor->getAudioParams();
        double frameRate = m_processor->getFrameRate();
        qint64 duration = m_processor->getDuration();
        AVRational timeBase = m_processor->getTimeBase();
        emit fileOpened(true, params, frameRate, duration, timeBase);

        FrameData firstFrame = m_processor->seekAndDecode(0);
        if(!firstFrame.image.isNull()) {
            m_currentPts = firstFrame.pts;
            emit frameReady(firstFrame);
        }
    } else {
        emit fileOpened(false, {}, 0.0, 0, {0, 1});
    }
}

void VideoWorker::processSeek(qint64 timestamp)
{
    FrameData frame = m_processor->seekAndDecode(timestamp);
    if (!frame.image.isNull()) {
        m_currentPts = frame.pts;
        emit frameReady(frame);
    }
}

void VideoWorker::processPlayPause(bool play)
{
    m_isPlaying = play;
    if (m_isPlaying) {
        double frameRate = m_processor->getFrameRate();
        if (frameRate > 0) {
            m_playbackTimer->start(1000 / frameRate);
        }
    } else {
        m_playbackTimer->stop();
    }
}

void VideoWorker::processNextFrame()
{
    if (m_isPlaying) return;
    onPlaybackTimerTimeout();
}

void VideoWorker::processPrevFrame()
{
    if (m_isPlaying) return;

    AVRational timeBase = m_processor->getTimeBase();
    if (timeBase.den == 0) return;
    double frameRate = m_processor->getFrameRate();
    if(frameRate == 0.0) return;

    qint64 currentTimeUs = m_currentPts * 1000000 * timeBase.num / timeBase.den;
    double frameDurationUs = 1000000.0 / frameRate;
    qint64 prevTimeUs = qMax((qint64)0, currentTimeUs - (long long)(1.5 * frameDurationUs));
    
    FrameData frame = m_processor->seekAndDecode(prevTimeUs);
    if (!frame.image.isNull()) {
        m_currentPts = frame.pts;
        emit frameReady(frame);
    }
}

void VideoWorker::stop()
{
    m_isPlaying = false;
    m_playbackTimer->stop();
    if (m_processor) {
        m_processor->stop_processing = true;
    }
    emit finished();
}

void VideoWorker::onPlaybackTimerTimeout()
{
    FrameData frame = m_processor->decodeNextFrame();
    if (!frame.image.isNull()) {
        m_currentPts = frame.pts;
        emit frameReady(frame);
    } else {
        m_isPlaying = false;
        m_playbackTimer->stop();
    }
}
