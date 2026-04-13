#pragma once

#include <drogon/HttpController.h>

namespace gif_converter {

/** ヘルスチェック用コントローラ。 */
class HealthController : public drogon::HttpController<HealthController> {
   public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HealthController::GetHealth, "/health", drogon::Get);
    METHOD_LIST_END

    /** サーバーの稼働状態を返す (GET /health)。 */
    void GetHealth(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

}  // namespace gif_converter
