# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

FORG is a C++17 rendering-API abstraction library (originating from the old forg.googlecode.com project, GPL-3). The `README.md` is lorem-ipsum placeholder — rely on the source. There are no tests; CI (`.github/workflows/`) just runs the standard CMake configure+build starter workflows.

## Build

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

- The checked-in `build/` directory may be stale (it can reference a previous checkout path in its cache). If `cmake --build` fails trying to regenerate from a wrong source path, delete `build/` and reconfigure.
- `CMAKE_OSX_ARCHITECTURES` is hardcoded to `arm64` at the top of the root `CMakeLists.txt`.
- Options `FORG_USE_OPENCL`, `FORG_USE_FREETYPE`, `FORG_USE_ZLIB` exist but default to OFF, and `extern/CMakeLists.txt` is currently empty — the vendored deps in `extern/` are not wired into the build.
- Only Windows and Apple platforms configure; anything else hits a `FATAL_ERROR`. Platform macros: `FORG_PLATFORM_WINDOWS`, `FORG_PLATFORM_OSX`, `FORG_PLATFORM_IOS`.

## What the CMake build actually covers

`src/CMakeLists.txt` only builds two targets:

- **`forg`** — the static library (`src/forg/`), with PCH (`src/forg_pch.h`).
- **`macapp`** — a minimal Cocoa app (`src/macapp/main.mm`) linking `forg`.

Everything else under `src/` (`glrenderer`, `swrenderer`, `clrenderer`, `amprenderer`, `winapp`, `emfc`) is **not** part of the CMake build. Those are renderer plugin DLLs and a Win32 sample app from the legacy Windows build, driven by `tools/msvc/*.vcxproj` (Sharpmake-generated; see `tools/sharpmake/`, `tools/generateprojects.bat`).

### Adding/removing source files

`src/forg/Sources.cmake` lists every file of the `forg` library explicitly, grouped per module, with generator expressions for platform-specific files (e.g. `$<${FORG_PLATFORM_WINDOWS}:AudioOutputWaveOut.cpp>`). New files must be added there or they won't compile.

## Architecture

- **Public headers**: `src/forg/include/forg/` — modules: `math`, `rendering`, `audio`, `core`, `fs`, `os`, `script` (XML parser/lexer), `image`, `mesh`, `ui`, `cpu`, `opencl`, `debug`. Umbrella headers `forg.h`, `rendering.h`.
- **Private implementation**: `src/forg/src/`, mirroring the module layout; OS-specific code under `src/forg/src/os/{win32,osx}/`.
- **Rendering abstraction**: interfaces `IRenderDevice`, `IRenderer`, `ITexture`, `IVertexBuffer`, etc. in `include/forg/rendering/`. A reference software renderer is compiled into the library itself (`include/forg/rendering/reference/`). The OpenGL/software/OpenCL/C++AMP backends in `src/{gl,sw,cl,amp}renderer/` implement these interfaces as separate plugin DLLs (Windows `DllMain` entry points).
- Some `.cpp` files live in the `include/` tree (e.g. `rendering/*.cpp`, `rendering/reference/*.cpp`) — header and source side by side; don't assume `include/` is header-only.
