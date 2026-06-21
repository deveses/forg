# Phase 1: Build and Quality Baseline

## Status

Implementation complete; cross-platform CI verification pending. The macOS
build is warning-clean, all canonical presets enable warnings as errors, and CI
contains dedicated formatting, clang-tidy, no-PCH, sanitizer, debug, release,
and Windows jobs.

## Objective

Make compiler and static-analysis feedback reliable enough to gate every later
modernization change.

## Implementation Slices

1. **Warning cleanup**
   - Capture warning inventories for `forg`, tests, software renderer, Metal,
     OpenGL, and sample applications on AppleClang and MSVC.
   - Fix unused parameters, signed comparisons, shadowing, format handling, and
     deprecated enum operations in maintained targets.
   - Suppress warnings only for external headers or platform APIs, at the
     narrowest target/source boundary and with a comment explaining why.

2. **Formatting gate**
   - Promote one root `.clang-format` and remove or reconcile conflicting local
     configurations.
   - Add `forg-format-check`, covering tracked C, C++, Objective-C++, and header
     files with `clang-format --dry-run --Werror`.
   - Add a matching CI job; formatting remains a check, never an automatic CI
     rewrite.

3. **Static analysis and header checks**
   - Keep `.clang-tidy` focused on bug-prone, performance, and portability rules;
     record intentional exclusions in that file.
   - Add a `forg-clang-tidy` preset/job for first-party targets on macOS.
   - Generate one compile-only translation unit per installed public header,
     including only that header, and build them with PCH disabled.

4. **CI enforcement**
   - Add a macOS no-PCH job and enable `FORG_WARNINGS_AS_ERRORS` on macOS and
     Windows after both warning inventories reach zero.
   - Keep sanitizer tests as a separate required job so failures are attributable.

## Public Interfaces

No intentional API or ABI changes. Include fixes may add missing standard headers
to public headers but must not remove transitive includes until a later major
version.

## Acceptance Gates

- `FORG_WARNINGS_AS_ERRORS=ON` builds every canonical target on macOS and Windows.
- Format, clang-tidy, public-header, PCH-off, debug, release, and sanitizer jobs pass.
- Third-party warnings do not appear as first-party failures.
- All existing tests and the installed-package consumer pass.

## Implementation Evidence

- A root `.clang-format` and `forg-format-check` cover tracked first-party C,
  C++, Objective-C++, and header files. Conflicting nested configurations were
  removed and the current tree passes the check with clang-format 18.1.5.
- Warning cleanup covers the library, tests, software renderer, Metal renderer,
  and macOS application. The `debug`, `release`, `debug-asan`, `no-pch`, and
  Windows presets now set `FORG_WARNINGS_AS_ERRORS=ON`.
- `forg-public-headers` generates and compiles one translation unit for each of
  the 91 installed headers without PCH. This exposed and fixed a missing
  `<fstream>` include, an x86-only SIMD include on ARM, and a private OpenCL
  configuration-header dependency.
- The `no-pch` and `clang-tidy` presets provide reproducible local and CI entry
  points. clang-tidy applies only to first-party targets, not fetched Catch2
  sources.
- GitHub Actions now separates formatting, clang-tidy, no-PCH, sanitizer,
  macOS debug/release, and Windows debug/release failures.

## Verification

Verified locally on AppleClang/arm64 on 2026-06-20:

- `cmake --build --preset debug` and `ctest --preset debug`: 75/75 passed.
- `cmake --build --preset release` and `ctest --preset release`: 75/75 passed.
- `cmake --build --preset no-pch` and `ctest --preset no-pch`: 75/75 passed.
- `cmake --build --preset debug-asan` and `ctest --preset debug-asan`: 75/75
  passed under AddressSanitizer and UndefinedBehaviorSanitizer.
- `forg-format-check`, all 91 public-header translation units, and the installed
  package consumer pass.
- The SSE public header also passes an AppleClang x86_64 syntax-only build with
  warnings as errors.

The MSVC and clang-tidy gates are configured but require their GitHub Actions
jobs to establish final Phase 1 completion.
