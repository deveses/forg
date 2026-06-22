# Phase 6: Canonical Windows Support

## Status

In progress and partially verified. Visual Studio 2022 presets, Windows CI jobs,
and CMake targets for `forg`, software/OpenGL renderers, and `winapp`
build with MSVC x64. Local Windows debug and release CMake builds and CTest runs
pass, including package consumption and software/OpenGL plugin lifecycle tests.
`winapp` now uses a direct Win32 shell. Interactive `winapp` smoke validation
and Sharpmake retirement remain pending.

## Objective

Make CMake and CTest the authoritative Windows build, then retire generated
Sharpmake projects without losing supported behavior.

## Implementation Slices

1. **Core Windows build**
   - Make `forg` and `forg_tests` compile with MSVC x64 in debug and release.
   - Fix platform source selection, Win32 type conflicts, export/static defines,
     resource compiler setup, runtime-library consistency, and warning parity.
   - Run the same deterministic unit tests and package-consumer test as macOS.

2. **Backend parity**
   - Build and smoke-test the GDI software renderer and OpenGL renderer DLLs.
   - Validate descriptor exports, DLL copying, configuration filenames, and
     create/destroy lifecycle from a test host.
   - Treat GPU/window presentation as a compile and lifecycle gate in CI; keep
     interactive visual checks documented for local Windows testing.

3. **Application parity**
   - Build `winapp` with current Windows SDK defaults.
   - Verify startup, config parsing, renderer selection, resize, input, shutdown,
     and missing/incompatible plugin diagnostics on a Windows machine.

4. **Legacy retirement**
   - Compare CMake outputs against the Sharpmake project target/source/define/link
     matrix and record any intentional omissions.
   - After two consecutive green main-branch Windows CI runs plus one documented
     local application smoke test, remove generated Visual Studio projects,
     Sharpmake generation scripts, and obsolete instructions in a dedicated PR.
   - Keep OpenCL and C++ AMP explicitly unsupported or archive them separately;
     they do not block canonical Windows parity.

## Public Interfaces

No intentional API changes. Windows static/shared definitions, plugin filenames,
and runtime configuration keys must remain compatible unless release notes and a
transition path are supplied.

## Acceptance Gates

- Complete: Windows debug and release configure, build, test, install, and
  consume cleanly with Visual Studio 2022 MSVC x64.
- Complete: Software and OpenGL plugin lifecycle tests pass on Windows.
- Pending: `winapp` completes the documented local smoke checklist.
- Pending: CMake parity is documented and Sharpmake retirement conditions are
  satisfied.
