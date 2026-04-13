#pragma once

#include <drogon/drogon.h>

#include <future>
#include <string>
#include <thread>

namespace gif_converter::test {

/**
 * テスト用の Drogon サーバーをプロセス内で1回だけ起動する。
 * GTest の Environment として登録して使う。
 */
class TestServer : public ::testing::Environment {
   public:
    void SetUp() override {
        std::promise<void> ready;
        auto ready_future = ready.get_future();

        thread_ = std::thread([&ready]() {
            drogon::app().addPlugin("gif_converter::AppContext", {}, {});
            drogon::app()
                .setLogLevel(trantor::Logger::kWarn)
                .addListener("127.0.0.1", 0)
                .setThreadNum(1)
                .registerBeginningAdvice([&ready]() { ready.set_value(); })
                .run();
        });

        ready_future.wait();
    }

    void TearDown() override {
        drogon::app().quit();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    /** テスト用 HTTP クライアントを返す。 */
    static drogon::HttpClientPtr CreateClient() {
        auto listeners = drogon::app().getListeners();
        auto port = listeners[0].toPort();
        return drogon::HttpClient::newHttpClient("http://127.0.0.1:" + std::to_string(port));
    }

   private:
    std::thread thread_;
};

}  // namespace gif_converter::test
