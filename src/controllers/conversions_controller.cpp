#include "conversions_controller.h"

#include <drogon/utils/Utilities.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "plugins/app_context.h"

namespace gif_converter {

namespace {

/** system_clock::time_point を ISO 8601 形式 (UTC) の文字列に変換する。 */
std::string FormatIso8601(std::chrono::system_clock::time_point tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

/** ConversionStatus を JSON レスポンス用の文字列に変換する。 */
std::string StatusToString(ConversionStatus status) {
    switch (status) {
        case ConversionStatus::Pending:
            return "pending";
        case ConversionStatus::Processing:
            return "processing";
        case ConversionStatus::Completed:
            return "completed";
        case ConversionStatus::Failed:
            return "failed";
    }
    return "unknown";
}

/** ConversionJob を JSON レスポンス用の Json::Value に変換する。 */
Json::Value JobToJson(const ConversionJob& job) {
    Json::Value json;
    json["id"] = job.id;
    json["status"] = StatusToString(job.status);
    json["inputFileName"] = job.input_file_name;
    json["progress"] = job.progress;
    json["options"]["width"] = job.options.width;
    json["options"]["fps"] = job.options.fps;
    json["createdAt"] = FormatIso8601(job.created_at);
    if (job.completed_at) {
        json["completedAt"] = FormatIso8601(*job.completed_at);
    } else {
        json["completedAt"] = Json::nullValue;
    }
    if (job.error_message) {
        json["errorMessage"] = *job.error_message;
    }
    return json;
}

}  // namespace

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

    auto resp = drogon::HttpResponse::newHttpJsonResponse(JobToJson(job));
    resp->setStatusCode(drogon::k202Accepted);
    resp->addHeader("Location", "/api/v1/conversions/" + job.id);
    callback(resp);
}

void ConversionsController::GetList(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    constexpr int kDefaultLimit = 20;
    constexpr int kMaxLimit = 100;

    int limit = kDefaultLimit;
    int offset = 0;

    if (!req->getParameter("limit").empty()) {
        limit = std::min(std::stoi(req->getParameter("limit")), kMaxLimit);
    }
    if (!req->getParameter("offset").empty()) {
        offset = std::stoi(req->getParameter("offset"));
    }

    auto* ctx = drogon::app().getPlugin<AppContext>();
    auto& repo = ctx->GetConversionRepository();
    auto jobs = repo.List(limit, offset);
    auto total_count = repo.Count();

    Json::Value items(Json::arrayValue);
    for (const auto& job : jobs) {
        items.append(JobToJson(job));
    }

    Json::Value body;
    body["items"] = items;
    body["totalCount"] = total_count;
    body["limit"] = limit;
    body["offset"] = offset;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}

void ConversionsController::GetOne(const drogon::HttpRequestPtr& /*req*/,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   const std::string& id) {
    auto* ctx = drogon::app().getPlugin<AppContext>();
    auto job = ctx->GetConversionRepository().Find(id);

    if (!job) {
        Json::Value error;
        error["type"] = "not_found";
        error["title"] = "Conversion not found";
        error["status"] = 404;
        error["detail"] = "No conversion job with ID '" + id + "' exists.";
        error["instance"] = "/api/v1/conversions/" + id;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(JobToJson(*job));
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}

}  // namespace gif_converter
