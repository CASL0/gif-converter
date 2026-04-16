#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <memory>

namespace gif_converter::ffmpeg {

/** AVFormatContext (入力) の RAII ラッパー。 */
struct AvFormatInputDeleter {
    void operator()(AVFormatContext* ctx) const {
        if (ctx) {
            avformat_close_input(&ctx);
        }
    }
};
using AvFormatInputPtr = std::unique_ptr<AVFormatContext, AvFormatInputDeleter>;

/** AVFormatContext (出力) の RAII ラッパー。 */
struct AvFormatOutputDeleter {
    void operator()(AVFormatContext* ctx) const {
        if (ctx) {
            if (!(ctx->oformat->flags & AVFMT_NOFILE)) {
                avio_closep(&ctx->pb);
            }
            avformat_free_context(ctx);
        }
    }
};
using AvFormatOutputPtr = std::unique_ptr<AVFormatContext, AvFormatOutputDeleter>;

/** AVCodecContext の RAII ラッパー。 */
struct AvCodecCtxDeleter {
    void operator()(AVCodecContext* ctx) const { avcodec_free_context(&ctx); }
};
using AvCodecCtxPtr = std::unique_ptr<AVCodecContext, AvCodecCtxDeleter>;

/** SwsContext の RAII ラッパー。 */
struct SwsCtxDeleter {
    void operator()(SwsContext* ctx) const { sws_freeContext(ctx); }
};
using SwsCtxPtr = std::unique_ptr<SwsContext, SwsCtxDeleter>;

/** AVFrame の RAII ラッパー。 */
struct AvFrameDeleter {
    void operator()(AVFrame* frame) const { av_frame_free(&frame); }
};
using AvFramePtr = std::unique_ptr<AVFrame, AvFrameDeleter>;

/** AVPacket の RAII ラッパー。 */
struct AvPacketDeleter {
    void operator()(AVPacket* pkt) const { av_packet_free(&pkt); }
};
using AvPacketPtr = std::unique_ptr<AVPacket, AvPacketDeleter>;

}  // namespace gif_converter::ffmpeg
