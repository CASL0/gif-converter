#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "conversion_repository.h"

namespace gif_converter {

/** ConversionRepository のインメモリ実装。スレッドセーフ。 */
class InMemoryConversionRepository : public ConversionRepository {
   public:
    InMemoryConversionRepository() = default;
    ~InMemoryConversionRepository() override = default;

    void Add(ConversionJob job) override;
    std::optional<ConversionJob> Find(const std::string& id) const override;
    std::vector<ConversionJob> List(int limit, int offset) const override;
    int Count() const override;
    bool Remove(const std::string& id) override;

   private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, ConversionJob> jobs_;
    std::vector<std::string> order_;
};

}  // namespace gif_converter
