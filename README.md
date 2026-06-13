# FORG

FORG is a C++17 rendering-API abstraction library. It defines a common set of rendering interfaces (`IRenderDevice`, `IRenderer`, `ITexture`, `IVertexBuffer`, …) and provides multiple backends behind them — a reference software renderer built into the library, plus OpenGL, software, OpenCL, and C++ AMP renderer plugins. It originated from the old `forg.googlecode.com` project.

## Supported platforms

- **macOS** (arm64) — primary CMake target
- **Windows** — legacy MSVC/Sharpmake build (renderer plugin DLLs and Win32 sample app)
- iOS platform macros exist (`FORG_PLATFORM_IOS`) but there is no app target yet

Any other platform fails CMake configuration with a fatal error.

## Building

Requires CMake ≥ 3.21 (for presets), Ninja, and a C++17 compiler. `CMakePresets.json` defines `debug` and `release` presets with output in `build/debug` and `build/release`:

```sh
cmake --preset release
cmake --build --preset release
```

(Use `--preset debug` for a debug build. The software renderer is ~4× faster in release.)

The CMake build produces these main targets:

- **`forg`** — the static library (`src/forg/`)
- **`swrenderer`** (macOS) — the software-renderer plugin, built as `libswrenderer.dylib`
- **`macapp`** (macOS) — the Cocoa sample app (`src/macapp/`): reads `config.xml` for window geometry and the renderer driver, loads the plugin with `dlopen`, and renders the demo scene
- **`forg_tests`** — Catch2-based unit tests, built when CMake testing is enabled

Run the sample with:

```sh
./build/release/src/macapp/macapp
```

A post-build step copies `libswrenderer.dylib` and `src/macapp/config.xml` next to the binary.

Notes:

- `CMAKE_OSX_ARCHITECTURES` is hardcoded to `arm64` at the top of the root `CMakeLists.txt`; change it there to build for `x86_64`.
- The options `FORG_USE_OPENCL`, `FORG_USE_FREETYPE`, and `FORG_USE_ZLIB` default to `OFF`, and the vendored dependencies in `extern/` are not currently wired into the build.
- Source files for the `forg` library are listed explicitly in `src/forg/Sources.cmake` — new files must be added there.

## Testing

The CMake build uses CTest with Catch2 v3. Catch2 is fetched with CMake `FetchContent` and pinned in `tests/CMakeLists.txt`, so the first configure of a fresh build directory needs network access.

Run the test suite with:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --test-dir build/debug --output-on-failure
```

Release tests use the matching preset and build directory:

```sh
cmake --preset release
cmake --build --preset release
ctest --test-dir build/release --output-on-failure
```

Testing is controlled by CMake's standard `BUILD_TESTING` option. To configure without tests:

```sh
cmake --preset release -DBUILD_TESTING=OFF
```

Initial coverage lives under `tests/` and focuses on deterministic library behavior: math types, `BitArray`, XML parsing, color conversion, and vertex declaration helpers. App/plugin tests, OpenCL, Cocoa windowing, and visual renderer validation are intentionally outside the first test layer.

### Legacy Windows build

The remaining renderer plugins (`glrenderer`, `clrenderer`, `amprenderer`, the GDI side of `swrenderer`) and the Win32 sample apps (`winapp`, `emfc`) are **not** part of the CMake build. They are built with the Sharpmake-generated Visual Studio projects under `tools/msvc/` (regenerate with `tools/generateprojects.bat`, sources in `tools/sharpmake/`).

## Project layout

```text
src/forg/include/forg/   Public headers, one directory per module:
                         math, rendering, audio, core, fs, os, script,
                         image, mesh, ui, cpu, opencl, debug
src/forg/src/            Private implementation, mirroring the module layout;
                         OS-specific code under os/{win32,osx}/
src/macapp/              Cocoa sample app (CMake)
src/winapp/, src/emfc/   Win32 sample apps (legacy MSVC build)
src/swrenderer/          Software-renderer plugin (CMake dylib on macOS,
                         legacy MSVC DLL on Windows)
src/{gl,cl,amp}renderer/ Renderer plugin DLLs (legacy MSVC build)
tests/                   Catch2/CTest unit tests for the forg library
extern/                  Vendored dependencies (freetype, zlib, OpenCL,
                         OpenGL headers, Sharpmake) — not wired into CMake
tools/                   Sharpmake scripts, MSVC projects, clang-format
```

The umbrella headers are `forg/forg.h` and `forg/rendering.h`. Note that some `.cpp` files live alongside their headers in the `include/` tree — it is not header-only.

## Architecture

The rendering abstraction lives in `include/forg/rendering/`. Backends implement the `IRenderDevice` family of interfaces:

- **Reference software renderer** — compiled into the library itself (`include/forg/rendering/reference/`)
- **Software renderer plugin** (`src/swrenderer/`) — wraps the reference renderer with a platform presentation layer (CoreGraphics/CALayer on macOS, GDI on Windows); loaded at runtime via `dlopen`/`LoadLibrary` from the driver named in `config.xml`
- **OpenGL / OpenCL / C++ AMP renderers** — separate plugin DLLs loaded at runtime (Windows only, legacy build)

Beyond rendering, the library includes math types, audio output, an XML parser/lexer (`script`), image and mesh loading, a UI layer, filesystem and OS abstractions.

## CI

GitHub Actions runs the standard CMake configure, build, and CTest workflows (`.github/workflows/`).

## License

Licensed under the GNU LGPL v3 — see [COPYING.LESSER](COPYING.LESSER) and [COPYING](COPYING).
