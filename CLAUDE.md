# gif-converter

動画ファイルをGIFに変換するC++サーバー。

## ビルド

```bash
# 前提: VCPKG_ROOT が設定済み
cmake --preset default
cmake --build build/
```

## テスト

```bash
ctest --preset default
```

## カバレッジ

```bash
cmake --preset coverage
cmake --build build-coverage/
ctest --preset coverage
lcov --capture --directory build-coverage/tests/ --output-file build-coverage/coverage.info --ignore-errors mismatch,negative,gcov
lcov --remove build-coverage/coverage.info '/usr/*' '*/vcpkg_installed/*' '*/tests/*' --output-file build-coverage/coverage-filtered.info --ignore-errors unused,negative
genhtml build-coverage/coverage-filtered.info --output-directory build-coverage/html --ignore-errors negative
```

HTML レポートは `build-coverage/html/index.html` に出力される。

## Docker

```bash
docker build -t gif-converter .
docker run -p 8080:8080 gif-converter
```

- ビルド: `debian:12-slim` (GCC 12)
- ランタイム: `gcr.io/distroless/cc-debian12:nonroot`
- GCC 12 の `-Wrestrict` 偽陽性（quill の JsonSink.h）を `-Wno-error=restrict` で抑制している

## 技術スタック

- C++23
- CMake 3.21+ / Ninja
- vcpkg (パッケージ管理)
- Drogon (HTTP サーバー)
- FFmpeg (動画デコード / GIF エンコード)
- Google Test (テスト)
- quill (構造化ロギング)

## 作業前に読むドキュメント

作業を開始する前に、以下のドキュメントを必ず確認すること。

- [docs/cpp-coding-standards.md](docs/cpp-coding-standards.md) — C++ コーディング規約（命名規則、クラス設計、メモリ管理等）
- [docs/security.md](docs/security.md) — セキュアコーディングガイド（CERT C++、OWASP、ファイルアップロード）
- [docs/git-workflow.md](docs/git-workflow.md) — Git ワークフロー（GitHub Flow、Conventional Commits）
- [docs/api-design.md](docs/api-design.md) — API 設計ガイド（エンドポイント設計、レスポンス、エラー処理等）

## コーディング規約

- `.clang-format` に従う (Google ベース, インデント4)
- 警告は `-Wall -Wextra -Wpedantic -Werror` で全てエラー扱い
- テストは `tests/` に配置、`*_test.cpp` の命名規則
