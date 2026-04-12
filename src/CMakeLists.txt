add_executable(gif-converter
    main.cpp
    logger.cpp
    controllers/health_controller.cpp
)

target_compile_features(gif-converter PRIVATE cxx_std_20)

target_compile_options(gif-converter PRIVATE
    -Wall -Wextra -Wpedantic -Werror
)

target_link_libraries(gif-converter PRIVATE
    Drogon::Drogon
    PkgConfig::LIBAV
    quill::quill
)
