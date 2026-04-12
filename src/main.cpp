#include <drogon/drogon.h>

int main() {
    drogon::app()
        .setLogLevel(trantor::Logger::kInfo)
        .addListener("0.0.0.0", 8080)
        .setThreadNum(0)  // auto-detect
        .run();
    return 0;
}
