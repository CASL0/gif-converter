#pragma once

#include <expected>
#include <string>

#include "models/conversion_job.h"

namespace gif_converter {

/**
 * FFmpeg を使って動画ファイルを GIF に変換するサービス。
 * 同期的に変換を実行し、完了/失敗を返す。
 * 非同期実行は呼び出し側の責務。
 */
class GifConverterService {
   public:
    /**
     * 動画を GIF に変換する。
     * @param job 変換ジョブ (input_file_path, options を使用)
     * @param output_path 出力 GIF のパス
     * @return 成功時は std::expected<void>、失敗時はエラーメッセージ
     */
    static std::expected<void, std::string> Convert(const ConversionJob& job,
                                                    const std::string& output_path);
};

}  // namespace gif_converter
