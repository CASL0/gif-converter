#include <drogon/drogon.h>
#include <gtest/gtest.h>

#include <future>

#include "test_server.h"

namespace {

TEST(ConversionsControllerTest, CreateReturns202WithJobId) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions");
    req->setMethod(drogon::Post);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody(R"({"inputFileName":"video.mp4"})");

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k202Accepted);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_FALSE((*json)["id"].asString().empty());
                            EXPECT_EQ((*json)["status"].asString(), "pending");
                            EXPECT_EQ((*json)["inputFileName"].asString(), "video.mp4");
                            EXPECT_FALSE((*json)["createdAt"].asString().empty());

                            auto location = resp->getHeader("Location");
                            EXPECT_FALSE(location.empty());
                            EXPECT_NE(location.find("/api/v1/conversions/"), std::string::npos);

                            done.set_value();
                        });

    done_future.wait();
}

TEST(ConversionsControllerTest, CreateWithOptionsReturns202) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions");
    req->setMethod(drogon::Post);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody(R"({"inputFileName":"clip.mp4","options":{"width":640,"fps":20}})");

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k202Accepted);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_EQ((*json)["status"].asString(), "pending");

                            done.set_value();
                        });

    done_future.wait();
}

TEST(ConversionsControllerTest, CreateWithInvalidBodyReturns400) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions");
    req->setMethod(drogon::Post);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody("not json");

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_EQ((*json)["type"].asString(), "validation_error");

                            done.set_value();
                        });

    done_future.wait();
}

}  // namespace
