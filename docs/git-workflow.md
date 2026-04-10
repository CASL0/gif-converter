# Git ワークフロー

## ブランチ戦略: GitHub Flow

シンプルなブランチモデルを採用する。`main` ブランチは常にデプロイ可能な状態を維持する。

```
main (常にデプロイ可能)
 ├── feat/add-mp4-support
 ├── fix/upload-size-validation
 └── chore/update-dependencies
```

### ルール

- `main` への直接コミットは禁止
- 全ての変更は feature ブランチから Pull Request を経由する
- PR はレビュー承認後にマージする
- マージ後の feature ブランチは削除する

### ブランチの流れ

```
1. main から feature ブランチを作成
2. feature ブランチで作業・コミット
3. Push して Pull Request を作成
4. レビュー・CI 通過を確認
5. main にマージ（Squash Merge）
6. feature ブランチを削除
```

## ブランチ命名規則

```
<type>/<short-description>
```

| type       | 用途                 | 例                                |
| ---------- | -------------------- | --------------------------------- |
| `feat`     | 新機能               | `feat/gif-encoding`               |
| `fix`      | バグ修正             | `fix/memory-leak-on-large-upload` |
| `refactor` | リファクタリング     | `refactor/extract-codec-layer`    |
| `docs`     | ドキュメント         | `docs/api-specification`          |
| `chore`    | ビルド・CI・依存更新 | `chore/upgrade-ffmpeg`            |
| `test`     | テスト追加・修正     | `test/converter-edge-cases`       |

## コミットメッセージ: Conventional Commits

[Conventional Commits v1.0.0](https://www.conventionalcommits.org/) に準拠する。

### フォーマット

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

### type 一覧

| type       | 用途                                         | SemVer |
| ---------- | -------------------------------------------- | ------ |
| `feat`     | 新機能の追加                                 | MINOR  |
| `fix`      | バグ修正                                     | PATCH  |
| `docs`     | ドキュメントのみの変更                       | -      |
| `style`    | フォーマット変更（動作に影響しない）         | -      |
| `refactor` | リファクタリング（機能追加・バグ修正でない） | -      |
| `perf`     | パフォーマンス改善                           | -      |
| `test`     | テストの追加・修正                           | -      |
| `build`    | ビルドシステム・外部依存の変更               | -      |
| `ci`       | CI 設定の変更                                | -      |
| `chore`    | その他の雑務                                 | -      |

### scope（任意）

変更の影響範囲を示す。本プロジェクトでは以下を使用する。

| scope    | 対象                  |
| -------- | --------------------- |
| `codec`  | FFmpeg / 変換処理     |
| `api`    | HTTP エンドポイント   |
| `server` | Drogon サーバー設定   |
| `docker` | Dockerfile / コンテナ |
| `deps`   | 依存ライブラリ        |

### 例

```
feat(codec): add MP4 to GIF conversion

Implement video decoding with libavcodec and GIF encoding
with the built-in GIF muxer. Supports H.264 input.
```

```
fix(api): validate upload size before reading body

Reject requests exceeding 500MB at Content-Length check
to prevent OOM on large uploads.

Refs: #42
```

```
chore(deps): update ffmpeg to 7.1
```

```
docs: add security coding guidelines
```

### 破壊的変更

API の互換性を壊す変更には `BREAKING CHANGE` フッターまたは `!` を付ける。

```
feat(api)!: change convert endpoint response format

BREAKING CHANGE: /api/convert now returns JSON with download URL
instead of raw GIF binary.
```

## Pull Request

### タイトル

コミットメッセージと同じ Conventional Commits 形式を使用する。

```
feat(codec): add WebM input support
```

### テンプレート

```markdown
## Summary

- 変更内容の箇条書き

## Test plan

- [ ] テスト項目
```

### マージ方法

- **Squash Merge** を使用する（feature ブランチの全コミットを1つにまとめる）
- マージコミットのメッセージは PR タイトルに合わせる

## 日常の操作例

### 新機能の開発

```bash
# 1. main を最新化
git switch main
git pull origin main

# 2. feature ブランチ作成
git switch -c feat/gif-encoding

# 3. 作業・コミット
git add src/codec/gif_encoder.cpp
git commit -m "feat(codec): implement GIF encoder with FFmpeg"

# 4. Push して PR 作成
git push -u origin feat/gif-encoding
gh pr create --title "feat(codec): implement GIF encoder with FFmpeg"

# 5. マージ後にブランチ削除
git switch main
git pull origin main
git branch -d feat/gif-encoding
```

### バグ修正

```bash
git switch -c fix/null-check-on-upload
# ... 修正 ...
git commit -m "fix(api): add null check for uploaded file pointer"
git push -u origin fix/null-check-on-upload
gh pr create --title "fix(api): add null check for uploaded file pointer"
```

## 参考資料

- [GitHub Flow](https://docs.github.com/en/get-started/using-git/github-flow)
- [Conventional Commits v1.0.0](https://www.conventionalcommits.org/en/v1.0.0/)
