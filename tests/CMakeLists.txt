add_executable(gif-converter-tests
    main.cpp
    health_controller_test.cpp
    in_memory_conversion_repository_test.cpp
    conversions_controller_test.cpp
    ../src/controllers/health_controller.cpp
    ../src/controllers/conversions_controller.cpp
    ../src/plugins/app_context.cpp
    ../src/repositories/in_memory_conversion_repository.cpp
)

target_compile_features(gif-converter-tests PRIVATE cxx_std_23)

target_include_directories(gif-converter-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(gif-converter-tests PRIVATE
    GTest::gtest
    Drogon::Drogon
)

include(GoogleTest)
gtest_discover_tests(gif-converter-tests)
