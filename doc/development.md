# development

- requires CMake 4.2+

## run

one-line for build / run

```powershell
./build.ps1 -VcpkgRoot "C:\vcpkg\dir"
```

build-tree runnable output (windows preset flow)

```powershell
.\build-win-vcpkg\bin\Debug\SDL3Defender.exe
```

or double-click `build-win-vcpkg/bin/Debug/SDL3Defender.exe`

## run all tests

`SDL3DEFENDER_BUILD_TESTS` defaults to `ON`. when enabled, configure requires `GTest` config packages.

```powershell
./test.ps1 -VcpkgRoot "C:\vcpkg\dir"
```

optional: run tests in release config

```powershell
./test.ps1 -VcpkgRoot "C:\vcpkg\dir" -Config Release
```

if you only want the game (tests off)

```powershell
$env:VCPKG_ROOT = "C:/path/to/vcpkg"
cmake --preset windows-vcpkg -D SDL3DEFENDER_BUILD_TESTS=OFF
cmake --build --preset windows-debug --target SDL3Defender
```

## analyze

requires `VCPKG_ROOT` in the current shell because the preset reads `$env{VCPKG_ROOT}`.

```powershell
$env:VCPKG_ROOT = "C:/path/to/vcpkg"
./analyze.ps1
```

optional: clean analyze build directory first

```powershell
./analyze.ps1 -Clean
```

## windows (preset-first)

set `VCPKG_ROOT` to your local vcpkg checkout

presets in `CMakePresets.json` are constrained to Windows hosts and currently pin the generator to
`Visual Studio 18 2026`.

```powershell
$env:VCPKG_ROOT = "C:/path/to/vcpkg"
cmake --preset windows-vcpkg
cmake --build --preset windows-debug --target SDL3Defender
```

## windows diagnostics lane (opt-in)

use the dedicated analyze preset for an opt-in MSVC diagnostics build path.
this preset sets `SDL3DEFENDER_MSVC_DIAGNOSTICS=ON` and `SDL3DEFENDER_BUILD_TESTS=OFF`.

```powershell
$env:VCPKG_ROOT = "C:/path/to/vcpkg"
cmake --preset windows-vcpkg-analyze
cmake --build --preset windows-debug-analyze --target SDL3Defender
```

when diagnostics are enabled on msvc, CMake adds `/W4`, `/analyze`, `/external:W0`, and marks both project `external/` and third-party include roots (like vcpkg) as `/external:I...`

## run all tests (manual cmake/ctest)

optional lower-level equivalent of `test.ps1` for explicit cmake/ctest control

```powershell
$env:VCPKG_ROOT = "C:/path/to/vcpkg"
cmake --preset windows-vcpkg
cmake --build --preset windows-debug --target SDL3Defender_tests
ctest --test-dir build-win-vcpkg -C Debug --output-on-failure
```

release variant

```powershell
$env:VCPKG_ROOT = "C:/path/to/vcpkg"
cmake --preset windows-vcpkg
cmake --build --preset windows-release --target SDL3Defender_tests
ctest --test-dir build-win-vcpkg -C Release --output-on-failure
```

optional: list all discovered tests without running them

```powershell
ctest --test-dir build-win-vcpkg -C Debug -N
```

## install (staged output)

install from an existing build tree

```powershell
cmake --install build-win-vcpkg --config Release --prefix build-win-vcpkg/install
```

install tree currently stages:

- `SDL3Defender` runtime target at install root (`SDL3Defender.exe` on Windows)
- runtime dependencies via `RUNTIME_DEPENDENCY_SET`
- `assets/` and `resources/`

note: build-tree post-build copy can include `data/` if present; current install rules do not
install `data/`.

## linux convenience script

`build.sh` is a Linux convenience build/run flow.

```bash
./build.sh
```

`build.sh` does not install dependencies. CMake still expects discoverable config packages for
`SDL3`, `SDL3_image`, `SDL3_ttf`, and `SDL3_mixer` (and `GTest` when tests are on).

`build.sh` also sets environment variables for the process running the script:

- prepends `$HOME/.local/lib` to `LD_LIBRARY_PATH`
- prepends `$HOME/.local/bin` to `PATH`
- prepends `$HOME/.local/lib/pkgconfig` to `PKG_CONFIG_PATH`
- sets `SDL_AUDIODRIVER=pipewire`

if you only want the game on Linux (tests off)

```bash
cmake -S . -B build -D SDL3DEFENDER_BUILD_TESTS=OFF
cmake --build build -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
```

## runtime paths / writable data

- startup attempts to set process working directory to `SDL_GetBasePath()`
- high scores writable path on Windows uses
  `SDL_GetPrefPath("SDL3Defender", "SDL3Defender")/highscores.txt`
- fallback path (and non-Windows default) is `resources/highscores.txt` under the base path
- if high score persistence fails (for example read-only install locations), the game logs a warning and
  continues with in-memory score updates for the session

## reproducibility / seeds

- launch seed format is `--seed=<uint64>`
- invalid `--seed=` values fail startup with an error message
- startup logs resolved seeds as `rng seeds base=<...> sim=<...> fx=<...>`
- default launch (no `--seed`) uses runtime entropy for base seed selection
- deterministic comparisons should keep external runtime inputs consistent (for example window size/resize events)

example:

```powershell
.\build-win-vcpkg\bin\Debug\SDL3Defender.exe --seed=12345
```

## contributor note: source discovery

project sources are collected with `file(GLOB_RECURSE ...)` (without `CONFIGURE_DEPENDS`).
after adding/removing `.cpp` files under `core/` or `entities/`, rerun configure:

```powershell
cmake --preset windows-vcpkg
```

```bash
cmake -S . -B build
```

test sources are explicitly listed in `CMakeLists.txt` for `SDL3Defender_tests`.
after adding/removing `.cpp` files under `tests/`, update that source list and rerun configure.

## Open in Visual Studio

1. Install Visual Studio with the **Desktop development with C++** workload.
2. Ensure `VCPKG_ROOT` is set to your vcpkg installation path.
3. Open the repository root folder in Visual Studio (`File > Open > Folder`).
4. Select a build preset from the toolbar:
   - `windows-debug` (for development)
   - `windows-release` (for optimized builds)
5. If prompted about a toolchain/cache mismatch, click **Delete and regenerate cache**.
6. Build and run the executable target from the toolbar.
