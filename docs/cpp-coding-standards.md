# C++ コーディング規約

本プロジェクトのC++コーディング規約。Google C++ Style Guide および C++ Core Guidelines に基づく。

## 命名規則

| 対象                           | スタイル                 | 例                                     |
| ------------------------------ | ------------------------ | -------------------------------------- |
| クラス / 構造体 / 型エイリアス | PascalCase               | `class RequestHandler`                 |
| 関数                           | PascalCase               | `void HandleRequest()`                 |
| 変数 / 引数                    | snake_case               | `int connection_count`                 |
| プライベートメンバ変数         | snake*case + 末尾 `*`    | `int count_`                           |
| 定数                           | 先頭 `k` + PascalCase    | `constexpr int kMaxConnections = 1024` |
| マクロ                         | ALL_CAPS（使用を避ける） | `#define GIF_CONVERTER_VERSION`        |
| 名前空間                       | snake_case               | `namespace gif_converter`              |
| 列挙値                         | PascalCase               | `enum class Color { Red, Blue }`       |

## ヘッダファイル

- 各ヘッダは自己完結してコンパイル可能であること
- インクルードガードは `#pragma once` を使用
- 可能な場合はフォワード宣言でコンパイル依存を減らす

### インクルード順序

1. 対応するヘッダ（`.cpp` の場合）
2. C システムヘッダ: `<cstdio>`, `<cstring>`
3. C++ 標準ライブラリ: `<vector>`, `<memory>`
4. サードパーティライブラリ
5. プロジェクトヘッダ: `"converter.h"`

各グループ間は空行で区切り、グループ内はアルファベット順。

## クラスと構造体

### struct vs class

- `struct`: 公開データのみの受動的なデータコンテナ（不変条件なし）
- `class`: カプセル化が必要なもの全て

### メンバ宣言順序

1. 型定義 / ネストクラス
2. 静的定数
3. ファクトリメソッド
4. コンストラクタ
5. デストラクタ
6. public メソッド
7. protected / private メソッド
8. データメンバ（private）

### コンストラクタ

- 単一引数のコンストラクタには `explicit` を付ける
- メンバのデフォルト値はクラス内初期化を使用: `int count_ = 0;`
- デフォルト動作で十分なら `= default` を使用
- コンストラクタ内で仮想メソッドを呼ばない

### コピーとムーブ

- コピー/ムーブは明示的に宣言または `= delete`
- ムーブコンストラクタには `noexcept` を付ける
- 継承より合成（コンポジション）を優先

```cpp
class Converter {
  public:
    explicit Converter(std::string input_path);
    ~Converter() = default;

    Converter(const Converter&) = delete;
    Converter& operator=(const Converter&) = delete;
    Converter(Converter&&) noexcept = default;
    Converter& operator=(Converter&&) noexcept = default;

    bool Convert();

  private:
    std::string input_path_;
};
```

## 関数

### 設計原則

- 単一責任: 1関数 = 1つの論理操作
- 行数の目安: 40行以内
- コンパイル時評価が可能なら `constexpr`
- 例外を投げない関数には `noexcept`

### パラメータ

| 目的             | 渡し方                  |
| ---------------- | ----------------------- |
| 入力（小さい型） | 値渡し                  |
| 入力（大きい型） | `const T&`              |
| 出力             | 戻り値（構造体、tuple） |
| 入出力           | 非const参照（控えめに） |

パラメータ順序: 入力 → 出力。

## メモリ管理

### スマートポインタ

- **`std::unique_ptr<T>`**: デフォルトの選択肢。排他的所有権
- **`std::shared_ptr<T>`**: 共有所有権が本当に必要な場合のみ
- **生の `new` / `delete` は使用禁止**

```cpp
// Good
auto converter = std::make_unique<GifConverter>(config);

// Bad
GifConverter* converter = new GifConverter(config);
```

### RAII

全てのリソース管理に RAII を適用する。

- メモリ: スマートポインタ
- ミューテックス: `std::lock_guard`, `std::scoped_lock`
- ファイル: RAII ラッパー

## エラー処理

- 回復可能なエラーには例外を使用
- `noexcept` 関数では例外を投げない
- デストラクタで例外を投げない
- 値が存在しない可能性には `std::optional<T>` を使用
- デバッグ用の不可能条件チェックには `assert()`
- 外部入力は使用前にバリデーション

## 並行処理

- 共有可変データは `std::mutex` / `std::shared_mutex` で保護
- ロックは RAII: `std::scoped_lock` を使用
- 不変の共有データを優先
- `thread_local` は控えめに使用

```cpp
// Good
std::mutex mutex_;
void UpdateState() {
    std::scoped_lock lock(mutex_);
    // ...
}
```

## コメント

### 原則

- 明確なコードはコメントより優先
- **"何を"ではなく"なぜ"** を書く
- 公開 API にはドキュメントコメントを付ける

### 避けるべきもの

- コードを言い換えるだけの冗長なコメント
- コメントアウトしたコード（バージョン管理を使う）
- 本番コードでの TODO（Issue トラッカーを使う）

## フォーマット

`.clang-format` による自動整形に従う。手動で気を付ける点:

- **`using namespace` 禁止**: 名前空間は完全修飾する
- **名前空間の閉じ括弧にコメント**: `}  // namespace gif_converter`
- **ネスト名前空間**: `namespace gif_converter::codec { }`

## C++20 機能の活用

| 機能                    | 用途                                          |
| ----------------------- | --------------------------------------------- |
| `std::format`           | 型安全な文字列フォーマット                    |
| Concepts                | テンプレート制約                              |
| Ranges                  | `std::views::filter` 等の宣言的イテレーション |
| Designated initializers | `Point{.x = 1, .y = 2}`                       |
| `constexpr` の拡充      | コンパイル時計算の活用                        |
| `std::span`             | 配列/バッファの安全な参照                     |

## 参考資料

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
