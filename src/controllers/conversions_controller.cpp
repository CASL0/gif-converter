#include "conversions_controller.h"

#include <drogon/MultiPart.h>
#include <drogon/utils/Utilities.h>

#include <chrono>
#include <ctime>
#include <filesystem>
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

constexpr size_t kMaxUploadSize = 500 * 1024 * 1024; /**< 500MB */
const std::string kUploadDir = "/tmp/gif-converter/uploads";

/** RFC 9457 形式のエラーレスポンスを生成する。 */
drogon::HttpResponsePtr MakeErrorResponse(drogon::HttpStatusCode status_code,
                                          const std::string& type, const std::string& title,
                                          const std::string& detail,
                                          const std::string& instance = "") {
    Json::Value error;
    error["type"] = type;
    error["title"] = title;
    error["status"] = static_cast<int>(status_code);
    error["detail"] = detail;
    if (!instance.empty()) {
        error["instance"] = instance;
    }
    auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
    resp->setStatusCode(status_code);
    return resp;
}

}  // namespace

void ConversionsController::Create(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    drogon::MultiPartParser parser;
    if (parser.parse(req) != 0) {
        callback(MakeErrorResponse(drogon::k400BadRequest, "validation_error",
                                   "Invalid request body", "Request must be multipart/form-data.",
                                   "/api/v1/conversions"));
        return;
    }

    auto files_map = parser.getFilesMap();
    auto file_it = files_map.find("file");
    if (file_it == files_map.end()) {
        callback(MakeErrorResponse(drogon::k400BadRequest, "validation_error", "Missing file",
                                   "A 'file' field is required.", "/api/v1/conversions"));
        return;
    }

    auto& file = file_it->second;
    if (file.fileLength() > kMaxUploadSize) {
        callback(MakeErrorResponse(drogon::k413RequestEntityTooLarge, "file_too_large",
                                   "File too large", "Upload size must not exceed 500MB.",
                                   "/api/v1/conversions"));
        return;
    }

    ConversionOptions options;
    auto& params = parser.getParameters();
    if (auto it = params.find("width"); it != params.end()) {
        options.width = std::stoi(it->second);
    }
    if (auto it = params.find("fps"); it != params.end()) {
        options.fps = std::stoi(it->second);
    }

    auto id = drogon::utils::getUuid();
    std::filesystem::create_directories(kUploadDir);
    auto save_path = std::filesystem::path(kUploadDir) /
                     (id + std::filesystem::path(file.getFileName()).extension().string());
    file.saveAs(save_path.string());

    ConversionJob job{
        .id = id,
        .status = ConversionStatus::Pending,
        .input_file_name = file.getFileName(),
        .input_file_path = save_path.string(),
        .output_file_path = {},
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
        callback(MakeErrorResponse(drogon::k404NotFound, "not_found", "Conversion not found",
                                   "No conversion job with ID '" + id + "' exists.",
                                   "/api/v1/conversions/" + id));
        return;
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(JobToJson(*job));
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}

void ConversionsController::Delete(const drogon::HttpRequestPtr& /*req*/,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   const std::string& id) {
    auto* ctx = drogon::app().getPlugin<AppContext>();
    auto removed = ctx->GetConversionRepository().Remove(id);

    if (!removed) {
        callback(MakeErrorResponse(drogon::k404NotFound, "not_found", "Conversion not found",
                                   "No conversion job with ID '" + id + "' exists.",
                                   "/api/v1/conversions/" + id));
        return;
    }

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k204NoContent);
    callback(resp);
}

void ConversionsController::GetResult(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    auto* ctx = drogon::app().getPlugin<AppContext>();
    auto job = ctx->GetConversionRepository().Find(id);

    if (!job) {
        callback(MakeErrorResponse(drogon::k404NotFound, "not_found", "Conversion not found",
                                   "No conversion job with ID '" + id + "' exists.",
                                   "/api/v1/conversions/" + id + "/result"));
        return;
    }

    if (job->status != ConversionStatus::Completed) {
        callback(
            MakeErrorResponse(drogon::k409Conflict, "not_ready", "Conversion not completed",
                              "The conversion job is still " + StatusToString(job->status) + ".",
                              "/api/v1/conversions/" + id + "/result"));
        return;
    }

    if (!std::filesystem::exists(job->output_file_path)) {
        callback(MakeErrorResponse(drogon::k500InternalServerError, "internal_error",
                                   "Result file missing",
                                   "The conversion result file could not be found."));
        return;
    }

    auto resp = drogon::HttpResponse::newFileResponse(job->output_file_path, "", drogon::CT_NONE);
    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeString("image/gif");
    callback(resp);
}

}  // namespace gif_converter
