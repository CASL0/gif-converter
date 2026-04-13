#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace gif_converter {

/** 変換ジョブの状態。 */
enum class ConversionStatus {
    Pending,    /**< 受付済み・処理待ち */
    Processing, /**< 変換処理中 */
    Completed,  /**< 変換完了 */
    Failed,     /**< 変換失敗 */
};

/** GIF 変換のオプション。 */
struct ConversionOptions {
    int width = 320; /**< 出力 GIF の幅 (px) */
    int fps = 10;    /**< 出力 GIF のフレームレート */
};

/** 変換ジョブを表すデータモデル。 */
struct ConversionJob {
    std::string id; /**< UUID */
    ConversionStatus status = ConversionStatus::Pending;
    std::string input_file_name;              /**< アップロードされた元ファイル名 */
    std::string input_file_path;              /**< サーバー上の保存パス */
    ConversionOptions options;                /**< 変換オプション */
    int progress = 0;                         /**< 進捗率 (0–100) */
    std::optional<std::string> error_message; /**< 失敗時のエラーメッセージ */
    std::chrono::system_clock::time_point created_at;
    std::optional<std::chrono::system_clock::time_point> completed_at;
};

}  // namespace gif_converter
