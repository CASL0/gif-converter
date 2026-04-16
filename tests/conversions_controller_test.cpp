#include <drogon/drogon.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <future>
#include <string>

#include "test_server.h"

namespace {

const std::string kTestUploadDir = "/tmp/gif-converter-test";

/** テスト用の一時ファイルを作成し、そのパスを返す。 */
std::string CreateTempFile(const std::string& filename, const std::string& content) {
    std::filesystem::create_directories(kTestUploadDir);
    auto path = std::filesystem::path(kTestUploadDir) / filename;
    std::ofstream ofs(path, std::ios::binary);
    ofs << content;
    return path.string();
}

drogon::HttpRequestPtr MakeUploadRequest(const std::string& file_path) {
    auto req = drogon::HttpRequest::newFileUploadRequest({drogon::UploadFile(file_path)});
    req->setPath("/api/v1/conversions");
    return req;
}

std::string CreateJobAndGetId(const drogon::HttpClientPtr& client) {
    std::promise<std::string> id_promise;
    auto id_future = id_promise.get_future();

    auto path = CreateTempFile("test.mp4", "fake video data");
    auto req = MakeUploadRequest(path);
    client->sendRequest(
        req, [&id_promise](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
            ASSERT_EQ(result, drogon::ReqResult::Ok);
            auto json = resp->getJsonObject();
            ASSERT_NE(json, nullptr);
            id_promise.set_value((*json)["id"].asString());
        });

    return id_future.get();
}

TEST(ConversionsControllerTest, CreateReturns202WithJobId) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto path = CreateTempFile("video.mp4", "fake video data");
    auto req = MakeUploadRequest(path);
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

TEST(ConversionsControllerTest, CreateWithInvalidBodyReturns400) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions");
    req->setMethod(drogon::Post);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody("not multipart");

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

TEST(ConversionsControllerTest, GetOneReturnsJob) {
    auto client = gif_converter::test::TestServer::CreateClient();
    auto job_id = CreateJobAndGetId(client);

    std::promise<void> done;
    auto done_future = done.get_future();

    auto get_req = drogon::HttpRequest::newHttpRequest();
    get_req->setPath("/api/v1/conversions/" + job_id);
    get_req->setMethod(drogon::Get);

    client->sendRequest(
        get_req, [&done, &job_id](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
            EXPECT_EQ(result, drogon::ReqResult::Ok);
            EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);

            auto json = resp->getJsonObject();
            ASSERT_NE(json, nullptr);
            EXPECT_EQ((*json)["id"].asString(), job_id);
            EXPECT_EQ((*json)["status"].asString(), "pending");
            EXPECT_TRUE((*json).isMember("progress"));
            EXPECT_TRUE((*json).isMember("options"));
            EXPECT_TRUE((*json).isMember("createdAt"));

            done.set_value();
        });

    done_future.wait();
}

TEST(ConversionsControllerTest, GetOneReturns404ForMissing) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions/nonexistent-id");
    req->setMethod(drogon::Get);

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k404NotFound);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_EQ((*json)["type"].asString(), "not_found");

                            done.set_value();
                        });

    done_future.wait();
}

TEST(ConversionsControllerTest, GetListReturnsItems) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions");
    req->setMethod(drogon::Get);

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_TRUE((*json)["items"].isArray());
                            EXPECT_TRUE((*json)["totalCount"].isInt());
                            EXPECT_EQ((*json)["limit"].asInt(), 20);
                            EXPECT_EQ((*json)["offset"].asInt(), 0);

                            done.set_value();
                        });

    done_future.wait();
}

TEST(ConversionsControllerTest, GetListRespectsLimitAndOffset) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions?limit=2&offset=1");
    req->setMethod(drogon::Get);

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_LE((*json)["items"].size(), 2u);
                            EXPECT_EQ((*json)["limit"].asInt(), 2);
                            EXPECT_EQ((*json)["offset"].asInt(), 1);

                            done.set_value();
                        });

    done_future.wait();
}

TEST(ConversionsControllerTest, DeleteReturns204) {
    auto client = gif_converter::test::TestServer::CreateClient();
    auto job_id = CreateJobAndGetId(client);

    std::promise<void> done;
    auto done_future = done.get_future();

    auto del_req = drogon::HttpRequest::newHttpRequest();
    del_req->setPath("/api/v1/conversions/" + job_id);
    del_req->setMethod(drogon::Delete);

    client->sendRequest(del_req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k204NoContent);

                            done.set_value();
                        });

    done_future.wait();
}

TEST(ConversionsControllerTest, DeleteReturns404ForMissing) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions/nonexistent-id");
    req->setMethod(drogon::Delete);

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k404NotFound);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_EQ((*json)["type"].asString(), "not_found");

                            done.set_value();
                        });

    done_future.wait();
}

TEST(ConversionsControllerTest, GetResultReturns404ForMissing) {
    auto client = gif_converter::test::TestServer::CreateClient();

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions/nonexistent-id/result");
    req->setMethod(drogon::Get);

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k404NotFound);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_EQ((*json)["type"].asString(), "not_found");

                            done.set_value();
                        });

    done_future.wait();
}

TEST(ConversionsControllerTest, GetResultReturns409WhenNotCompleted) {
    auto client = gif_converter::test::TestServer::CreateClient();
    auto job_id = CreateJobAndGetId(client);

    std::promise<void> done;
    auto done_future = done.get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/conversions/" + job_id + "/result");
    req->setMethod(drogon::Get);

    client->sendRequest(req,
                        [&done](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                            EXPECT_EQ(result, drogon::ReqResult::Ok);
                            EXPECT_EQ(resp->getStatusCode(), drogon::k409Conflict);

                            auto json = resp->getJsonObject();
                            ASSERT_NE(json, nullptr);
                            EXPECT_EQ((*json)["type"].asString(), "not_ready");

                            done.set_value();
                        });

    done_future.wait();
}

}  // namespace
