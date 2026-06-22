# Phase 2: C++20 Value Types and Utilities

## Status

In progress. `Color` and `Math` use C++20 facilities and have regression tests.
Vectors, matrices, quaternions, planes, declarations, and many legacy spellings
remain unchanged.

## Objective

Modernize non-owning value types without changing memory layout, numeric
conventions, serialized values, or rendering results.

## Implementation Slices

1. **Lock behavior and layout**
   - Add size, alignment, standard-layout, construction, and representative
     arithmetic tests for `Vector2/3/4`, `Matrix4`, `Quaternion`, `Plane`,
     `Color`, `VertexElement`, and `VertexDeclaration`.
   - Add compile-time assertions for layouts consumed by vertex buffers or plugin
     interfaces.

2. **Math types**
   - Replace constructor-body assignment with member initialization and defaulted
     special members.
   - Add `const`, `constexpr`, `[[nodiscard]]`, and `noexcept` only where tests and
     called operations justify them.
   - Preserve handedness, matrix storage order, angle units, normalization edge
     cases, and current floating-point tolerances.

3. **Standard-library utilities**
   - Replace remaining local bit/count/min/max helpers with `<bit>`, `<algorithm>`,
     `<numbers>`, and `<limits>` while retaining compatibility wrappers.
   - Replace unsafe zeroing of non-trivial values and C-style numeric casts in
     maintained math/rendering value code.

4. **Legacy spelling cleanup**
   - Replace reserved include guards, `NULL`, and implementation-local `typedef`
     declarations with portable equivalents.
   - Keep public aliases such as `uint` and LP-style names during 1.x; introduce
     fixed-width or `using` alternatives before deprecating them.

## Public Interfaces

- Existing constructors, fields, operators, and constants remain callable.
- Add named conversions where implicit conversions are error-prone; do not make
  existing conversions `explicit` during 1.x.
- No field reordering, virtual layout changes, or serialized representation changes.

## Acceptance Gates

- Layout assertions pass on arm64 macOS and x64 Windows.
- Existing and new math tests pass in debug, release, and sanitizers.
- Reference-renderer output checks remain unchanged.
- The installed-package consumer can use every modernized value header directly.
