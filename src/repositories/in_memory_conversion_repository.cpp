#include "in_memory_conversion_repository.h"

#include <algorithm>

namespace gif_converter {

void InMemoryConversionRepository::Add(ConversionJob job) {
    std::scoped_lock lock(mutex_);
    auto id = job.id;
    jobs_.emplace(id, std::move(job));
    order_.push_back(std::move(id));
}

std::optional<ConversionJob> InMemoryConversionRepository::Find(const std::string& id) const {
    std::scoped_lock lock(mutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<ConversionJob> InMemoryConversionRepository::List(int limit, int offset) const {
    std::scoped_lock lock(mutex_);
    std::vector<ConversionJob> result;

    auto begin = static_cast<int>(order_.size()) - 1 - offset;
    if (begin < 0) {
        return result;
    }

    for (int i = begin; i >= 0 && static_cast<int>(result.size()) < limit; --i) {
        auto it = jobs_.find(order_[static_cast<size_t>(i)]);
        if (it != jobs_.end()) {
            result.push_back(it->second);
        }
    }

    return result;
}

int InMemoryConversionRepository::Count() const {
    std::scoped_lock lock(mutex_);
    return static_cast<int>(jobs_.size());
}

bool InMemoryConversionRepository::Remove(const std::string& id) {
    std::scoped_lock lock(mutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end()) {
        return false;
    }
    jobs_.erase(it);
    order_.erase(std::remove(order_.begin(), order_.end(), id), order_.end());
    return true;
}

}  // namespace gif_converter
