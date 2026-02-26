# AGENTS.md

This file provides **agent-oriented guidance** for working in the eCAL repository.
It summarizes repository layout, component ownership boundaries, and practical coding / validation rules derived from the current build and CI configuration.

## 1) Repository structure at a glance

Top-level components:

- `ecal/`: Core middleware libraries, core APIs, service internals, benchmarks, and low-level tests.
- `app/`: End-user applications and tools (CLI / TUI / GUI), including monitor, recorder, player, sys, and utilities.
- `serialization/`: Message-layer adapters and serializers (e.g. protobuf, string, capnproto, flatbuffers).
- `lang/`: Language bindings:
  - `lang/c/`: C API and tests/samples.
  - `lang/python/`: Python bindings (nanobind-based) + tests/samples.
  - `lang/csharp/`: C# bindings, tests, and samples.
- `lib/`: Shared helper libraries used by multiple components (parsers, utility libs, Qt helpers, threading utils).
- `contrib/`: Optional / extension components (e.g. measurement, ecalhdf5, time plugins).
- `samples/`: C++ sample applications.
- `tests/`: Additional integration / component tests not colocated with code.
- `doc/`: Sphinx documentation sources and tooling.
- `cmake/`, `cpack/`: Build and packaging infrastructure.
- `thirdparty/`: Vendored submodules / dependencies.

## 2) Build system and baseline expectations

- Primary build system is **CMake**.
- The root `CMakeLists.txt` defines many feature toggles (e.g. `ECAL_USE_*`, `ECAL_BUILD_*`, `ECAL_THIRDPARTY_BUILD_*`) and includes most components via `add_subdirectory(...)`.
- CI builds use **Ninja** on Linux and Windows for core builds, with additional Visual Studio generation for C# binding builds on Windows.

### Linux CI reference (Ubuntu)

The Ubuntu workflow (`.github/workflows/build-ubuntu.yml`) is a good baseline when validating broad changes:

1. Install dependencies (Qt, protobuf, HDF5, yaml-cpp, spdlog, tinyxml2, gtest, python venv tooling, etc.).
2. Initialize selected submodules.
3. Configure CMake with `-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake/submodule_dependencies.cmake` and a broad set of features enabled (apps, samples, tests, benchmarks, python binding, docs).
4. Build: `cmake --build _build --parallel -- -k 0`
5. Test: `ctest -V --test-dir _build`

### Windows CI reference

The Windows workflow (`.github/workflows/build-windows.yml`) uses:

- MSVC toolchain (v142 / VS2019 compatibility, running on windows-2022).
- Ninja for SDK + complete builds.
- A separate Visual Studio configure/build for `lang/csharp` tests/samples.
- Typical validate sequence: configure -> build -> ctest for core and C#.

When you need a “known good” matrix of build flags, mirror these workflows first.

## 3) Coding guidelines by component

## 3.1 C++ core, libraries, apps, and contrib (`ecal/`, `app/`, `lib/`, `contrib/`, `samples/`, `serialization/`)

- Follow existing local style in the touched file; minimize unrelated formatting churn.
- Keep public API changes intentional and synchronized with affected bindings / samples / docs.
- Prefer CMake options and existing helper macros/functions over ad-hoc build logic.
- Preserve cross-platform behavior (Linux + Windows at minimum).

Static analysis guidance:

- `.clang-tidy` is configured and enforced in CI review workflows for C/C++ source changes.
- The enabled checks are broad (`clang-analyzer`, `bugprone`, `cppcoreguidelines`, `modernize`, `performance`, `readability`) with selected suppressions.
- clang-tidy excludes thirdparty, build directories, tests, and language-binding directories in the review workflow.

Practical rule: for non-trivial C/C++ changes, run at least targeted build + tests for affected components and keep code clang-tidy friendly.

## 3.2 C binding (`lang/c/`)

- Keep C API stable and straightforward; avoid introducing C++-specific semantics into C-facing interfaces.
- Update corresponding samples/tests when API behavior changes.
- Validate through CMake options that include C binding and tests where applicable.

## 3.3 Python binding (`lang/python/`)

- Python packaging/build is defined in root `pyproject.toml` (scikit-build-core + hatch).
- Use existing test discovery conventions (`lang/python/tests`, files matching `*_test.py`).
- Keep binding behavior aligned with underlying core semantics.
- For Python-related changes, run relevant Python tests and/or binding build target (`ecal_python`) where possible.

## 3.4 C# binding (`lang/csharp/`)

- C# bindings are built/tested separately in Windows CI after creating an install tree from the core build.
- Keep C# sample/test projects consistent with exported native artifacts and CMake packaging.
- If modifying binding interfaces, ensure corresponding tests/samples still compile and run.

## 3.5 Documentation (`doc/`)

- Documentation is Sphinx-based, with dependencies defined in `doc/pyproject.toml` and `doc/requirements.txt` (used in CI).
- If behavior, options, or public interfaces change, update docs and examples in the same change.

## 4) Change-management expectations for agents

- Prefer **small, scoped changes**; avoid opportunistic refactors outside the task.
- Do not modify `thirdparty/` unless the task explicitly requires dependency updates.
- If changing build flags, packaging, or dependency handling, check both Linux and Windows workflow assumptions.
- Keep generated artifacts out of commits unless explicitly required.
- Preserve license headers and existing copyright notices.

## 5) Validation guidance (agent checklist)

Pick the smallest set that gives confidence for your scope:

1. **Configure** affected build(s) with CMake.
2. **Build** touched targets.
3. **Run tests** for touched components (ctest / pytest / binding tests).
4. If docs/user-facing behavior changed, run doc build or at least check docs compile path consistency.

For broad core changes, use the Ubuntu CI recipe as default local reference. For C# changes, include Windows/C# validation expectations from `build-windows.yml`.

## 6) Quick “where to edit” map

- Core middleware behavior: `ecal/core/`, `ecal/service/`
- Core APIs / protocol definitions: `ecal/core_pb/`, `serialization/`
- End-user tools: `app/`
- Shared utility libs: `lib/`
- Python: `lang/python/`
- C#: `lang/csharp/`
- C API: `lang/c/`
- Docs: `doc/`
- CI/build behavior references: `.github/workflows/`

---

If in doubt, mirror the closest existing implementation pattern in the same subdirectory and validate with the corresponding CI-equivalent build/test path.
