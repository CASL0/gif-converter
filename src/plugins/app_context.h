#pragma once

#include <drogon/plugins/Plugin.h>

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "repositories/conversion_repository.h"
#include "repositories/in_memory_conversion_repository.h"

namespace gif_converter {

/**
 * アプリケーション全体の共有リソースを管理する Drogon Plugin。
 * コントローラから drogon::app().getPlugin<AppContext>() で取得する。
 */
class AppContext : public drogon::Plugin<AppContext> {
   public:
    void initAndStart(const Json::Value& config) override;

    /** シャットダウン時にバックグラウンドスレッドの完了を待つ。 */
    void shutdown() override;

    /** 変換ジョブリポジトリを返す。 */
    ConversionRepository& GetConversionRepository();

    /**
     * バックグラウンドでタスクを実行する。
     * shutdown 時に全スレッドの完了を待機する。
     * @param task 実行するタスク (stop_token でキャンセル検知可能)
     */
    void RunAsync(std::function<void(std::stop_token)> task);

   private:
    std::shared_ptr<ConversionRepository> repo_;
    std::mutex threads_mutex_;
    std::vector<std::jthread> threads_;
};

}  // namespace gif_converter
