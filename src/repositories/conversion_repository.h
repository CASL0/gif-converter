#pragma once

#include <optional>
#include <string>
#include <vector>

#include "models/conversion_job.h"

namespace gif_converter {

/** 変換ジョブの永続化を抽象化するリポジトリインターフェース。 */
class ConversionRepository {
   public:
    virtual ~ConversionRepository() = default;

    ConversionRepository() = default;
    ConversionRepository(const ConversionRepository&) = delete;
    ConversionRepository& operator=(const ConversionRepository&) = delete;
    ConversionRepository(ConversionRepository&&) = delete;
    ConversionRepository& operator=(ConversionRepository&&) = delete;

    /** ジョブを保存する。 */
    virtual void Add(ConversionJob job) = 0;

    /**
     * ID でジョブを検索する。
     * @param id 検索対象のジョブ ID
     * @return 見つかった場合は ConversionJob、見つからない場合は std::nullopt
     */
    virtual std::optional<ConversionJob> Find(const std::string& id) const = 0;

    /**
     * ジョブ一覧を新しい順に取得する。
     * @param limit 最大取得件数
     * @param offset スキップする件数
     */
    virtual std::vector<ConversionJob> List(int limit, int offset) const = 0;

    /** 保存されているジョブの総数を返す。 */
    virtual int Count() const = 0;

    /**
     * 既存のジョブを上書き更新する。
     * @param job 更新後のジョブデータ (id で特定)
     * @return 更新できた場合は true、存在しない場合は false
     */
    virtual bool Update(const ConversionJob& job) = 0;

    /**
     * ID 指定でジョブを削除する。
     * @param id 削除対象のジョブ ID
     * @return 削除できた場合は true、存在しない場合は false
     */
    virtual bool Remove(const std::string& id) = 0;
};

}  // namespace gif_converter
