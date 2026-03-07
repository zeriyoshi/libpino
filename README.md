# libpino

[![CI](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/zeriyoshi/libpino)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**Packageing Notation - PINO**

[🇯🇵 日本語版 README はこちら](README_ja.md)

libpino is a serialization library written in C99. It provides a handler-based architecture for packing and unpacking structured data, with helpers for byte-order conversion and optional SIMD-backed memory operations.

## Features

- **Pure C99 Implementation** - No external runtime dependencies
- **Handler-Based Architecture** - Extensible design with custom type handlers
- **Byte-Order Helpers** - Utilities for converting between little-endian, big-endian, and native byte order
- **Optional SIMD Paths** - AVX2 (amd64), NEON (arm64), and WASM SIMD128 implementations are available where supported
- **Custom Memory Managers** - Handler-local allocation is routed through the library memory manager
- **WebAssembly Support** - Can be compiled to WASM using Emscripten
- **Test Coverage** - Includes unit tests, sanitizer builds, and Valgrind support

## Quick Start

### Building

```bash
# Clone the repository
git clone https://github.com/zeriyoshi/libpino.git
cd libpino

# Build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `PINO_USE_SIMD` | `ON` | Enable SIMD optimizations |
| `PINO_USE_TESTS` | `OFF` | Build test suite |
| `PINO_USE_VALGRIND` | `OFF` | Enable Valgrind memory checking |
| `PINO_USE_COVERAGE` | `OFF` | Enable code coverage |
| `PINO_USE_ASAN` | `OFF` | Enable AddressSanitizer |
| `PINO_USE_MSAN` | `OFF` | Enable MemorySanitizer |
| `PINO_USE_UBSAN` | `OFF` | Enable UndefinedBehaviorSanitizer |

### Running Tests

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DPINO_USE_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Usage Example

```c
#include <pino.h>
#include <pino/handler.h>
#include <stdio.h>
#include <string.h>

// Define a custom handler for your data structure
PH_BEGIN(mydt);

PH_DEF_STATIC_FIELDS_STRUCT(mydt) {
    uint32_t data_size;
}
PH_DEF_STATIC_FIELDS_STRUCT_END;

PH_DEF_STRUCT(mydt) {
    uint8_t *buffer;
}
PH_DEF_STRUCT_END;

PH_DEFUN_SERIALIZE_SIZE(mydt) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydt, data_size, &size);
    return (size_t)size;
}

PH_DEFUN_SERIALIZE(mydt) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydt, data_size, &size);
    PH_SERIALIZE_DATA(mydt, buffer, (size_t)size);
    return true;
}

PH_DEFUN_UNSERIALIZE(mydt) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydt, data_size, &size);
    PH_UNSERIALIZE_DATA(mydt, buffer, (size_t)size);
    return true;
}

PH_DEFUN_PACK(mydt) {
    uint32_t size = (uint32_t)PH_ARG_SIZE;
    PH_THIS_STATIC_SET(mydt, data_size, &size);
    PH_PACK_DATA(mydt, buffer, PH_ARG_SIZE);
    return true;
}

PH_DEFUN_UNPACK_SIZE(mydt) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydt, data_size, &size);
    return (size_t)size;
}

PH_DEFUN_UNPACK(mydt) {
    uint32_t size;
    PH_THIS_STATIC_GET(mydt, data_size, &size);
    PH_UNPACK_DATA(mydt, buffer, (size_t)size);
    return true;
}

PH_DEFUN_CREATE(mydt) {
    PH_CREATE_THIS(mydt);
    
    PH_THIS(mydt)->buffer = (uint8_t *)PH_CALLOC(mydt, 1, PH_ARG_SIZE);
    if (!PH_THIS(mydt)->buffer) {
        PH_DESTROY_THIS(mydt);
        return NULL;
    }
    
    uint32_t size = (uint32_t)PH_ARG_SIZE;
    PH_THIS_STATIC_SET(mydt, data_size, &size);
    
    return PH_THIS(mydt);
}

PH_DEFUN_DESTROY(mydt) {
    if (PH_THIS(mydt)->buffer) {
        PH_FREE(mydt, PH_THIS(mydt)->buffer);
    }
    PH_DESTROY_THIS(mydt);
}

PH_END(mydt);

int main(void) {
    // Initialize PINO
    if (!pino_init()) {
        fprintf(stderr, "Failed to initialize PINO\n");
        return 1;
    }
    
    // Register handler
    if (!PH_REG(mydt)) {
        fprintf(stderr, "Failed to register handler\n");
        pino_free();
        return 1;
    }
    
    // Prepare data
    const char *message = "Hello, PINO!";
    size_t message_len = strlen(message) + 1;
    
    // Pack data
    pino_t *pino = pino_pack("mydt", message, message_len);
    if (!pino) {
        fprintf(stderr, "Failed to pack data\n");
        PH_UNREG(mydt);
        pino_free();
        return 1;
    }
    
    // Serialize
    size_t serialized_size = pino_serialize_size(pino);
    uint8_t *serialized = (uint8_t *)malloc(serialized_size);
    if (!pino_serialize(pino, serialized)) {
        fprintf(stderr, "Failed to serialize\n");
        free(serialized);
        pino_destroy(pino);
        PH_UNREG(mydt);
        pino_free();
        return 1;
    }
    
    pino_destroy(pino);
    
    // Unserialize
    pino_t *restored = pino_unserialize(serialized, serialized_size);
    free(serialized);
    
    if (!restored) {
        fprintf(stderr, "Failed to unserialize\n");
        PH_UNREG(mydt);
        pino_free();
        return 1;
    }
    
    // Unpack
    size_t unpacked_size = pino_unpack_size(restored);
    char *unpacked = (char *)malloc(unpacked_size);
    if (!pino_unpack(restored, unpacked)) {
        fprintf(stderr, "Failed to unpack\n");
        free(unpacked);
        pino_destroy(restored);
        PH_UNREG(mydt);
        pino_free();
        return 1;
    }
    
    printf("Unpacked message: %s\n", unpacked);
    
    // Cleanup
    free(unpacked);
    pino_destroy(restored);
    PH_UNREG(mydt);
    pino_free();
    
    return 0;
}
```

## API Reference

### Core Types

```c
typedef struct _pino_t pino_t;              // Main PINO object
typedef struct _pino_handler_t pino_handler_t;  // Handler definition
typedef char pino_magic_t[4];               // 4-byte magic identifier
typedef char pino_magic_safe_t[5];          // Null-terminated magic
typedef uint64_t pino_static_fields_size_t; // Static fields size type
typedef uint32_t pino_buildtime_t;          // Build timestamp type
```

### Main Functions

#### `pino_init`

```c
bool pino_init(void);
```

Initializes the PINO library. Must be called before any other PINO functions.

**Returns:** `true` on success, `false` on failure.

#### `pino_free`

```c
void pino_free(void);
```

Releases global handler-registration state owned by the PINO library. Existing PINO objects remain valid until `pino_destroy()` is called for each object.

#### `pino_pack`

```c
pino_t *pino_pack(pino_magic_safe_t magic, const void *src, size_t size);
```

Packs raw data into a PINO object using the specified handler.

**Parameters:**
- `magic` - 4-character handler identifier (null-terminated)
- `src` - Pointer to source data
- `size` - Size of source data in bytes

**Returns:** New PINO object, or `NULL` on failure.

#### `pino_unpack`

```c
bool pino_unpack(const pino_t *pino, void *dest);
```

Unpacks data from a PINO object into a buffer.

**Parameters:**
- `pino` - PINO object to unpack
- `dest` - Destination buffer (must be at least `pino_unpack_size()` bytes)

**Returns:** `true` on success, `false` on failure.

#### `pino_unpack_size`

```c
size_t pino_unpack_size(const pino_t *pino);
```

Gets the size of unpacked data.

**Parameters:**
- `pino` - PINO object

**Returns:** Size in bytes, or 0 on error.

#### `pino_serialize`

```c
bool pino_serialize(const pino_t *pino, void *dest);
```

Serializes a PINO object to a byte buffer for storage or transmission.

**Parameters:**
- `pino` - PINO object to serialize
- `dest` - Destination buffer (must be at least `pino_serialize_size()` bytes)

**Returns:** `true` on success, `false` on failure.

#### `pino_serialize_size`

```c
size_t pino_serialize_size(const pino_t *pino);
```

Gets the size needed for serialization.

**Parameters:**
- `pino` - PINO object

**Returns:** Size in bytes, or 0 on error.

#### `pino_unserialize`

```c
pino_t *pino_unserialize(const void *src, size_t size);
```

Unserializes a byte buffer back into a PINO object.

**Parameters:**
- `src` - Serialized data buffer
- `size` - Size of serialized data

**Returns:** New PINO object, or `NULL` on failure.

#### `pino_destroy`

```c
void pino_destroy(pino_t *pino);
```

Destroys a PINO object and frees all associated resources.

**Parameters:**
- `pino` - PINO object to destroy

Lifecycle note: `pino_free()` only releases global registration state. Every `pino_t` still has to be released with `pino_destroy()`.

Representation note: `pino_pack()` and `pino_unpack()` operate on native in-memory layout. `pino_serialize()` and `pino_unserialize()` are the wire-format boundary and are responsible for little-endian conversion.

### Handler API

#### `pino_handler_register`

```c
bool pino_handler_register(pino_magic_safe_t magic, pino_handler_t *handler);
```

Registers a custom handler with a magic identifier.

**Parameters:**
- `magic` - 4-character identifier (null-terminated)
- `handler` - Pointer to handler structure

**Returns:** `true` on success, `false` on failure.

#### `pino_handler_unregister`

```c
bool pino_handler_unregister(pino_magic_safe_t magic);
```

Unregisters a handler by magic identifier.

**Parameters:**
- `magic` - Handler identifier

**Returns:** `true` on success, `false` on failure.

### Endianness API

```c
// Copy with endianness conversion
void *pino_endianness_memcpy_le2native(void *dest, const void *src, size_t size, size_t elem_size);
void *pino_endianness_memcpy_be2native(void *dest, const void *src, size_t size, size_t elem_size);
void *pino_endianness_memcpy_native2le(void *dest, const void *src, size_t size, size_t elem_size);
void *pino_endianness_memcpy_native2be(void *dest, const void *src, size_t size, size_t elem_size);

// Move with endianness conversion
void *pino_endianness_memmove_le2native(void *dest, const void *src, size_t size, size_t elem_size);
void *pino_endianness_memmove_be2native(void *dest, const void *src, size_t size, size_t elem_size);
void *pino_endianness_memmove_native2le(void *dest, const void *src, size_t size, size_t elem_size);
void *pino_endianness_memmove_native2be(void *dest, const void *src, size_t size, size_t elem_size);

// Compare with endianness conversion
int pino_endianness_memcmp_le2native(const void *s1, const void *s2, size_t size, size_t elem_size);
int pino_endianness_memcmp_be2native(const void *s1, const void *s2, size_t size, size_t elem_size);
int pino_endianness_memcmp_native2le(const void *s1, const void *s2, size_t size, size_t elem_size);
int pino_endianness_memcmp_native2be(const void *s1, const void *s2, size_t size, size_t elem_size);
```

`elem_size` is the size of each converted element. For scalars, pass `sizeof(value_type)`. For arrays, pass `sizeof(array[0])`.

### Utility Functions

```c
// Get library version as integer
uint32_t pino_version_id(void);

// Get library build timestamp
pino_buildtime_t pino_buildtime(void);
```

### Handler Macros

PINO provides a comprehensive set of macros for defining custom handlers:

#### Structure Definition

```c
PH_BEGIN(name)                          // Begin handler definition
PH_DEF_STRUCT(name) { ... }             // Define handler instance structure
PH_DEF_STRUCT_END
PH_DEF_STATIC_FIELDS_STRUCT(name) { ... } // Define static fields structure
PH_DEF_STATIC_FIELDS_STRUCT_END
PH_END(name)                            // End handler definition
```

#### Function Definition

```c
PH_DEFUN_SERIALIZE_SIZE(name)           // Define serialize size function
PH_DEFUN_SERIALIZE(name)                // Define serialize function
PH_DEFUN_UNSERIALIZE(name)              // Define unserialize function
PH_DEFUN_PACK(name)                     // Define pack function
PH_DEFUN_UNPACK_SIZE(name)              // Define unpack size function
PH_DEFUN_UNPACK(name)                   // Define unpack function
PH_DEFUN_CREATE(name)                   // Define create function
PH_DEFUN_DESTROY(name)                  // Define destroy function
```

#### Data Access

```c
PH_THIS(name)                           // Access instance structure
PH_THIS_STATIC(name)                    // Access static fields structure
PH_THIS_STATIC_GET(name, param, dest)   // Get static field value
PH_THIS_STATIC_SET(name, param, src)    // Set static field value
```

#### Memory Management

```c
PH_CREATE_THIS(name)                    // Allocate instance structure
PH_DESTROY_THIS(name)                   // Free instance structure
PH_MALLOC(name, size)                   // Allocate memory
PH_CALLOC(name, count, size)            // Allocate zero-initialized memory
PH_FREE(name, ptr)                      // Free memory
```

#### Data Operations

```c
PH_SERIALIZE_DATA(name, src, size)      // Serialize data field
PH_UNSERIALIZE_DATA(name, dest, size)   // Unserialize data field
PH_PACK_DATA(name, param, size)         // Pack data into field
PH_UNPACK_DATA(name, param, size)       // Unpack data from field
```

#### Registration

```c
PH_REG(name)                            // Register handler
PH_UNREG(name)                          // Unregister handler
```

#### Endianness Conversion

```c
PH_MEMCPY_N2L(dst, src, size)           // Native to little-endian
PH_MEMCPY_N2B(dst, src, size)           // Native to big-endian
PH_MEMCPY_L2N(dst, src, size)           // Little-endian to native
PH_MEMCPY_B2N(dst, src, size)           // Big-endian to native
```

## Platform Support

| Platform | SIMD | Status |
|----------|------|--------|
| Linux x86_64 | AVX2 | ✅ Fully supported |
| Linux ARM64 | NEON | ✅ Fully supported |
| Linux i386 | None | ✅ Supported (scalar) |
| Linux s390x | None | ✅ Supported (scalar) |
| macOS x86_64 | AVX2 | ✅ Fully supported |
| macOS ARM64 | NEON | ✅ Fully supported |
| Windows x86_64 | AVX2 | ✅ Fully supported |
| WebAssembly | SIMD128 | ✅ Fully supported |

## Project Structure

```
libpino/
├── include/
│   ├── pino.h               # Main public header
│   └── pino/
│       ├── endianness.h     # Endianness conversion API
│       └── handler.h        # Handler definition API
├── src/
│   ├── pino.c               # Main API implementation
│   ├── handler.c            # Handler registry
│   ├── memory.c             # Memory manager
│   ├── endianness.c         # Endianness conversion
│   └── internal/
│       ├── common.h         # Internal types and macros
│       └── simd.h           # SIMD abstractions
├── tests/                   # Test suite using Unity
│   ├── test_basic.c         # Basic functionality tests
│   ├── test_endianness.c    # Endianness tests
│   ├── test_invalid.c       # Error handling tests
│   ├── handler_spl1.h       # Sample handler implementation
│   └── util.h               # Test utilities
├── cmake/                   # CMake modules
│   ├── buildtime.cmake      # Build timestamp generation
│   ├── emscripten.cmake     # WebAssembly configuration
│   └── test.cmake           # Test configuration
└── third_party/             # Dependencies (Unity, Emscripten SDK)
```

## Use Cases

libpino is intended for scenarios where you need:

- **Cross-platform binary data exchange** - A fixed wire format can be combined with the byte-order helpers
- **Custom serialization formats** - Handler-based architecture allows defining domain-specific formats
- **Memory-copy-heavy workloads** - SIMD-backed paths are available for supported targets
- **Embedded systems** - Pure C99 with no external runtime dependencies
- **WebAssembly applications** - Emscripten builds are supported

## License

MIT License - see [LICENSE](LICENSE) for details.

## Author

**Go Kudo** ([@zeriyoshi](https://github.com/zeriyoshi)) - [zeriyoshi@gmail.com](mailto:zeriyoshi@gmail.com)
