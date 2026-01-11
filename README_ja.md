# libpino

[![CI](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/zeriyoshi/libpino)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**Packageing Notation - PINO**

[🇬🇧 English README](README.md)

libpino は、C99 で書かれた軽量・高性能なシリアライゼーションライブラリです。ハンドラーベースの柔軟なアーキテクチャにより、構造化データのパックとアンパックを実現し、自動的なエンディアン処理と SIMD 高速化をサポートします。

## 特徴

- **純粋な C99 実装** - 外部依存なし、プラットフォーム間で移植可能
- **ハンドラーベースアーキテクチャ** - カスタム型ハンドラーによる拡張可能な設計
- **自動エンディアン処理** - ビッグエンディアン、リトルエンディアン、ネイティブバイトオーダー間のシームレスな変換
- **SIMD 高速化** - AVX2 (x86_64)、NEON (ARM64)、WASM SIMD128 による自動最適化
- **メモリ安全** - 適切なリソース管理とカスタムメモリマネージャーを備えた慎重に設計された API
- **WebAssembly 対応** - Emscripten を使用して WASM にコンパイル可能
- **包括的なテスト** - サニタイザと Valgrind サポートを含む広範なテストスイート

## クイックスタート

### ビルド

```bash
# リポジトリをクローン
git clone https://github.com/zeriyoshi/libpino.git
cd libpino

# CMake でビルド
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### ビルドオプション

| オプション | デフォルト | 説明 |
|--------|---------|-------------|
| `PINO_USE_SIMD` | `ON` | SIMD 最適化を有効化 |
| `PINO_USE_TESTS` | `OFF` | テストスイートをビルド |
| `PINO_USE_VALGRIND` | `OFF` | Valgrind メモリチェックを有効化 |
| `PINO_USE_COVERAGE` | `OFF` | コードカバレッジを有効化 |
| `PINO_USE_ASAN` | `OFF` | AddressSanitizer を有効化 |
| `PINO_USE_MSAN` | `OFF` | MemorySanitizer を有効化 |
| `PINO_USE_UBSAN` | `OFF` | UndefinedBehaviorSanitizer を有効化 |

### テストの実行

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DPINO_USE_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## 使用例

```c
#include <pino.h>
#include <pino/handler.h>
#include <stdio.h>
#include <string.h>

// データ構造用のカスタムハンドラーを定義
PH_BEGIN(mydata);

PH_DEF_STATIC_FIELDS_STRUCT(mydata) {
    uint32_t data_size;
}
PH_DEF_STATIC_FIELDS_STRUCT_END;

PH_DEF_STRUCT(mydata) {
    uint8_t *buffer;
}
PH_DEF_STRUCT_END;

PH_DEFUN_SERIALIZE_SIZE(mydata) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydata, data_size, &size);
    return (size_t)size;
}

PH_DEFUN_SERIALIZE(mydata) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydata, data_size, &size);
    PH_SERIALIZE_DATA(mydata, buffer, (size_t)size);
    return true;
}

PH_DEFUN_UNSERIALIZE(mydata) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydata, data_size, &size);
    PH_UNSERIALIZE_DATA(mydata, buffer, (size_t)size);
    return true;
}

PH_DEFUN_PACK(mydata) {
    uint32_t size = (uint32_t)PH_ARG_SIZE;
    PH_THIS_STATIC_SET(mydata, data_size, &size);
    PH_PACK_DATA(mydata, buffer, PH_ARG_SIZE);
    return true;
}

PH_DEFUN_UNPACK_SIZE(mydata) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydata, data_size, &size);
    return (size_t)size;
}

PH_DEFUN_UNPACK(mydata) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydata, data_size, &size);
    PH_UNPACK_DATA(mydata, buffer, (size_t)size);
    return true;
}

PH_DEFUN_CREATE(mydata) {
    PH_CREATE_THIS(mydata);
    
    PH_THIS(mydata)->buffer = (uint8_t *)PH_CALLOC(mydata, 1, PH_ARG_SIZE);
    if (!PH_THIS(mydata)->buffer) {
        PH_DESTROY_THIS(mydata);
        return NULL;
    }
    
    uint32_t size = (uint32_t)PH_ARG_SIZE;
    PH_THIS_STATIC_SET(mydata, data_size, &size);
    
    return PH_THIS(mydata);
}

PH_DEFUN_DESTROY(mydata) {
    if (PH_THIS(mydata)->buffer) {
        PH_FREE(mydata, PH_THIS(mydata)->buffer);
    }
    PH_DESTROY_THIS(mydata);
}

PH_END(mydata);

int main(void) {
    // PINO を初期化
    if (!pino_init()) {
        fprintf(stderr, "PINO の初期化に失敗しました\n");
        return 1;
    }
    
    // ハンドラーを登録
    if (!PH_REG(mydata)) {
        fprintf(stderr, "ハンドラーの登録に失敗しました\n");
        pino_free();
        return 1;
    }
    
    // データを準備
    const char *message = "Hello, PINO!";
    size_t message_len = strlen(message) + 1;
    
    // データをパック
    pino_t *pino = pino_pack("mydata", message, message_len);
    if (!pino) {
        fprintf(stderr, "データのパックに失敗しました\n");
        PH_UNREG(mydata);
        pino_free();
        return 1;
    }
    
    // シリアライズ
    size_t serialized_size = pino_serialize_size(pino);
    uint8_t *serialized = (uint8_t *)malloc(serialized_size);
    if (!pino_serialize(pino, serialized)) {
        fprintf(stderr, "シリアライズに失敗しました\n");
        free(serialized);
        pino_destroy(pino);
        PH_UNREG(mydata);
        pino_free();
        return 1;
    }
    
    pino_destroy(pino);
    
    // デシリアライズ
    pino_t *restored = pino_unserialize(serialized, serialized_size);
    free(serialized);
    
    if (!restored) {
        fprintf(stderr, "デシリアライズに失敗しました\n");
        PH_UNREG(mydata);
        pino_free();
        return 1;
    }
    
    // アンパック
    size_t unpacked_size = pino_unpack_size(restored);
    char *unpacked = (char *)malloc(unpacked_size);
    if (!pino_unpack(restored, unpacked)) {
        fprintf(stderr, "アンパックに失敗しました\n");
        free(unpacked);
        pino_destroy(restored);
        PH_UNREG(mydata);
        pino_free();
        return 1;
    }
    
    printf("アンパックされたメッセージ: %s\n", unpacked);
    
    // クリーンアップ
    free(unpacked);
    pino_destroy(restored);
    PH_UNREG(mydata);
    pino_free();
    
    return 0;
}
```

## API リファレンス

### コア型

```c
typedef struct _pino_t pino_t;              // メイン PINO オブジェクト
typedef struct _pino_handler_t pino_handler_t;  // ハンドラー定義
typedef char pino_magic_t[4];               // 4 バイトのマジック識別子
typedef char pino_magic_safe_t[5];          // NULL 終端マジック
typedef uint64_t pino_static_fields_size_t; // 静的フィールドサイズ型
typedef uint32_t pino_buildtime_t;          // ビルドタイムスタンプ型
```

### メイン関数

#### `pino_init`

```c
bool pino_init(void);
```

PINO ライブラリを初期化します。他の PINO 関数を呼び出す前に呼び出す必要があります。

**戻り値:** 成功時 `true`、失敗時 `false`。

#### `pino_free`

```c
void pino_free(void);
```

PINO ライブラリによって割り当てられたすべてのリソースを解放します。PINO の使用終了時に呼び出す必要があります。

#### `pino_pack`

```c
pino_t *pino_pack(pino_magic_safe_t magic, const void *src, size_t size);
```

指定されたハンドラーを使用して生データを PINO オブジェクトにパックします。

**パラメータ:**
- `magic` - 4 文字のハンドラー識別子（NULL 終端）
- `src` - ソースデータへのポインタ
- `size` - ソースデータのサイズ（バイト単位）

**戻り値:** 新しい PINO オブジェクト、失敗時は `NULL`。

#### `pino_unpack`

```c
bool pino_unpack(const pino_t *pino, void *dest);
```

PINO オブジェクトからバッファにデータをアンパックします。

**パラメータ:**
- `pino` - アンパックする PINO オブジェクト
- `dest` - 宛先バッファ（少なくとも `pino_unpack_size()` バイト必要）

**戻り値:** 成功時 `true`、失敗時 `false`。

#### `pino_unpack_size`

```c
size_t pino_unpack_size(const pino_t *pino);
```

アンパックされたデータのサイズを取得します。

**パラメータ:**
- `pino` - PINO オブジェクト

**戻り値:** サイズ（バイト単位）、エラー時は 0。

#### `pino_serialize`

```c
bool pino_serialize(const pino_t *pino, void *dest);
```

PINO オブジェクトを保存または転送用のバイトバッファにシリアライズします。

**パラメータ:**
- `pino` - シリアライズする PINO オブジェクト
- `dest` - 宛先バッファ（少なくとも `pino_serialize_size()` バイト必要）

**戻り値:** 成功時 `true`、失敗時 `false`。

#### `pino_serialize_size`

```c
size_t pino_serialize_size(const pino_t *pino);
```

シリアライズに必要なサイズを取得します。

**パラメータ:**
- `pino` - PINO オブジェクト

**戻り値:** サイズ（バイト単位）、エラー時は 0。

#### `pino_unserialize`

```c
pino_t *pino_unserialize(const void *src, size_t size);
```

バイトバッファを PINO オブジェクトにデシリアライズします。

**パラメータ:**
- `src` - シリアライズされたデータバッファ
- `size` - シリアライズされたデータのサイズ

**戻り値:** 新しい PINO オブジェクト、失敗時は `NULL`。

#### `pino_destroy`

```c
void pino_destroy(pino_t *pino);
```

PINO オブジェクトを破棄し、関連するすべてのリソースを解放します。

**パラメータ:**
- `pino` - 破棄する PINO オブジェクト

### ハンドラー API

#### `pino_handler_register`

```c
bool pino_handler_register(pino_magic_safe_t magic, pino_handler_t *handler);
```

マジック識別子を使用してカスタムハンドラーを登録します。

**パラメータ:**
- `magic` - 4 文字の識別子（NULL 終端）
- `handler` - ハンドラー構造体へのポインタ

**戻り値:** 成功時 `true`、失敗時 `false`。

#### `pino_handler_unregister`

```c
bool pino_handler_unregister(pino_magic_safe_t magic);
```

マジック識別子によってハンドラーの登録を解除します。

**パラメータ:**
- `magic` - ハンドラー識別子

**戻り値:** 成功時 `true`、失敗時 `false`。

### エンディアン API

```c
// エンディアン変換付きコピー
void *pino_endianness_memcpy_le2native(void *dest, const void *src, size_t size);
void *pino_endianness_memcpy_be2native(void *dest, const void *src, size_t size);
void *pino_endianness_memcpy_native2le(void *dest, const void *src, size_t size);
void *pino_endianness_memcpy_native2be(void *dest, const void *src, size_t size);

// エンディアン変換付き移動
void *pino_endianness_memmove_le2native(void *dest, const void *src, size_t size);
void *pino_endianness_memmove_be2native(void *dest, const void *src, size_t size);
void *pino_endianness_memmove_native2le(void *dest, const void *src, size_t size);
void *pino_endianness_memmove_native2be(void *dest, const void *src, size_t size);

// エンディアン変換付き比較
int pino_endianness_memcmp_le2native(const void *s1, const void *s2, size_t size);
int pino_endianness_memcmp_be2native(const void *s1, const void *s2, size_t size);
int pino_endianness_memcmp_native2le(const void *s1, const void *s2, size_t size);
int pino_endianness_memcmp_native2be(const void *s1, const void *s2, size_t size);
```

### ユーティリティ関数

```c
// ライブラリバージョンを整数で取得
uint32_t pino_version_id(void);

// ライブラリのビルドタイムスタンプを取得
pino_buildtime_t pino_buildtime(void);
```

### ハンドラーマクロ

PINO は、カスタムハンドラーを定義するための包括的なマクロセットを提供します：

#### 構造体定義

```c
PH_BEGIN(name)                          // ハンドラー定義の開始
PH_DEF_STRUCT(name) { ... }             // ハンドラーインスタンス構造体を定義
PH_DEF_STRUCT_END
PH_DEF_STATIC_FIELDS_STRUCT(name) { ... } // 静的フィールド構造体を定義
PH_DEF_STATIC_FIELDS_STRUCT_END
PH_END(name)                            // ハンドラー定義の終了
```

#### 関数定義

```c
PH_DEFUN_SERIALIZE_SIZE(name)           // シリアライズサイズ関数を定義
PH_DEFUN_SERIALIZE(name)                // シリアライズ関数を定義
PH_DEFUN_UNSERIALIZE(name)              // デシリアライズ関数を定義
PH_DEFUN_PACK(name)                     // パック関数を定義
PH_DEFUN_UNPACK_SIZE(name)              // アンパックサイズ関数を定義
PH_DEFUN_UNPACK(name)                   // アンパック関数を定義
PH_DEFUN_CREATE(name)                   // 作成関数を定義
PH_DEFUN_DESTROY(name)                  // 破棄関数を定義
```

#### データアクセス

```c
PH_THIS(name)                           // インスタンス構造体にアクセス
PH_THIS_STATIC(name)                    // 静的フィールド構造体にアクセス
PH_THIS_STATIC_GET(name, param, dest)   // 静的フィールド値を取得
PH_THIS_STATIC_SET(name, param, src)    // 静的フィールド値を設定
```

#### メモリ管理

```c
PH_CREATE_THIS(name)                    // インスタンス構造体を割り当て
PH_DESTROY_THIS(name)                   // インスタンス構造体を解放
PH_MALLOC(name, size)                   // メモリを割り当て
PH_CALLOC(name, count, size)            // ゼロ初期化メモリを割り当て
PH_FREE(name, ptr)                      // メモリを解放
```

#### データ操作

```c
PH_SERIALIZE_DATA(name, src, size)      // データフィールドをシリアライズ
PH_UNSERIALIZE_DATA(name, dest, size)   // データフィールドをデシリアライズ
PH_PACK_DATA(name, param, size)         // フィールドにデータをパック
PH_UNPACK_DATA(name, param, size)       // フィールドからデータをアンパック
```

#### 登録

```c
PH_REG(name)                            // ハンドラーを登録
PH_UNREG(name)                          // ハンドラーの登録を解除
```

#### エンディアン変換

```c
PH_MEMCPY_N2L(dst, src, size)           // ネイティブからリトルエンディアン
PH_MEMCPY_N2B(dst, src, size)           // ネイティブからビッグエンディアン
PH_MEMCPY_L2N(dst, src, size)           // リトルエンディアンからネイティブ
PH_MEMCPY_B2N(dst, src, size)           // ビッグエンディアンからネイティブ
```

## プラットフォームサポート

| プラットフォーム | SIMD | 状態 |
|----------|------|--------|
| Linux x86_64 | AVX2 | ✅ 完全サポート |
| Linux ARM64 | NEON | ✅ 完全サポート |
| Linux i386 | なし | ✅ サポート（スカラー） |
| Linux s390x | なし | ✅ サポート（スカラー） |
| macOS x86_64 | AVX2 | ✅ 完全サポート |
| macOS ARM64 | NEON | ✅ 完全サポート |
| Windows x86_64 | AVX2 | ✅ 完全サポート |
| WebAssembly | SIMD128 | ✅ 完全サポート |

## プロジェクト構成

```
libpino/
├── include/
│   ├── pino.h               # メイン公開ヘッダー
│   └── pino/
│       ├── endianness.h     # エンディアン変換 API
│       └── handler.h        # ハンドラー定義 API
├── src/
│   ├── pino.c               # メイン API 実装
│   ├── handler.c            # ハンドラーレジストリ
│   ├── memory.c             # メモリマネージャー
│   ├── endianness.c         # エンディアン変換
│   └── internal/
│       ├── common.h         # 内部型とマクロ
│       └── simd.h           # SIMD 抽象化
├── tests/                   # Unity を使用したテストスイート
│   ├── test_basic.c         # 基本機能テスト
│   ├── test_endianness.c    # エンディアンテスト
│   ├── test_invalid.c       # エラーハンドリングテスト
│   ├── handler_spl1.h       # サンプルハンドラー実装
│   └── util.h               # テストユーティリティ
├── cmake/                   # CMake モジュール
│   ├── buildtime.cmake      # ビルドタイムスタンプ生成
│   ├── emscripten.cmake     # WebAssembly 設定
│   └── test.cmake           # テスト設定
└── third_party/             # 依存関係（Unity、Emscripten SDK）
```

## ユースケース

PINO は以下のようなシナリオに最適です：

- **クロスプラットフォームバイナリデータ交換** - 自動エンディアン処理によりデータの移植性を確保
- **カスタムシリアライゼーション形式** - ハンドラーベースアーキテクチャによりドメイン固有の形式を定義可能
- **高性能データパッキング** - メモリ操作の SIMD 高速化
- **組み込みシステム** - 外部依存のない純粋な C99
- **WebAssembly アプリケーション** - SIMD 最適化を伴う完全な WASM サポート

## ライセンス

MIT License - 詳細は [LICENSE](LICENSE) を参照してください。

## 作者

**Go Kudo** ([@zeriyoshi](https://github.com/zeriyoshi)) - [zeriyoshi@gmail.com](mailto:zeriyoshi@gmail.com)
