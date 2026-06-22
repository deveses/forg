# FORG Modernization Roadmap

## Summary

Modernize incrementally to C++20 while preserving existing source compatibility. Keep macOS and Windows first-class, retain current renderer behavior, and deliver each phase as a separately reviewable change.

Detailed execution plans, current status, and acceptance gates are indexed in
[`modernization/README.md`](modernization/README.md).

## Current Progress

As of 2026-06-20, Phase 1 implementation is complete locally. Warning-clean
debug, release, no-PCH, and sanitizer builds pass on AppleClang, along with all
75 tests, the installed-package consumer, the formatting gate, and 91
standalone public-header checks. The configured MSVC and clang-tidy GitHub
Actions jobs remain the final cross-platform acceptance gates.

## Implementation

1. **Build and quality baseline**
   - Apply warning flags directly to `forg` and backend targets instead of exposing them only through `INTERFACE`.
   - Add optional `FORG_WARNINGS_AS_ERRORS` and `FORG_ENABLE_CLANG_TIDY` settings.
   - Consolidate formatting configuration and add CI format/static-analysis checks.
   - Pin downloaded dependencies reproducibly and support installed Catch2 before using `FetchContent`.

2. **C++20 value types and utilities**
   - Raise the public compiler requirement to C++20.
   - Modernize `Color`, vectors, matrices, and `Math` with member initialization, defaulted special members, `const`, `noexcept`, and `constexpr` where behavior permits.
   - Implement existing bit/math helpers using `<bit>`, `<numbers>`, `<algorithm>`, and `<limits>`, preserving their current names and results.
   - Replace `NULL`, C-style casts, reserved include guards, legacy typedefs, and unsafe `memset` initialization in maintained code.

3. **Ownership and containers**
   - Replace remaining internal `core::vector`, raw arrays, and manual cleanup with standard containers and RAII.
   - Introduce an internal RAII wrapper for `AddRef`/`Release` resources.
   - Keep public raw-pointer factories and `MeshPtr` compatibility initially; mark custom `auto_ptr` APIs deprecated and provide additive modern factories returning `std::unique_ptr`.
   - Add ownership-focused tests before converting the X/PLY loaders and rendering resources.

4. **Project structure and packaging**
   - Move implementation `.cpp` files out of the public include tree into mirrored private source directories.
   - Make each public header self-contained instead of relying on the precompiled header.
   - Keep the PCH as an optional build optimization, never as a correctness dependency.
   - Add CMake install/export rules and a package-consumer test for `find_package(Forg)`.

5. **Renderer and plugin boundary**
   - Add a versioned C plugin descriptor alongside the existing `forgCreateRenderer` entry point.
   - Let the loader recognize legacy plugins while validating new plugin API versions explicitly.
   - Do not change existing virtual interface layouts until all in-tree backends use the versioned boundary.
   - Add headless reference-renderer tests plus plugin load/create/destroy smoke tests.

6. **Canonical Windows support**
   - Add CMake targets for the Windows library, `winapp`, OpenGL renderer, and Windows software renderer.
   - Use current MSVC and Windows SDK defaults rather than the pinned legacy SDK.
   - Add Windows debug/release presets and GitHub Actions jobs that build and run the same unit tests as macOS.
   - Retire generated Visual Studio projects after CMake reaches build parity.
   - Leave C++ AMP and OpenCL as explicitly unsupported legacy targets until independently revived or removed.

## Public Interfaces

- Existing method names, raw-pointer overloads, numeric enums, and renderer factories remain available during the 1.x modernization.
- Add modern overloads using `std::span`, `std::string_view`, scoped enums, and RAII return types; legacy overloads forward to them and become deprecated gradually.
- Source compatibility is the target, but binary compatibility is not guaranteed when consumers rebuild against a new 1.x release.
- A future v2 may remove custom containers, `auto_ptr`, LP-style aliases, and deprecated overloads.

## Test Plan

- Preserve the original 64 tests and all newly added regression tests throughout every phase.
- Run debug, release, and ASan/UBSan builds on macOS.
- Run debug and release builds and tests with current MSVC on Windows.
- Add tests for math edge cases, color clamping/conversion, ownership and failure paths, plugin-version rejection, backend lifecycle, and installed-package consumption.
- Require warning-clean builds for maintained targets before enabling warnings-as-errors in CI.

## Assumptions

- Maintainability is the primary goal; rendering behavior changes are out of scope.
- macOS and Windows are first-class; Linux is not part of this roadmap.
- Changes land as small subsystem-focused pull requests rather than one repository-wide rewrite.
- The first implementation slice is build-quality enforcement followed by `Color` and `Math`, since these are low-risk and already covered by tests.
