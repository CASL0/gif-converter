#pragma once

#include <drogon/HttpController.h>

namespace gif_converter::api::v1 {

#define GIF_CONVERTER_CONVERSIONS_BASE "/api/v1/conversions"

/** 変換ジョブの CRUD を提供するコントローラ (POST /api/v1/conversions)。 */
class ConversionsController : public drogon::HttpController<ConversionsController> {
   public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ConversionsController::Create, GIF_CONVERTER_CONVERSIONS_BASE, drogon::Post);
    ADD_METHOD_TO(ConversionsController::GetList, GIF_CONVERTER_CONVERSIONS_BASE, drogon::Get);
    ADD_METHOD_TO(ConversionsController::GetOne, GIF_CONVERTER_CONVERSIONS_BASE "/{id}",
                  drogon::Get);
    ADD_METHOD_TO(ConversionsController::Delete, GIF_CONVERTER_CONVERSIONS_BASE "/{id}",
                  drogon::Delete);
    ADD_METHOD_TO(ConversionsController::GetResult, GIF_CONVERTER_CONVERSIONS_BASE "/{id}/result",
                  drogon::Get);
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

    /** 変換結果の GIF を取得する (GET /api/v1/conversions/{id}/result)。 */
    void GetResult(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                   const std::string& id);
};

#undef GIF_CONVERTER_CONVERSIONS_BASE

}  // namespace gif_converter::api::v1
