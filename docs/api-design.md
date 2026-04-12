# API 設計ガイド

REST API 設計のベストプラクティスに基づく。
本プロジェクトの REST API を設計・実装する際の指針を定める。

## エンドポイント設計

### URI はリソースを表す名詞にする

URI はリソース（名詞）を表し、操作は HTTP メソッドで表現する。

```
# Good
GET    /api/v1/conversions          # 変換一覧取得
POST   /api/v1/conversions          # 新規変換作成
GET    /api/v1/conversions/{id}     # 特定の変換取得
DELETE /api/v1/conversions/{id}     # 変換削除

# Bad: 動詞を URI に含めない
POST   /api/v1/convertVideo
GET    /api/v1/getConversion
```

### 複数形の名詞を使う

リソース名は常に複数形にする。単一リソースの取得でも複数形 URI + ID で表現する。

```
# Good
GET /api/v1/conversions/{id}

# Bad
GET /api/v1/conversion/{id}
```

### ネストは浅く保つ

リソースのネストは 1 階層までとする。深いネストは URI が長くなり可読性が下がる。

```
# Good: 1階層のネスト
GET /api/v1/conversions/{id}/result

# Bad: 深いネスト
GET /api/v1/users/{uid}/conversions/{cid}/result/frames/{fid}
```

### URI にはケバブケースを使う

```
# Good
GET /api/v1/conversion-jobs

# Bad
GET /api/v1/conversionJobs
GET /api/v1/conversion_jobs
```

## HTTP メソッドの使い分け

| メソッド | 用途               | 冪等性 | 安全性 |
| -------- | ------------------ | ------ | ------ |
| GET      | リソース取得       | Yes    | Yes    |
| POST     | リソース作成       | No     | No     |
| PUT      | リソース全体の置換 | Yes    | No     |
| PATCH    | リソースの部分更新 | No     | No     |
| DELETE   | リソース削除       | Yes    | No     |

- GET, HEAD は副作用を持たせない
- PUT は冪等にする（同じリクエストを何度送っても同じ結果）
- POST はリソース作成に使い、作成されたリソースの URI を `Location` ヘッダで返す

## レスポンス設計

### JSON フォーマット

レスポンスボディのフォーマットは JSON とする。

- フィールド名はキャメルケース (camelCase) を使う
- 日時は ISO 8601 形式 (`2026-04-12T09:30:00Z`)
- null は省略せず明示的に返す

```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "status": "completed",
  "inputFileName": "video.mp4",
  "outputUrl": "/api/v1/conversions/550e8400/result",
  "createdAt": "2026-04-12T09:30:00Z",
  "completedAt": "2026-04-12T09:30:05Z",
  "options": {
    "width": 320,
    "fps": 10
  }
}
```

### エンベロープは使わない

レスポンスをエンベロープ (`{"status": "success", "data": {...}}`) で包まない。
HTTP ステータスコードが成功/失敗を示すため、冗長になる。

```json
// Bad: エンベロープ
{
  "status": "success",
  "data": {
    "id": "550e8400",
    "status": "completed"
  }
}

// Good: リソースを直接返す
{
  "id": "550e8400",
  "status": "completed"
}
```

### コレクション取得時のレスポンス構造

一覧取得ではオブジェクトの配列を直接返さず、メタデータを含むオブジェクトで返す。
これにより後方互換性を保ちながらメタデータの追加が可能になる。

```json
{
  "items": [
    { "id": "aaa", "status": "completed" },
    { "id": "bbb", "status": "processing" }
  ],
  "totalCount": 42,
  "limit": 20,
  "offset": 0
}
```

## エラーレスポンス

### 統一されたエラーフォーマット

全てのエラーレスポンスは同一の構造を使う。

```json
{
  "type": "validation_error",
  "title": "Invalid request parameters",
  "status": 400,
  "detail": "The 'fps' parameter must be between 1 and 30.",
  "instance": "/api/v1/conversions"
}
```

これは [RFC 9457 (Problem Details for HTTP APIs)](https://www.rfc-editor.org/rfc/rfc9457) に準拠した形式とする。

### Content-Type

エラーレスポンスの Content-Type は `application/problem+json` とする。

## HTTP ステータスコード

### 成功

| コード         | 用途                                      |
| -------------- | ----------------------------------------- |
| 200 OK         | 取得・更新成功                            |
| 201 Created    | リソース作成成功（`Location` ヘッダ必須） |
| 202 Accepted   | 非同期処理の受付完了                      |
| 204 No Content | 削除成功（ボディなし）                    |

### クライアントエラー

| コード                     | 用途                                     |
| -------------------------- | ---------------------------------------- |
| 400 Bad Request            | リクエスト不正（バリデーションエラー等） |
| 404 Not Found              | リソースが存在しない                     |
| 405 Method Not Allowed     | 許可されていない HTTP メソッド           |
| 406 Not Acceptable         | Accept ヘッダで対応不可のメディアタイプ  |
| 409 Conflict               | リソースの状態が競合                     |
| 413 Content Too Large      | アップロードサイズ超過                   |
| 415 Unsupported Media Type | サポートしない動画形式                   |
| 422 Unprocessable Content  | 構文は正しいがセマンティクスが不正       |
| 429 Too Many Requests      | レートリミット超過                       |

### サーバーエラー

| コード                    | 用途                 |
| ------------------------- | -------------------- |
| 500 Internal Server Error | サーバー内部エラー   |
| 503 Service Unavailable   | 一時的なサービス停止 |

- クライアントに内部実装の詳細（スタックトレース等）を漏洩しない
- 500 系エラーは汎用メッセージのみ返し、詳細はサーバーログに記録する

## バージョニング

### URI パスにバージョンを含める

```
/api/v1/conversions
/api/v2/conversions
```

- メジャーバージョンのみ URI に含める
- 後方互換性のある変更（フィールド追加等）ではバージョンを上げない
- 破壊的変更時にのみバージョンを上げる
- 旧バージョンは廃止予定を告知してから一定期間維持する

## ページネーション

### オフセットベース

シンプルな一覧取得に使用する。

```
GET /api/v1/conversions?offset=20&limit=20
```

- `limit` のデフォルト値とサーバー側の最大値を設定する（例: デフォルト 20、最大 100）
- クライアントが上限を超えた `limit` を指定した場合はサーバー側最大値に切り詰める

### レスポンスにページネーション情報を含める

```json
{
  "items": [...],
  "totalCount": 100,
  "limit": 20,
  "offset": 20
}
```

## フィルタリング・ソート

### クエリパラメータで指定する

```
GET /api/v1/conversions?status=completed&sort=-createdAt
```

- フィルタ: `?status=completed`
- ソート: `?sort=createdAt` (昇順)、`?sort=-createdAt` (降順)
- 許可されていないフィールドでのソート・フィルタは 400 を返す

## Content-Type とコンテントネゴシエーション

### リクエスト

- JSON ボディのリクエストは `Content-Type: application/json` を必須とする
- ファイルアップロードは `Content-Type: multipart/form-data` を使用する
- Content-Type が不正な場合は 415 を返す

### レスポンス

- 通常のレスポンスは `Content-Type: application/json` で返す
- GIF ファイルのダウンロードは `Content-Type: image/gif` で返す
- `Accept` ヘッダを尊重し、対応できない場合は 406 を返す

## レートリミット

### レスポンスヘッダで残量を通知する

```
X-RateLimit-Limit: 60
X-RateLimit-Remaining: 56
X-RateLimit-Reset: 1713000000
```

- `X-RateLimit-Reset` は UNIX タイムスタンプで返す
- 超過時は 429 を返し、`Retry-After` ヘッダを含める

## キャッシュ

### 適切なキャッシュヘッダを設定する

```
# 変換結果（不変のリソース）
Cache-Control: public, max-age=86400, immutable
ETag: "abc123"

# 変換状態（頻繁に変わるリソース）
Cache-Control: no-cache
ETag: "def456"

# 機密情報を含むレスポンス
Cache-Control: private, no-store
```

- 不変のリソース（完了済みの GIF）には長い `max-age` と `immutable` を設定する
- 状態が変化するリソースには `ETag` を使い、条件付きリクエスト (`If-None-Match`) をサポートする
- `304 Not Modified` を適切に返す

## CORS (Cross-Origin Resource Sharing)

### 必要最小限のオリジンを許可する

```
Access-Control-Allow-Origin: https://example.com
Access-Control-Allow-Methods: GET, POST, DELETE
Access-Control-Allow-Headers: Content-Type, Authorization
Access-Control-Max-Age: 86400
```

- ワイルドカード (`*`) は開発環境以外では使わない
- プリフライトリクエスト (OPTIONS) に対して `Access-Control-Max-Age` を設定しリクエスト数を削減する

## 非同期処理パターン

動画→GIF 変換は時間がかかるため、非同期パターンを採用する。

### リクエスト受付

```
POST /api/v1/conversions
Content-Type: multipart/form-data

=> 202 Accepted
Location: /api/v1/conversions/550e8400
```

```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "status": "pending",
  "createdAt": "2026-04-12T09:30:00Z"
}
```

### ポーリングによる状態確認

```
GET /api/v1/conversions/550e8400

=> 200 OK
```

```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "status": "processing",
  "progress": 65,
  "createdAt": "2026-04-12T09:30:00Z"
}
```

- `status` の遷移: `pending` → `processing` → `completed` / `failed`
- ポーリング間隔のヒントとして `Retry-After` ヘッダを含める

### 結果取得

```
GET /api/v1/conversions/550e8400/result

=> 200 OK
Content-Type: image/gif
```

## セキュリティ

### 認証・認可

- API キー認証の場合、キーは `Authorization` ヘッダで送信する
- URI にトークンやキーを含めない（アクセスログに残る）

```
# Good
Authorization: Bearer <token>

# Bad
GET /api/v1/conversions?api_key=SECRET
```

### セキュリティヘッダ

全てのレスポンスに以下のヘッダを含める。

```
X-Content-Type-Options: nosniff
X-Frame-Options: DENY
Strict-Transport-Security: max-age=31536000; includeSubDomains
```

### 入力バリデーション

- リクエストパラメータは厳密にバリデーションする
- 未知のフィールドは無視する（エラーにしない）
- バリデーションエラーは具体的なメッセージを返す

```json
{
  "type": "validation_error",
  "title": "Invalid request parameters",
  "status": 400,
  "detail": "Validation failed for 1 field(s).",
  "errors": [
    {
      "field": "options.fps",
      "message": "Must be between 1 and 30."
    }
  ]
}
```

## ドキュメンテーション

### OpenAPI (Swagger) で API 仕様を記述する

- API 仕様は OpenAPI 3.x 形式で管理する
- リクエスト・レスポンスの例を必ず含める
- 各エンドポイントにエラーケースを記載する

## 冪等性

### POST リクエストの冪等性を保証する

ネットワーク障害によるリトライに備え、`Idempotency-Key` ヘッダをサポートする。

```
POST /api/v1/conversions
Idempotency-Key: unique-client-generated-key
Content-Type: multipart/form-data
```

- 同じ `Idempotency-Key` で複数回リクエストされた場合、最初のレスポンスを返す
- キーの有効期限を設定する（例: 24 時間）

## ヘルスチェック

### 監視用エンドポイントを提供する

```
GET /health

=> 200 OK
```

```json
{
  "status": "healthy",
  "version": "1.0.0",
  "uptime": 3600
}
```

- ロードバランサーやオーケストレーターからのヘルスチェックに使用する
- 依存サービスの状態も含める場合は `/health/ready` と `/health/live` を分離する

## 参考資料

- [RFC 9457 - Problem Details for HTTP APIs](https://www.rfc-editor.org/rfc/rfc9457)
- [RFC 9110 - HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110)
- [OpenAPI Specification](https://spec.openapis.org/oas/latest.html)
- [Microsoft REST API Guidelines](https://github.com/microsoft/api-guidelines)
- [Google API Design Guide](https://cloud.google.com/apis/design)
