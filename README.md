# FORG

FORG is a C++20 rendering-API abstraction library. It defines a common set of rendering interfaces (`IRenderDevice`, `IRenderer`, `ITexture`, `IVertexBuffer`, …) and provides multiple backends behind them — a reference software renderer built into the library, a native Apple Metal backend (macOS), plus OpenGL, software, OpenCL, and C++ AMP renderer plugins. It originated from the old `forg.googlecode.com` project.

## Supported platforms

- **macOS** — primary CMake target
- **Windows** — CMake/MSVC build with OpenGL and software renderer plugins plus the Win32 sample app
- iOS platform macros exist (`FORG_PLATFORM_IOS`) but there is no app target yet

Any other platform fails CMake configuration with a fatal error.

## Building

Requires CMake >= 3.21, Ninja, and a C++20 compiler. `CMakePresets.json` defines `debug` and `release` presets with output in `build/debug` and `build/release`:

```sh
cmake --preset release
cmake --build --preset release
```

(Use `--preset debug` for a debug build. The software renderer is ~4× faster in release.)

The CMake build produces these main targets:

- **`forg`** — the static library (`src/forg/`)
- **`swrenderer`** (macOS) — the software-renderer plugin, built as `libswrenderer.dylib`
- **`metalrenderer`** (macOS) — the native Apple Metal backend, built as `libmetalrenderer.dylib`; the default `config.yml` driver
- **`macapp`** (macOS) — the Cocoa sample app (`src/macapp/`): reads `config.yml` for window geometry, control-server settings, and the renderer driver, loads the plugin with `dlopen`, and renders the demo scene
- **`forg_tests`** — Catch2-based unit tests, built when CMake testing is enabled

Run the sample with:

```sh
./build/release/src/macapp/macapp
```

A post-build step copies `libswrenderer.dylib`, `libmetalrenderer.dylib`, and `src/macapp/config.yml` next to the binary. `config.yml` selects which plugin `macapp` loads (default: `libmetalrenderer.dylib`; switch to `libswrenderer.dylib` to compare).

If `controlserver.enabled` is `true` in `config.yml`, the sample starts a
local HTTP control endpoint. It accepts scene/camera commands plus normalized
input events such as `/input/drag?button=left&dx=12&dy=-4` for orbit,
`/input/drag?button=right&dx=12&dy=-4` for truck, and
`/input/scroll?delta=1` for zoom. Native macOS and Win32 mouse handling uses
the same `forg::InputEvent` path through `Engine::HandleInput`.

Notes:

- CMake uses the host default macOS architecture. Pass `-DCMAKE_OSX_ARCHITECTURES=x86_64` or `-DCMAKE_OSX_ARCHITECTURES=arm64` at configure time when you need a specific architecture.
- The options `FORG_USE_OPENCL`, `FORG_USE_FREETYPE`, and `FORG_USE_ZLIB` default to `OFF`; their vendored dependencies in `extern/` (`freetype`, `zlib`, OpenCL/OpenGL headers) are not currently wired into the build. The header-only `cgltf` parser in `extern/cgltf/` *is* wired in (`extern/CMakeLists.txt`) and linked into `forg` for glTF mesh loading.
- Source files for the `forg` library are listed explicitly in `src/forg/Sources.cmake` — new files must be added there.

## Using FORG from CMake

The package target is **`Forg::forg`** and public headers are included as
`<forg/...>`.

Build-tree consumers can add this repository directly:

```cmake
add_subdirectory(path/to/forg)
target_link_libraries(my_app PRIVATE Forg::forg)
```

Installed consumers should point CMake at the install prefix:

```sh
cmake --install build/release --prefix /path/to/forg-sdk
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/forg-sdk
```

Then consume the exported package:

```cmake
find_package(Forg 1.0 CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE Forg::forg)
```

The install tree contains the static library, public headers under
`include/forg`, and package files under `lib/cmake/Forg`. Private source
directories are not part of the SDK surface.

Supported compiler configurations are AppleClang through the macOS presets and
MSVC 2022 through the Windows presets. `FORG_WARNINGS_AS_ERRORS`,
`FORG_ENABLE_CLANG_TIDY`, and `FORG_ENABLE_PCH` control local quality checks.
`FORG_USE_OPENCL`, `FORG_USE_FREETYPE`, and `FORG_USE_ZLIB` are project feature
switches and default to `OFF`. On Windows, `Forg::forg` publishes the
`FORG_STATIC`, `NOMINMAX`, and `WIN32_LEAN_AND_MEAN` definitions required by
consumers of the static library.

## Testing

The CMake build uses CTest with Catch2 v3. Catch2 is fetched with CMake `FetchContent` and pinned in `tests/CMakeLists.txt`, so the first configure of a fresh build directory needs network access.

Run the test suite with:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Release tests use the matching preset and build directory:

```sh
cmake --preset release
cmake --build --preset release
ctest --preset release
```

For local memory and undefined-behavior checks, use the sanitizer preset:

```sh
cmake --preset debug-asan
cmake --build --preset debug-asan
ctest --preset debug-asan
```

Testing is controlled by CMake's standard `BUILD_TESTING` option. To configure without tests:

```sh
cmake --preset release -DBUILD_TESTING=OFF
```

Coverage lives under `tests/` and focuses on deterministic library behavior:
math/value types, ownership helpers, XML/YAML parsing, mesh factories and glTF
loading, command parsing/queues, package consumption, renderer plugin ABI
compatibility, and a headless reference-renderer triangle checksum. Cocoa
windowing and OpenCL remain outside the automated test surface.

### Windows build

Use the `windows-debug` and `windows-release` presets with Visual Studio 2022. CMake builds `forg`, the direct Win32 `winapp`, `glrenderer`, the Windows software renderer, and the test suite. OpenCL and C++ AMP remain unsupported legacy targets until they are independently revived or removed.

A post-build step copies `glrenderer.dll`, `swrenderer.dll`, and `src/winapp/config.yml` next to `winapp.exe`. `config.yml` selects which plugin `winapp` loads and controls the initial window geometry.

## Project layout

```text
src/forg/include/forg/   Public headers and root APIs such as Engine.h/Input.h,
                         plus one directory per module:
                         math, rendering, audio, core, fs, os, script,
                         image, mesh, nn, ui, cpu, opencl, debug
src/forg/src/            Private implementation, mirroring the module layout;
                         OS-specific code under os/{win32,osx}/
src/macapp/              Cocoa sample app (CMake)
src/winapp/              Direct Win32 sample app (CMake)
src/swrenderer/          Software-renderer plugin (CMake dylib on macOS,
                         CMake DLL on Windows)
src/metalrenderer/       Native Apple Metal renderer plugin (CMake dylib, macOS)
src/glrenderer/          OpenGL renderer plugin (CMake DLL on Windows)
src/{cl,amp}renderer/    Unsupported legacy renderer sources
tests/                   Catch2/CTest unit tests for the forg library
docs/nn/README.md        Notes and examples for the tiny neural-network module
extern/                  Vendored dependencies: cgltf (glTF 2.0 parser, wired
                         into CMake and linked into forg); freetype, zlib,
                         OpenCL/OpenGL headers
tools/                   Repository tooling, including clang-format
```

The umbrella headers are `forg/forg.h` and `forg/rendering.h`. Note that some `.cpp` files live alongside their headers in the `include/` tree — it is not header-only.

## Architecture

The rendering abstraction lives in `include/forg/rendering/`. Backends implement the `IRenderDevice` family of interfaces:

- **Reference software renderer** — compiled into the library itself (`include/forg/rendering/reference/`)
- **Software renderer plugin** (`src/swrenderer/`) — wraps the reference renderer with a platform presentation layer (CoreGraphics/CALayer on macOS, GDI on Windows); loaded at runtime via `dlopen`/`LoadLibrary` from the driver named in the sample app config
- **Metal renderer plugin** (`src/metalrenderer/`) — native Apple Metal backend hosting a `CAMetalLayer` in the sample's `NSView`; macOS only, the default `config.yml` driver
- **OpenGL renderer plugin** (`src/glrenderer/`) — loaded at runtime on Windows via `LoadLibrary`
- **OpenCL / C++ AMP renderers** — unsupported legacy sources, not part of the canonical CMake build

Canonical renderer plugins expose a versioned descriptor. Version 2 adds a
plugin-local destroy callback so hosts do not delete plugin-owned renderer
objects across module or CRT boundaries. The loaders still accept version-1
descriptors and legacy `forgCreateRenderer`-only plugins for 1.x
compatibility.

Beyond rendering, the library includes math types, audio output, XML and YAML parsers (`script`), image loading, mesh loading (DirectX `.x`, `.ply`, and glTF 2.0 `.gltf`/`.glb` static meshes via `Mesh::FromFile`), a UI layer, filesystem and OS abstractions.

`Engine` owns the active camera and handles normalized input events from
`forg/Input.h`; sample apps only translate native events before calling
`Engine::HandleInput`.

## CI

GitHub Actions runs canonical debug and release CMake workflows on macOS and Windows. Linux remains unsupported and fails configuration explicitly.

## License

Licensed under the GNU LGPL v3 — see [COPYING.LESSER](COPYING.LESSER) and [COPYING](COPYING).
