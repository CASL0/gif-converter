add_executable(gif-converter
    main.cpp
    logger.cpp
    controllers/health_controller.cpp
    controllers/conversions_controller.cpp
    plugins/app_context.cpp
    repositories/in_memory_conversion_repository.cpp
    services/gif_converter_service.cpp
)

target_compile_features(gif-converter PRIVATE cxx_std_23)

target_include_directories(gif-converter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(gif-converter PRIVATE
    -Wall -Wextra -Wpedantic -Werror
)

target_link_libraries(gif-converter PRIVATE
    Drogon::Drogon
    PkgConfig::LIBAV
    quill::quill
)
