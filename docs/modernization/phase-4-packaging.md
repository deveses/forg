# Phase 4: Project Structure and Packaging

## Status

Done. Implementations live under `src/forg/src`, PCH is optional, install/export
rules exist, and the package-consumer test installs the SDK, validates the
install tree, configures through `find_package(Forg 1.0 CONFIG REQUIRED)`,
builds, and runs the consumer. Public-header isolation is part of the default
test build and runs in the AppleClang and MSVC CI presets.

## Objective

Provide a conventional, self-contained SDK layout usable through
`find_package(Forg CONFIG REQUIRED)` without source-tree assumptions.

## Implementation Slices

1. **Header boundary audit**
   - Keep every installed header compiling independently with PCH disabled on
     AppleClang, and run the same header target on MSVC.
   - Replace reliance on private headers or accidental transitive includes.
   - Ensure no implementation file or private debug helper is installed.

2. **Target hygiene**
   - Keep build-tree and install-tree include directories separate.
   - Ensure private dependencies do not leak through `Forg::forg`.
   - Give every optional backend a target-local option and dependency check.

3. **Package validation**
   - Test install and consumption from a clean directory on macOS and Windows in
     debug and release configurations.
   - Verify version compatibility through `ForgConfigVersion.cmake`.
   - Add an installed consumer that exercises value types, mesh declarations,
     and the renderer plugin descriptor, not only `Color`.

4. **Documentation**
   - Document build-tree use, installation, `CMAKE_PREFIX_PATH`, target names,
     options, supported compilers, and static-library definitions.

## Public Interfaces

The canonical imported target is `Forg::forg`. Installed include paths remain
`<forg/...>`. No private source-directory include path may be required by a
consumer.

## Acceptance Gates

- Every public header compiles independently on AppleClang and MSVC.
- Clean installed consumers build and run on macOS and Windows.
- The install tree contains headers, library artifacts, and CMake package files
  only where expected.
- PCH-on and PCH-off builds have identical test results.
