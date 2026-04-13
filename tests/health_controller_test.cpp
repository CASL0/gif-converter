#include <drogon/drogon.h>
#include <gtest/gtest.h>

#include <future>

#include "test_server.h"

namespace {

TEST(HealthControllerTest, ReturnsHealthyStatus) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/health");
    req->setMethod(drogon::Get);

    client->sendRequest(
        req, [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
            EXPECT_EQ(result, drogon::ReqResult::Ok);
            EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);

            auto json = resp->getJsonObject();
            ASSERT_NE(json, nullptr);
            EXPECT_EQ((*json)["status"].asString(), "healthy");
            EXPECT_EQ((*json)["version"].asString(), "0.1.0");
            EXPECT_TRUE((*json)["uptime"].isInt64());

            done.set_value();
        });

    done_future.wait();
}

}  // namespace
