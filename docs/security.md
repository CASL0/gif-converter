# セキュアコーディングガイド

SEI CERT C++ Coding Standard および OWASP セキュアコーディングプラクティスに基づく。
動画ファイルを受け付けるサーバーアプリケーションとして特に重要な項目を整理する。

## メモリ管理 (MEM)

### MEM50-CPP: 解放済みメモリにアクセスしない (Use-After-Free)

```cpp
// Bad: 生ポインタの手動管理
auto* buf = new char[size];
process(buf);
delete[] buf;
buf[0] = 'x';  // undefined behavior

// Good: スマートポインタで自動管理
auto buf = std::make_unique<char[]>(size);
process(buf.get());
// スコープ終了で自動解放
```

### MEM51-CPP: 動的確保したリソースを適切に解放する

- 全てのリソース管理に RAII を適用
- `new`/`delete` は直接使用しない
- ファイルハンドル、ソケット等もスコープで管理

### MEM52-CPP: メモリ確保エラーを検出・処理する

```cpp
// アップロードサイズの事前チェック（OOM攻撃の防止）
constexpr size_t kMaxUploadSize = 500 * 1024 * 1024;  // 500MB
if (content_length > kMaxUploadSize) {
    return HttpResponse::newHttpResponse(k413RequestEntityTooLarge);
}
```

## 整数 (INT)

### INT30-C / INT32-C: 整数オーバーフローの防止

```cpp
// Bad: オーバーフローチェックなし
size_t total = width * height * bytes_per_pixel;

// Good: 事前チェック
if (width > 0 && height > std::numeric_limits<size_t>::max() / width) {
    throw std::overflow_error("dimension overflow");
}
size_t pixels = width * height;
if (pixels > std::numeric_limits<size_t>::max() / bytes_per_pixel) {
    throw std::overflow_error("buffer size overflow");
}
size_t total = pixels * bytes_per_pixel;
```

### INT31-C: 整数変換でデータを失わない

```cpp
// Bad: 暗黙の縮小変換
int frame_count = static_cast<int>(large_value);  // 切り捨ての可能性

// Good: 範囲チェック付き
if (large_value > std::numeric_limits<int>::max()) {
    throw std::out_of_range("frame count exceeds int range");
}
int frame_count = static_cast<int>(large_value);
```

## コンテナ (CTR)

### CTR50-CPP: コンテナのインデックス/イテレータが有効範囲内であることを保証する

```cpp
// Bad: 境界チェックなし
auto value = vec[index];

// Good: 境界チェック付き
auto value = vec.at(index);  // 範囲外で std::out_of_range をスロー
```

### CTR51-CPP: コンテナ要素への参照/イテレータの有効性を保証する

- `insert()`, `erase()`, `resize()` 後はイテレータが無効化される
- 変更後はイテレータを再取得する
- 範囲ベース for ループを優先

## 文字列 (STR)

### STR50-CPP: 文字列データに十分なストレージを保証する

```cpp
// Bad: C スタイル配列（バッファオーバーフローの危険）
char buffer[256];
strcpy(buffer, user_input);

// Good: std::string を使用
std::string safe_input(user_input_ptr ? user_input_ptr : "");
```

### ファイルパスの安全な取り扱い

```cpp
// Bad: 文字列結合
std::string path = upload_dir + "/" + user_filename;

// Good: std::filesystem::path でパストラバーサルを防止
std::filesystem::path base_dir = "/var/uploads";
std::filesystem::path safe_name = GenerateUuid();  // ユーザー入力のファイル名を使用しない
auto full_path = base_dir / safe_name;

// パストラバーサル検証
auto canonical = std::filesystem::weakly_canonical(full_path);
if (canonical.string().find(base_dir.string()) != 0) {
    throw std::runtime_error("path traversal detected");
}
```

## 入出力 (FIO)

### FIO30-C: フォーマット文字列にユーザー入力を含めない

```cpp
// Bad: フォーマット文字列インジェクション
spdlog::info(user_message);

// Good: プレースホルダを使用
spdlog::info("User message: {}", user_message);
```

### FIO51-CPP: 不要になったファイルを閉じる

```cpp
// Good: RAII でファイルハンドルを管理
{
    std::ifstream input(video_path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open file");
    }
    ProcessVideo(input);
}  // スコープ終了で自動クローズ
```

## 式 (EXP)

### EXP53-CPP: 未初期化メモリを読み取らない

```cpp
// Bad
int frame_count;
ProcessFrames(frame_count);  // 不定値

// Good
int frame_count = 0;
```

### EXP61-CPP: ラムダがキャプチャしたオブジェクトより長生きしない

```cpp
// Bad: 参照キャプチャのダングリング
std::function<void()> CreateTask() {
    std::string local_path = "/tmp/video.mp4";
    return [&local_path]() { Process(local_path); };  // ダングリング参照
}

// Good: 値キャプチャ
std::function<void()> CreateTask() {
    std::string local_path = "/tmp/video.mp4";
    return [local_path]() { Process(local_path); };
}
```

## 例外とエラー処理 (ERR)

### ERR56-CPP: 例外安全性を保証する

- **強い保証**: 操作が成功するか、副作用なしにロールバック
- ファイル書き込みは一時ファイル → アトミックリネームパターンを使用

```cpp
// Good: アトミックなファイル出力
void SaveGif(const std::filesystem::path& output_path, const GifData& data) {
    auto temp_path = output_path;
    temp_path += ".tmp";
    WriteGifToFile(temp_path, data);
    std::filesystem::rename(temp_path, output_path);  // アトミック
}
```

### ERR57-CPP: 例外処理時にリソースをリークしない

- 例外ハンドラ内でも RAII を使用
- スマートポインタは例外のアンワインド時に自動解放される

## 並行処理 (CON)

### CON50-CPP: ミューテックスをロックした状態でブロッキング操作を行わない

- デッドロック防止のためロック範囲を最小化
- 複数ミューテックスには `std::scoped_lock` を使用

### CON52-CPP: ミューテックスが保持されている間にロック順序を変えない

```cpp
// Good: scoped_lock で一括ロック（デッドロック回避）
std::scoped_lock lock(mutex_a, mutex_b);
```

## ファイルアップロードセキュリティ (OWASP)

本アプリケーションはユーザーから動画ファイルを受け付けるため、以下を必須とする。

### 1. ファイルサイズ制限

- アップロード開始前に `Content-Length` を検証
- 読み込み中もバイト数を累計し上限を超えたら中断

### 2. ファイルタイプ検証

```cpp
// マジックバイトで検証（拡張子や Content-Type を信用しない）
constexpr std::array<uint8_t, 4> kMp4Magic = {0x66, 0x74, 0x79, 0x70};  // "ftyp"

bool ValidateMp4(const std::vector<uint8_t>& header) {
    if (header.size() < 8) return false;
    return std::equal(kMp4Magic.begin(), kMp4Magic.end(), header.begin() + 4);
}
```

### 3. ファイル名の無害化

- ユーザー提供のファイル名をファイルシステム操作に使用しない
- サーバー側で UUID を生成して使用
- パストラバーサル文字列 (`../`, `..\\`, ヌルバイト) を拒否

### 4. ファイル保存

- Web ルート外に保存: `/var/uploads/`
- 実行権限を付与しない: `chmod 640`
- 一時ファイルは処理完了後に削除

## コンパイラフラグ

### 必須（全ビルド）

```cmake
target_compile_options(gif-converter PRIVATE
    -Wall -Wextra -Wpedantic -Werror
    -Wconversion                    # 暗黙の型変換警告
    -Woverloaded-virtual            # 仮想関数の隠蔽
    -Wnon-virtual-dtor              # virtual デストラクタ欠落
    -fstack-protector-strong         # スタックカナリア
)
```

### デバッグ/テスト時

```cmake
target_compile_options(gif-converter PRIVATE
    -fsanitize=address,undefined    # ASan + UBSan
    -D_GLIBCXX_DEBUG                # STL の境界チェック有効化
)
target_link_options(gif-converter PRIVATE
    -fsanitize=address,undefined
)
```

## 静的解析ツール

| ツール           | コマンド                                     | 用途                |
| ---------------- | -------------------------------------------- | ------------------- |
| Clang-Tidy       | `clang-tidy src/*.cpp -- -std=c++20`         | CERT ルールチェック |
| Cppcheck         | `cppcheck --enable=all src/`                 | 汎用静的解析        |
| AddressSanitizer | `-fsanitize=address` でビルド+テスト実行     | メモリエラー検出    |
| UBSanitizer      | `-fsanitize=undefined` でビルド+テスト実行   | 未定義動作検出      |
| Valgrind         | `valgrind --leak-check=full ./gif-converter` | メモリリーク検出    |

## 参考資料

- [SEI CERT C++ Coding Standard](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [OWASP Secure Coding Practices](https://owasp.org/www-project-secure-coding-practices-quick-reference-guide/)
- [OWASP File Upload Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/File_Upload_Cheat_Sheet.html)
