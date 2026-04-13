#include <gtest/gtest.h>

#include "test_server.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new gif_converter::test::TestServer());
    return RUN_ALL_TESTS();
}
