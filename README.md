##### Story

[Version 1 of the project](https://github.com/nathan-websculpt/sdl3-defender) was the project I used to ask for peer reviews of my code as I taught myself C++. The reviewers disagreed with one another on many topics, but I researched all opinions. These reviews led me to come back to Windows, so Version 1 is now a much older Linux based codebase that is frozen. Version 2 has many improvements, but I have yet to test on Linux, and the Windows release-gate is becoming an increasingly large part of what I am working on and learning, these days. I learned this with [the Garden Sim project](https://garden-sim.com) and am trying to find the time to finish this game's Windows release-gate. 

# running

## run

```powershell
./build.ps1 -VcpkgRoot "C:\vcpkg\dir"
```

## run all tests

```powershell
./test.ps1 -VcpkgRoot "C:\vcpkg\dir"
```

optional: run tests in release config

```powershell
./test.ps1 -VcpkgRoot "C:\vcpkg\dir" -Config Release
```

## analyze

```powershell
./analyze.ps1 -VcpkgRoot "C:\vcpkg\dir"
```

## windows (preset-first)

set `VCPKG_ROOT` to your local vcpkg checkout

```powershell
$env:VCPKG_ROOT = "C:/path/to/vcpkg"
cmake --preset windows-vcpkg
cmake --build --preset windows-debug --target SDL3Defender
```

## windows diagnostics lane (opt-in)

use the dedicated analyze preset for an opt-in MSVC diagnostics build path

```powershell
$env:VCPKG_ROOT = "C:/path/to/vcpkg"
cmake --preset windows-vcpkg-analyze
cmake --build --preset windows-debug-analyze --target SDL3Defender
```

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

run from terminal

```powershell
.\build-win-vcpkg\bin\Debug\SDL3Defender.exe
```

or double-click `build-win-vcpkg/bin/Debug/SDL3Defender.exe`

## linux convenience script

`build.sh` is still the Linux convenience flow

```bash
./build.sh
```

## Open in Visual Studio

1. Install Visual Studio with the **Desktop development with C++** workload.
2. Ensure `VCPKG_ROOT` is set to your vcpkg installation path.
3. Open the repository root folder in Visual Studio (`File > Open > Folder`).
4. Select a build preset from the toolbar:
   - `windows-debug` (for development)
   - `windows-release` (for optimized builds)
5. If prompted about a toolchain/cache mismatch, click **Delete and regenerate cache**.
6. Build and run the executable target from the toolbar.
