#pragma once

#include <drogon/HttpController.h>

namespace gif_converter {

/** 変換ジョブの CRUD を提供するコントローラ (POST /api/v1/conversions)。 */
class ConversionsController : public drogon::HttpController<ConversionsController> {
   public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ConversionsController::Create, "/api/v1/conversions", drogon::Post);
    ADD_METHOD_TO(ConversionsController::GetList, "/api/v1/conversions", drogon::Get);
    ADD_METHOD_TO(ConversionsController::GetOne, "/api/v1/conversions/{id}", drogon::Get);
    ADD_METHOD_TO(ConversionsController::Delete, "/api/v1/conversions/{id}", drogon::Delete);
    METHOD_LIST_END

    /** 新しい変換ジョブを作成する (POST /api/v1/conversions)。 */
    void Create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /** 変換ジョブの一覧を取得する (GET /api/v1/conversions)。 */
    void GetList(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /** 指定 ID の変換ジョブを取得する (GET /api/v1/conversions/{id})。 */
    void GetOne(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                const std::string& id);

    /** 指定 ID の変換ジョブを削除する (DELETE /api/v1/conversions/{id})。 */
    void Delete(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                const std::string& id);
};

}  // namespace gif_converter
