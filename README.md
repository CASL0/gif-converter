# gif-converter

**動画を GIF に変換する、小さくて速い HTTP サーバー。**

アップロードした動画ファイルを、シンプルな REST API で GIF アニメーションに変換します。
C++23 と FFmpeg を使い、非同期で処理。Distroless コンテナで 1 コマンド起動できます。

---

## 特徴

- **シンプルな REST API** — `POST /api/v1/conversions` にファイルを投げるだけ
- **非同期変換** — 大きな動画もブロックせず、ポーリングで進捗を確認
- **柔軟なオプション** — 幅とフレームレートを指定可能
- **コンテナファースト** — Distroless ベースの最小ランタイムイメージ
- **プロダクション品質** — C++23、構造化ロギング (quill)、RFC 9457 準拠のエラーレスポンス

---

## クイックスタート

### Docker で起動

```bash
docker build -t gif-converter .
docker run -p 8080:8080 gif-converter
```

### 動画を GIF に変換

```bash
# 変換リクエスト (202 Accepted が返る)
curl -X POST http://localhost:8080/api/v1/conversions \
  -F "file=@video.mp4" \
  -F "width=480" \
  -F "fps=15"

# レスポンス例
# {
#   "id": "550e8400-e29b-41d4-a716-446655440000",
#   "status": "pending",
#   "inputFileName": "video.mp4",
#   "options": { "width": 480, "fps": 15 },
#   "createdAt": "2026-04-18T03:30:00Z"
# }

# 進捗確認
curl http://localhost:8080/api/v1/conversions/550e8400-...

# 変換結果の GIF を取得
curl http://localhost:8080/api/v1/conversions/550e8400-.../result -o output.gif
```

---

## API エンドポイント

| メソッド | パス                              | 用途                      |
| -------- | --------------------------------- | ------------------------- |
| POST     | `/api/v1/conversions`             | 変換ジョブの作成          |
| GET      | `/api/v1/conversions`             | ジョブ一覧取得            |
| GET      | `/api/v1/conversions/{id}`        | ジョブの状態取得          |
| GET      | `/api/v1/conversions/{id}/result` | 変換後 GIF のダウンロード |
| DELETE   | `/api/v1/conversions/{id}`        | ジョブ削除                |
| GET      | `/health`                         | ヘルスチェック            |

### 変換オプション

| パラメータ | 型   | デフォルト | 説明                 |
| ---------- | ---- | ---------- | -------------------- |
| `file`     | file | (必須)     | 変換する動画ファイル |
| `width`    | int  | 320        | 出力 GIF の幅 (px)   |
| `fps`      | int  | 10         | 出力フレームレート   |

- 最大アップロードサイズ: **500MB**
- エラーレスポンスは [RFC 9457 (Problem Details)](https://www.rfc-editor.org/rfc/rfc9457) 形式

---

## ソースからビルド

### 必要なもの

- C++23 対応コンパイラ (GCC 13+ / Clang 16+)
- CMake 3.21+
- Ninja
- [vcpkg](https://github.com/microsoft/vcpkg) (環境変数 `VCPKG_ROOT` を設定)

### ビルド & 起動

```bash
cmake --preset default
cmake --build build/
./build/src/gif-converter
```

サーバーは `0.0.0.0:8080` で起動します。

### テスト

```bash
ctest --preset default
```

### カバレッジ計測

```bash
cmake --preset coverage
cmake --build build-coverage/
ctest --preset coverage
```

詳しい手順は [CLAUDE.md](CLAUDE.md) を参照してください。

---

## 技術スタック

- **[Drogon](https://github.com/drogonframework/drogon)** — 高速な C++ HTTP フレームワーク
- **[FFmpeg](https://ffmpeg.org/)** — 動画デコード / GIF エンコード
- **[quill](https://github.com/odygrd/quill)** — 低レイテンシ構造化ロギング
- **[GoogleTest](https://github.com/google/googletest)** — ユニットテスト
- **[vcpkg](https://github.com/microsoft/vcpkg)** — 依存管理

---

## ドキュメント

- [API 設計ガイド](docs/api-design.md)
- [C++ コーディング規約](docs/cpp-coding-standards.md)
- [セキュアコーディングガイド](docs/security.md)
- [Git ワークフロー](docs/git-workflow.md)

---

## ライセンス

[MIT License](LICENSE)
