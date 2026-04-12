#pragma once

#include <drogon/HttpController.h>

namespace gif_converter {

class HealthController : public drogon::HttpController<HealthController> {
   public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HealthController::GetHealth, "/health", drogon::Get);
    METHOD_LIST_END

    void GetHealth(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

}  // namespace gif_converter
