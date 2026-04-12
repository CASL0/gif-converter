#include "health_controller.h"

#include <chrono>

namespace gif_converter {

namespace {
const auto kStartTime = std::chrono::steady_clock::now();
}

void HealthController::GetHealth(const drogon::HttpRequestPtr& /*req*/,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - kStartTime).count();

    Json::Value json;
    json["status"] = "healthy";
    json["version"] = "0.1.0";
    json["uptime"] = static_cast<Json::Int64>(uptime);

    auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}

}  // namespace gif_converter
