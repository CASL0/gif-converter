#include "logger.h"

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/sinks/JsonSink.h>

namespace gif_converter {

namespace {
quill::Logger* g_logger = nullptr;
}

void InitLogger() {
    quill::Backend::start();

    auto sink = quill::Frontend::create_or_get_sink<quill::JsonConsoleSink>("json_stdout");

    g_logger = quill::Frontend::create_or_get_logger(
        "quill", std::move(sink),
        quill::PatternFormatterOptions{"", "%Y-%m-%dT%H:%M:%S.%QnsZ", quill::Timezone::GmtTime});
}

quill::Logger* GetLogger() { return g_logger; }

}  // namespace gif_converter
