# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

FORG is a C++17 rendering-API abstraction library (originating from the old forg.googlecode.com project, LGPL-3). The `README.md` is current enough to use as the first orientation source, with this file capturing repo-specific build notes for agents.

## Build

`CMakePresets.json` defines `debug` and `release` presets (Ninja) with output in `build/debug` and `build/release`:

```sh
cmake --preset debug && cmake --build --preset debug
cmake --preset release && cmake --build --preset release
```

Run tests through the matching CTest presets:

```sh
ctest --preset debug
ctest --preset release
```

Run the sample with `./build/<preset>/src/macapp/macapp` (a post-build step puts `libswrenderer.dylib` and `config.xml` next to it). The software renderer is ~4× faster in release (`-O0` debug runs ~20 fps at 800×600, release hits the 60 fps timer cap).

- `build/` is generated (not tracked). If a build tree's cache references a stale checkout path, delete that tree and reconfigure.
- CMake uses the host default macOS architecture. Pass `-DCMAKE_OSX_ARCHITECTURES=x86_64` or `-DCMAKE_OSX_ARCHITECTURES=arm64` at configure time when you need a specific architecture.
- Options `FORG_USE_OPENCL`, `FORG_USE_FREETYPE`, `FORG_USE_ZLIB` exist but default to OFF, and `extern/CMakeLists.txt` is currently empty — the vendored deps in `extern/` are not wired into the build.
- Only Windows and Apple platforms configure; anything else hits a `FATAL_ERROR`. Platform macros: `FORG_PLATFORM_WINDOWS`, `FORG_PLATFORM_OSX`, `FORG_PLATFORM_IOS`.
- GitHub Actions has one canonical CMake workflow on macOS, covering both `debug` and `release` configure/build/test presets.

## What the CMake build actually covers

`src/CMakeLists.txt` builds:

- **`forg`** — the static library (`src/forg/`), with PCH (`src/forg_pch.h`).
- **`swrenderer`** (macOS only) — the software-renderer plugin as `libswrenderer.dylib` (`SWRenderer.cpp` + the Cocoa presentation layer `SWRenderDevice_osx.mm`).
- **`macapp`** (macOS only) — the Cocoa sample app (`src/macapp/main.mm`): reads `config.xml` for window geometry and the renderer driver, `dlopen`s the plugin, renders the demo scene. A post-build step copies `libswrenderer.dylib` and `src/macapp/config.xml` next to the binary.
- **`forg_tests`** — Catch2/CTest coverage for deterministic library behavior such as math types, `BitArray`, XML parsing, color conversion, and vertex declaration helpers.

Everything else under `src/` (`glrenderer`, `clrenderer`, `amprenderer`, `winapp`, `emfc`, plus the Windows GDI side of `swrenderer`) is **not** part of the CMake build. Those are renderer plugin DLLs and a Win32 sample app from the legacy Windows build, driven by `tools/msvc/*.vcxproj` (Sharpmake-generated; see `tools/sharpmake/`, `tools/generateprojects.bat`).

### Adding/removing source files

`src/forg/Sources.cmake` lists every file of the `forg` library explicitly, grouped per module, with generator expressions for platform-specific files (e.g. `$<${FORG_PLATFORM_WINDOWS}:AudioOutputWaveOut.cpp>`). New files must be added there or they won't compile.

## Architecture

- **Public headers**: `src/forg/include/forg/` — modules: `math`, `rendering`, `audio`, `core`, `fs`, `os`, `script` (XML parser/lexer), `image`, `mesh`, `ui`, `cpu`, `opencl`, `debug`. Umbrella headers `forg.h`, `rendering.h`.
- **Private implementation**: `src/forg/src/`, mirroring the module layout; OS-specific code under `src/forg/src/os/{win32,osx}/`.
- **Rendering abstraction**: interfaces `IRenderDevice`, `IRenderer`, `ITexture`, `IVertexBuffer`, etc. in `include/forg/rendering/`. A reference software renderer is compiled into the library itself (`include/forg/rendering/reference/`). The OpenGL/software/OpenCL/C++AMP backends in `src/{gl,sw,cl,amp}renderer/` implement these interfaces as separate plugin DLLs (Windows `DllMain` entry points).
- Some `.cpp` files live in the `include/` tree (e.g. `rendering/*.cpp`, `rendering/reference/*.cpp`) — header and source side by side; don't assume `include/` is header-only.
