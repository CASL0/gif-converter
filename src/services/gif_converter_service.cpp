#include "gif_converter_service.h"

#include <cmath>
#include <expected>
#include <string>

#include "ffmpeg/ffmpeg_types.h"

extern "C" {
#include <libavutil/imgutils.h>
}

namespace gif_converter {

namespace {

struct DecoderContext {
    ffmpeg::AvFormatInputPtr format_ctx;
    ffmpeg::AvCodecCtxPtr codec_ctx;
    int stream_index = -1;
};

struct EncoderContext {
    ffmpeg::AvFormatOutputPtr format_ctx;
    ffmpeg::AvCodecCtxPtr codec_ctx;
    AVStream* stream = nullptr;
};

/**
 * 入力ファイルを開きデコーダを初期化する。
 * @param input_path 入力動画ファイルのパス
 * @return 成功時は DecoderContext、失敗時はエラーメッセージ
 */
std::expected<DecoderContext, std::string> OpenDecoder(const std::string& input_path) {
    DecoderContext dec;

    AVFormatContext* raw_ctx = nullptr;
    if (avformat_open_input(&raw_ctx, input_path.c_str(), nullptr, nullptr) < 0) {
        return std::unexpected("Failed to open input file.");
    }
    dec.format_ctx.reset(raw_ctx);

    if (avformat_find_stream_info(dec.format_ctx.get(), nullptr) < 0) {
        return std::unexpected("Failed to find stream info.");
    }

    const AVCodec* decoder = nullptr;
    dec.stream_index =
        av_find_best_stream(dec.format_ctx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (dec.stream_index < 0) {
        return std::unexpected("No video stream found.");
    }

    dec.codec_ctx.reset(avcodec_alloc_context3(decoder));
    if (!dec.codec_ctx) {
        return std::unexpected("Failed to allocate decoder context.");
    }
    if (avcodec_parameters_to_context(dec.codec_ctx.get(),
                                      dec.format_ctx->streams[dec.stream_index]->codecpar) < 0) {
        return std::unexpected("Failed to copy codec parameters.");
    }
    if (avcodec_open2(dec.codec_ctx.get(), decoder, nullptr) < 0) {
        return std::unexpected("Failed to open decoder.");
    }
    return dec;
}

/**
 * GIF エンコーダと出力ファイルを初期化する。
 * @param output_path 出力 GIF ファイルのパス
 * @param width 出力幅 (px)
 * @param height 出力高さ (px)
 * @param fps 出力フレームレート
 * @return 成功時は EncoderContext、失敗時はエラーメッセージ
 */
std::expected<EncoderContext, std::string> OpenEncoder(const std::string& output_path, int width,
                                                       int height, int fps) {
    EncoderContext enc;

    AVFormatContext* raw_ctx = nullptr;
    if (avformat_alloc_output_context2(&raw_ctx, nullptr, "gif", output_path.c_str()) < 0) {
        return std::unexpected("Failed to create output context.");
    }
    enc.format_ctx.reset(raw_ctx);

    const AVCodec* gif_encoder = avcodec_find_encoder(AV_CODEC_ID_GIF);
    if (!gif_encoder) {
        return std::unexpected("GIF encoder not found.");
    }

    enc.stream = avformat_new_stream(enc.format_ctx.get(), nullptr);
    if (!enc.stream) {
        return std::unexpected("Failed to create output stream.");
    }

    enc.codec_ctx.reset(avcodec_alloc_context3(gif_encoder));
    if (!enc.codec_ctx) {
        return std::unexpected("Failed to allocate encoder context.");
    }
    enc.codec_ctx->width = width;
    enc.codec_ctx->height = height;
    enc.codec_ctx->pix_fmt = AV_PIX_FMT_RGB8;
    enc.codec_ctx->time_base = AVRational{1, fps};
    enc.codec_ctx->framerate = AVRational{fps, 1};

    if (avcodec_open2(enc.codec_ctx.get(), gif_encoder, nullptr) < 0) {
        return std::unexpected("Failed to open GIF encoder.");
    }
    if (avcodec_parameters_from_context(enc.stream->codecpar, enc.codec_ctx.get()) < 0) {
        return std::unexpected("Failed to copy encoder parameters.");
    }
    enc.stream->time_base = enc.codec_ctx->time_base;

    if (avio_open(&enc.format_ctx->pb, output_path.c_str(), AVIO_FLAG_WRITE) < 0) {
        return std::unexpected("Failed to open output file.");
    }

    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "loop", "0", 0);
    int ret = avformat_write_header(enc.format_ctx.get(), &opts);
    av_dict_free(&opts);
    if (ret < 0) {
        return std::unexpected("Failed to write header.");
    }
    return enc;
}

/**
 * スケール済みフレームを GIF エンコーダに送りパケットを書き出す。
 * @param frame エンコード対象のフレーム (nullptr でフラッシュ)
 * @param enc エンコーダコンテキスト
 * @param pkt 作業用パケット (再利用)
 */
void EncodeAndWrite(AVFrame* frame, EncoderContext& enc, AVPacket* pkt) {
    avcodec_send_frame(enc.codec_ctx.get(), frame);
    while (avcodec_receive_packet(enc.codec_ctx.get(), pkt) == 0) {
        av_packet_rescale_ts(pkt, enc.codec_ctx->time_base, enc.stream->time_base);
        pkt->stream_index = enc.stream->index;
        av_interleaved_write_frame(enc.format_ctx.get(), pkt);
    }
}

/**
 * デコード→スケール→エンコードのメインループを実行する。
 * @param dec デコーダコンテキスト
 * @param enc エンコーダコンテキスト
 * @param sws_ctx スケーラコンテキスト
 * @param target_fps 出力フレームレート
 * @return 成功時は std::expected<void>、失敗時はエラーメッセージ
 */
std::expected<void, std::string> TranscodeFrames(DecoderContext& dec, EncoderContext& enc,
                                                 SwsContext* sws_ctx, int target_fps) {
    ffmpeg::AvPacketPtr pkt(av_packet_alloc());
    ffmpeg::AvFramePtr dec_frame(av_frame_alloc());
    ffmpeg::AvFramePtr scaled_frame(av_frame_alloc());
    if (!pkt || !dec_frame || !scaled_frame) {
        return std::unexpected("Failed to allocate frame/packet.");
    }

    scaled_frame->format = AV_PIX_FMT_RGB8;
    scaled_frame->width = enc.codec_ctx->width;
    scaled_frame->height = enc.codec_ctx->height;
    if (av_frame_get_buffer(scaled_frame.get(), 0) < 0) {
        return std::unexpected("Failed to allocate scaled frame buffer.");
    }

    double input_fps = av_q2d(dec.format_ctx->streams[dec.stream_index]->r_frame_rate);
    int frame_step = std::max(1, static_cast<int>(std::round(input_fps / target_fps)));
    int64_t input_frame_count = 0;
    int64_t output_frame_index = 0;

    auto process_decoded_frame = [&]() {
        if (++input_frame_count % frame_step != 0) {
            return;
        }
        sws_scale(sws_ctx, dec_frame->data, dec_frame->linesize, 0, dec_frame->height,
                  scaled_frame->data, scaled_frame->linesize);
        scaled_frame->pts = output_frame_index++;
        EncodeAndWrite(scaled_frame.get(), enc, pkt.get());
    };

    while (av_read_frame(dec.format_ctx.get(), pkt.get()) >= 0) {
        if (pkt->stream_index != dec.stream_index) {
            av_packet_unref(pkt.get());
            continue;
        }
        avcodec_send_packet(dec.codec_ctx.get(), pkt.get());
        av_packet_unref(pkt.get());

        while (avcodec_receive_frame(dec.codec_ctx.get(), dec_frame.get()) == 0) {
            process_decoded_frame();
        }
    }

    // デコーダフラッシュ
    avcodec_send_packet(dec.codec_ctx.get(), nullptr);
    while (avcodec_receive_frame(dec.codec_ctx.get(), dec_frame.get()) == 0) {
        process_decoded_frame();
    }

    // エンコーダフラッシュ
    EncodeAndWrite(nullptr, enc, pkt.get());

    return {};
}

}  // namespace

std::expected<void, std::string> GifConverterService::Convert(const ConversionJob& job,
                                                              const std::string& output_path) {
    auto dec_result = OpenDecoder(job.input_file_path);
    if (!dec_result) {
        return std::unexpected(dec_result.error());
    }
    auto& dec = *dec_result;

    int out_width = job.options.width;
    int out_height = (dec.codec_ctx->height * out_width) / dec.codec_ctx->width;
    out_height = std::max(out_height & ~1, 2);

    auto enc_result = OpenEncoder(output_path, out_width, out_height, job.options.fps);
    if (!enc_result) {
        return std::unexpected(enc_result.error());
    }
    auto& enc = *enc_result;

    ffmpeg::SwsCtxPtr sws_ctx(sws_getContext(
        dec.codec_ctx->width, dec.codec_ctx->height, dec.codec_ctx->pix_fmt, out_width, out_height,
        AV_PIX_FMT_RGB8, SWS_LANCZOS, nullptr, nullptr, nullptr));
    if (!sws_ctx) {
        return std::unexpected("Failed to create scaler.");
    }

    auto transcode_result = TranscodeFrames(dec, enc, sws_ctx.get(), job.options.fps);
    if (!transcode_result) {
        return std::unexpected(transcode_result.error());
    }

    av_write_trailer(enc.format_ctx.get());
    return {};
}

}  // namespace gif_converter
