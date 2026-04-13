#include "repositories/in_memory_conversion_repository.h"

#include <gtest/gtest.h>

#include <chrono>

namespace {

gif_converter::ConversionJob MakeJob(const std::string& id) {
    return gif_converter::ConversionJob{
        .id = id,
        .status = gif_converter::ConversionStatus::Pending,
        .input_file_name = "video.mp4",
        .options = {.width = 320, .fps = 10},
        .progress = 0,
        .error_message = std::nullopt,
        .created_at = std::chrono::system_clock::now(),
        .completed_at = std::nullopt,
    };
}

class InMemoryConversionRepositoryTest : public ::testing::Test {
   protected:
    gif_converter::InMemoryConversionRepository repo_;
};

TEST_F(InMemoryConversionRepositoryTest, AddAndFind) {
    repo_.Add(MakeJob("job-1"));

    auto found = repo_.Find("job-1");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, "job-1");
    EXPECT_EQ(found->input_file_name, "video.mp4");
}

TEST_F(InMemoryConversionRepositoryTest, FindReturnsNulloptForMissing) {
    auto found = repo_.Find("nonexistent");
    EXPECT_FALSE(found.has_value());
}

TEST_F(InMemoryConversionRepositoryTest, ListReturnsNewestFirst) {
    repo_.Add(MakeJob("job-1"));
    repo_.Add(MakeJob("job-2"));
    repo_.Add(MakeJob("job-3"));

    auto result = repo_.List(10, 0);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].id, "job-3");
    EXPECT_EQ(result[1].id, "job-2");
    EXPECT_EQ(result[2].id, "job-1");
}

TEST_F(InMemoryConversionRepositoryTest, ListRespectsLimitAndOffset) {
    repo_.Add(MakeJob("job-1"));
    repo_.Add(MakeJob("job-2"));
    repo_.Add(MakeJob("job-3"));

    auto result = repo_.List(1, 1);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].id, "job-2");
}

TEST_F(InMemoryConversionRepositoryTest, Count) {
    EXPECT_EQ(repo_.Count(), 0);
    repo_.Add(MakeJob("job-1"));
    repo_.Add(MakeJob("job-2"));
    EXPECT_EQ(repo_.Count(), 2);
}

TEST_F(InMemoryConversionRepositoryTest, RemoveExisting) {
    repo_.Add(MakeJob("job-1"));
    EXPECT_TRUE(repo_.Remove("job-1"));
    EXPECT_EQ(repo_.Count(), 0);
    EXPECT_FALSE(repo_.Find("job-1").has_value());
}

TEST_F(InMemoryConversionRepositoryTest, RemoveNonexistentReturnsFalse) {
    EXPECT_FALSE(repo_.Remove("nonexistent"));
}

}  // namespace
