#include "app_context.h"

namespace gif_converter {

void AppContext::initAndStart(const Json::Value& /*config*/) {
    repo_ = std::make_shared<InMemoryConversionRepository>();
}

void AppContext::shutdown() {
    {
        std::scoped_lock lock(threads_mutex_);
        for (auto& t : threads_) {
            t.request_stop();
        }
    }
    // jthread のデストラクタが join を呼ぶ
    threads_.clear();
    repo_.reset();
}

ConversionRepository& AppContext::GetConversionRepository() { return *repo_; }

void AppContext::RunAsync(std::function<void(std::stop_token)> task) {
    std::scoped_lock lock(threads_mutex_);
    threads_.emplace_back(std::move(task));
}

}  // namespace gif_converter
