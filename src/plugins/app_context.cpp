#include "app_context.h"

namespace gif_converter {

void AppContext::initAndStart(const Json::Value& /*config*/) {
    repo_ = std::make_shared<InMemoryConversionRepository>();
}

void AppContext::shutdown() { repo_.reset(); }

ConversionRepository& AppContext::GetConversionRepository() { return *repo_; }

}  // namespace gif_converter
