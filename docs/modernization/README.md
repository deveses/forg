# Modernization Phase Plans

These plans turn the roadmap into reviewable implementation slices. A phase is
complete only when every acceptance gate in its document passes. Work already
landed in `f174416` is recorded separately from remaining work.

## Current Status

| Phase | State | Next Gate |
| --- | --- | --- |
| Phase 1 | Done | None |
| Phase 2 | Done locally | Confirm x64 Windows CI |
| Phase 3 | In progress | Retire remaining first-party custom pointer/container uses |
| Phase 4 | Mostly complete | Validate headers and installed consumers on MSVC |
| Phase 5 | In progress | Add descriptor v2 destruction and runtime coverage |
| Phase 6 | Done | None |

## Recommended Order

1. [Phase 1: Build and Quality Baseline](phase-1-build-quality.md)
2. [Phase 6: Canonical Windows Validation](phase-6-windows.md)
3. [Phase 2: C++20 Value Types and Utilities](phase-2-cpp20-values.md)
4. [Phase 3: Ownership and Containers](phase-3-ownership.md)
5. [Phase 5: Renderer and Plugin Boundary](phase-5-plugins.md)
6. [Phase 4: Project Structure and Packaging](phase-4-packaging.md)

Windows validation moves forward because its feedback must shape subsequent API
and ownership changes. Phase 4 closes last because its consumer and header checks
serve as the final compatibility gate.

## Global Rules

- Preserve source compatibility during 1.x; removals belong to a future v2.
- Do not change rendering output unless a test demonstrates an existing defect.
- Keep each pull request focused on one subsystem or one quality gate.
- Every pull request must pass debug, release, ASan/UBSan, and package-consumer
  tests on supported hosts.
- Update the relevant phase document when scope or completion status changes.
