#include "conversions_controller.h"

#include <drogon/utils/Utilities.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "plugins/app_context.h"

namespace gif_converter {

void ConversionsController::Create(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value error;
        error["type"] = "validation_error";
        error["title"] = "Invalid request body";
        error["status"] = 400;
        error["detail"] = "Request body must be valid JSON.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        callback(resp);
        return;
    }

    ConversionOptions options;
    if (json->isMember("options")) {
        auto& opts = (*json)["options"];
        if (opts.isMember("width")) {
            options.width = opts["width"].asInt();
        }
        if (opts.isMember("fps")) {
            options.fps = opts["fps"].asInt();
        }
    }

    auto id = drogon::utils::getUuid();
    ConversionJob job{
        .id = id,
        .status = ConversionStatus::Pending,
        .input_file_name = json->get("inputFileName", "").asString(),
        .options = options,
        .progress = 0,
        .error_message = std::nullopt,
        .created_at = std::chrono::system_clock::now(),
        .completed_at = std::nullopt,
    };

    auto* ctx = drogon::app().getPlugin<AppContext>();
    ctx->GetConversionRepository().Add(job);

    Json::Value body;
    body["id"] = job.id;
    body["status"] = "pending";
    body["inputFileName"] = job.input_file_name;
    auto time_t = std::chrono::system_clock::to_time_t(job.created_at);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    body["createdAt"] = oss.str();

    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(drogon::k202Accepted);
    resp->addHeader("Location", "/api/v1/conversions/" + job.id);
    callback(resp);
}

}  // namespace gif_converter
