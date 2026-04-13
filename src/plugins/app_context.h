#pragma once

#include <drogon/plugins/Plugin.h>

#include <memory>

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
    void shutdown() override;

    /** 変換ジョブリポジトリを返す。 */
    ConversionRepository& GetConversionRepository();

   private:
    std::shared_ptr<ConversionRepository> repo_;
};

}  // namespace gif_converter
