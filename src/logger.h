#pragma once

#include <quill/Logger.h>

namespace gif_converter {

/** Quill バックエンドを起動し、JSON 形式の標準出力 logger を初期化する。 */
void InitLogger();

/** アプリケーション共有の logger を返す。InitLogger() 呼び出し前は nullptr。 */
quill::Logger* GetLogger();

}  // namespace gif_converter
