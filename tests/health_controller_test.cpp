#include <drogon/drogon.h>
#include <gtest/gtest.h>

#include <future>
#include <thread>

namespace {

class HealthControllerTest : public ::testing::Test {
   protected:
    static void SetUpTestSuite() {
        std::promise<void> ready;
        auto ready_future = ready.get_future();

        server_thread_ = std::thread([&ready]() {
            drogon::app()
                .setLogLevel(trantor::Logger::kWarn)
                .addListener("127.0.0.1", 0)
                .setThreadNum(1)
                .registerBeginningAdvice([&ready]() { ready.set_value(); })
                .run();
        });

        ready_future.wait();
    }

    static void TearDownTestSuite() {
        drogon::app().quit();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    static std::thread server_thread_;
};

std::thread HealthControllerTest::server_thread_;

TEST_F(HealthControllerTest, ReturnsHealthyStatus) {
    auto listeners = drogon::app().getListeners();
    ASSERT_FALSE(listeners.empty());
    auto port = listeners[0].toPort();

    auto client = drogon::HttpClient::newHttpClient("http://127.0.0.1:" + std::to_string(port));

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/health");
    req->setMethod(drogon::Get);

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
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
