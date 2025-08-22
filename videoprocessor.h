// videoprocessor.h - Version 1.5
#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QString>
#include <QImage>
#include <QByteArray>
#include <atomic> // Thêm vào để sử dụng std::atomic

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

struct FrameData {
    QImage image;
    QByteArray audioData;
    int64_t pts = 0;
};

class VideoProcessor
{
public:
    struct AudioParams {
        bool isValid = false;
        int sample_rate = 0;
        int channels = 0;
    };

    VideoProcessor();
    ~VideoProcessor();

    bool openFile(const QString &filePath);
    FrameData decodeNextFrame();
    FrameData seekAndDecode(int64_t timestamp);

    int64_t getDuration() const;
    AVRational getTimeBase() const;
    double getFrameRate() const;
    AudioParams getAudioParams() const;

    // THÊM MỚI: Cờ điều khiển an toàn cho đa luồng
    std::atomic<bool> stop_processing;

private:
    void cleanup();
    QImage convertFrameToImage(AVFrame* frame);
    QByteArray resampleAudioFrame(AVFrame* frame);
    bool seek(int64_t timestamp);

    AVFormatContext *formatContext = nullptr;
    // Video
    AVCodecContext *videoCodecContext = nullptr;
    const AVCodec *videoCodec = nullptr;
    SwsContext *swsContext = nullptr;
    int videoStreamIndex = -1;
    // Audio
    AVCodecContext *audioCodecContext = nullptr;
    const AVCodec *audioCodec = nullptr;
    SwrContext *swrContext = nullptr;
    int audioStreamIndex = -1;
    AudioParams m_audioParams;
};

#endif // VIDEOPROCESSOR_H
