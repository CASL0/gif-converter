#include <drogon/drogon.h>
#include <quill/LogMacros.h>
#include <trantor/utils/Logger.h>

#include <string_view>

#include "logger.h"

int main() {
    gif_converter::InitLogger();
    auto* logger = gif_converter::GetLogger();

    trantor::Logger::setOutputFunction(
        [logger](const char* msg, const uint64_t len) {
            std::string_view sv(msg, static_cast<size_t>(len));
            while (!sv.empty() && sv.back() == '\n') {
                sv.remove_suffix(1);
            }
            LOG_INFO(logger, "trantor: {}", sv);
        },
        []() {});

    drogon::app()
        .setLogLevel(trantor::Logger::kInfo)
        .addListener("0.0.0.0", 8080)
        .setThreadNum(0)  // auto-detect
        .run();

    return 0;
}
