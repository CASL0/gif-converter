#pragma once

#include <drogon/HttpController.h>

namespace gif_converter {

/** 変換ジョブの CRUD を提供するコントローラ (POST /api/v1/conversions)。 */
class ConversionsController : public drogon::HttpController<ConversionsController> {
   public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ConversionsController::Create, "/api/v1/conversions", drogon::Post);
    METHOD_LIST_END

    /** 新しい変換ジョブを作成する (POST /api/v1/conversions)。 */
    void Create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

}  // namespace gif_converter
