add_executable(gif-converter-tests
    health_controller_test.cpp
    in_memory_conversion_repository_test.cpp
    ../src/controllers/health_controller.cpp
    ../src/repositories/in_memory_conversion_repository.cpp
)

target_compile_features(gif-converter-tests PRIVATE cxx_std_20)

target_include_directories(gif-converter-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(gif-converter-tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    Drogon::Drogon
)

include(GoogleTest)
gtest_discover_tests(gif-converter-tests)
