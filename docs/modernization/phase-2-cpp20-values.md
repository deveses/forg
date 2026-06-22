# Phase 2: C++20 Value Types and Utilities

## Status

Done. `Color`, `Math`, vectors, matrices, quaternions, material/light
values, and vertex declaration values use C++20 facilities and have layout or
behavior regression tests. The targeted value-code cleanup has removed live
C-style numeric casts, reserved guards, `memset`, `NULL`, and local typedefs
from those files and adjacent maintained rendering interfaces. `Plane` has no
public type in the current tree, so it is tracked as a stale roadmap item rather
than a modernization target. Debug, release, sanitizer, public-header, and
package-consumer gates pass on arm64 macOS, and x64 Windows layout validation
passes in CI.

## Objective

Modernize non-owning value types without changing memory layout, numeric
conventions, serialized values, or rendering results.

## Implementation Slices

1. **Lock behavior and layout**
   - Add size, alignment, standard-layout, construction, and representative
     arithmetic tests for `Vector2/3/4`, `Matrix4`, `Quaternion`, `Color`,
     and `VertexElement`.
   - Preserve `VertexDeclaration` as a legacy virtual interface and cover its
     size and behavior without claiming standard-layout.
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
   - Targeted math/rendering value files and adjacent rendering interfaces are
     clean; continue the same cleanup pattern opportunistically through broader
     maintained rendering and loader code.
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
