// videoprocessor.cpp - Version 1.6 (Fixed typo)
#include "videoprocessor.h"
#include <QDebug>

VideoProcessor::VideoProcessor() : stop_processing(false) {}
VideoProcessor::~VideoProcessor() { cleanup(); }

bool VideoProcessor::openFile(const QString &filePath)
{
    cleanup();
    stop_processing = false; // Reset cờ khi mở file mới
    std::string filePathStr = filePath.toStdString();
    if (avformat_open_input(&formatContext, filePathStr.c_str(), nullptr, nullptr) != 0) return false;
    if (avformat_find_stream_info(formatContext, nullptr) < 0) { cleanup(); return false; }

    // --- Video Stream ---
    videoStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &videoCodec, 0);
    if (videoStreamIndex >= 0) {
        const AVCodecParameters *codecParameters = formatContext->streams[videoStreamIndex]->codecpar;
        videoCodec = avcodec_find_decoder(codecParameters->codec_id);
        if (videoCodec) {
            videoCodecContext = avcodec_alloc_context3(videoCodec);
            if (videoCodecContext && avcodec_parameters_to_context(videoCodecContext, codecParameters) >= 0) {
                if (avcodec_open2(videoCodecContext, videoCodec, nullptr) < 0) { cleanup(); return false; }
            } else { cleanup(); return false; }
        } else { cleanup(); return false; }
    } else { cleanup(); return false; }

    // --- Audio Stream ---
    audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, videoStreamIndex, &audioCodec, 0);
    if (audioStreamIndex >= 0) {
        const AVCodecParameters *codecParameters = formatContext->streams[audioStreamIndex]->codecpar;
        audioCodec = avcodec_find_decoder(codecParameters->codec_id);
        if (audioCodec) {
            audioCodecContext = avcodec_alloc_context3(audioCodec);
            if (audioCodecContext && avcodec_parameters_to_context(audioCodecContext, codecParameters) >= 0) {
                if (avcodec_open2(audioCodecContext, audioCodec, nullptr) < 0) { audioStreamIndex = -1; }
                else {
                    swrContext = swr_alloc();
                    av_opt_set_chlayout(swrContext, "in_chlayout", &audioCodecContext->ch_layout, 0);
                    av_opt_set_int(swrContext, "in_sample_rate", audioCodecContext->sample_rate, 0);
                    av_opt_set_sample_fmt(swrContext, "in_sample_fmt", audioCodecContext->sample_fmt, 0);

                    AVChannelLayout out_ch_layout;
                    av_channel_layout_default(&out_ch_layout, 2);
                    av_opt_set_chlayout(swrContext, "out_chlayout", &out_ch_layout, 0);
                    av_opt_set_int(swrContext, "out_sample_rate", 44100, 0);
                    av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
                    swr_init(swrContext);

                    m_audioParams = {true, 44100, 2};
                }
            } else { audioStreamIndex = -1; }
        } else { audioStreamIndex = -1; }
    }
    
    qDebug() << "Successfully opened video file:" << filePath;
    return true;
}

FrameData VideoProcessor::decodeNextFrame()
{
    if (!formatContext) return {};
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    FrameData result;

    while (av_read_frame(formatContext, packet) >= 0 && !stop_processing) {
        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(videoCodecContext, packet) == 0) {
                if (avcodec_receive_frame(videoCodecContext, frame) == 0) {
                    result.image = convertFrameToImage(frame);
                    result.pts = frame->pts;
                }
            }
        } else if (packet->stream_index == audioStreamIndex) {
            if (avcodec_send_packet(audioCodecContext, packet) == 0) {
                if (avcodec_receive_frame(audioCodecContext, frame) == 0) {
                    result.audioData.append(resampleAudioFrame(frame));
                }
            }
        }
        av_packet_unref(packet);
        
        if(!result.image.isNull()) {
            av_frame_free(&frame);
            av_packet_free(&packet);
            return result;
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    return {};
}

FrameData VideoProcessor::seekAndDecode(int64_t target_ts_us)
{
    if (!seek(target_ts_us)) return {};
    FrameData frameData;
    while (!stop_processing) {
        frameData = decodeNextFrame();
        if (frameData.image.isNull()) return frameData;
        AVRational timeBase = getTimeBase();
        if (timeBase.den == 0) return {};
        int64_t current_ts_us = frameData.pts * 1000000 * timeBase.num / timeBase.den;
        if (current_ts_us >= target_ts_us) return frameData;
    }
    return {}; // Trả về rỗng nếu bị dừng
}

bool VideoProcessor::seek(int64_t timestamp)
{
    if (!formatContext) return false;
    int64_t seek_target = av_rescale(timestamp, formatContext->streams[videoStreamIndex]->time_base.den, formatContext->streams[videoStreamIndex]->time_base.num) / 1000000;
    if (av_seek_frame(formatContext, videoStreamIndex, seek_target, AVSEEK_FLAG_BACKWARD) < 0) return false;
    avcodec_flush_buffers(videoCodecContext);
    if(audioCodecContext) avcodec_flush_buffers(audioCodecContext);
    return true;
}

int64_t VideoProcessor::getDuration() const { return formatContext ? formatContext->duration : 0; }
AVRational VideoProcessor::getTimeBase() const { return (formatContext && videoStreamIndex >= 0) ? formatContext->streams[videoStreamIndex]->time_base : AVRational{0, 1}; }
double VideoProcessor::getFrameRate() const { if (formatContext && videoStreamIndex >= 0) { AVRational fr = formatContext->streams[videoStreamIndex]->avg_frame_rate; return (double)fr.num / fr.den; } return 0.0; }
VideoProcessor::AudioParams VideoProcessor::getAudioParams() const { return m_audioParams; }

QImage VideoProcessor::convertFrameToImage(AVFrame* frame)
{
    if (!frame) return QImage();
    swsContext = sws_getCachedContext(swsContext, frame->width, frame->height, (AVPixelFormat)frame->format, frame->width, frame->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsContext) return QImage();
    QImage image(frame->width, frame->height, QImage::Format_RGB888);
    uint8_t* const data[] = { image.bits() };
    const int linesize[] = { static_cast<int>(image.bytesPerLine()) };
    sws_scale(swsContext, (const uint8_t* const*)frame->data, frame->linesize, 0, frame->height, data, linesize);
    return image;
}

QByteArray VideoProcessor::resampleAudioFrame(AVFrame *frame)
{
    if (!swrContext || !frame) return QByteArray();
    
    uint8_t **dst_data = nullptr;
    int dst_linesize;
    int dst_nb_samples = av_rescale_rnd(swr_get_delay(swrContext, frame->sample_rate) + frame->nb_samples, 44100, frame->sample_rate, AV_ROUND_UP);

    av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, 2, dst_nb_samples, AV_SAMPLE_FMT_S16, 0);

    int out_samples = swr_convert(swrContext, dst_data, dst_nb_samples, (const uint8_t **)frame->extended_data, frame->nb_samples);
    
    QByteArray buffer;
    if(out_samples > 0) {
        buffer = QByteArray(reinterpret_cast<char*>(dst_data[0]), out_samples * 2 * sizeof(int16_t));
    }

    if (dst_data) av_freep(&dst_data[0]);
    av_freep(&dst_data);

    return buffer;
}

void VideoProcessor::cleanup()
{
    stop_processing = true; 
    // SỬA LỖI: Sửa lỗi đánh máy
    if (swsContext) { sws_freeContext(swsContext); swsContext = nullptr; }
    if (videoCodecContext) { avcodec_free_context(&videoCodecContext); videoCodecContext = nullptr; }
    if (swrContext) { swr_free(&swrContext); swrContext = nullptr; }
    if (audioCodecContext) { avcodec_free_context(&audioCodecContext); audioCodecContext = nullptr; }
    if (formatContext) { avformat_close_input(&formatContext); formatContext = nullptr; }
    videoStreamIndex = -1; videoCodec = nullptr;
    audioStreamIndex = -1; audioCodec = nullptr;
    m_audioParams = {false, 0, 0};
}
