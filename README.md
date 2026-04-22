# Defender-style game in C++ / SDL3

![Gameplay Demo](doc/assets/def-gif-1.gif)

## Current Design

- **Deterministic simulation**  
  Fixed timestep and stable update ordering, with seed-driven reproducibility when `--seed=<uint64>` is provided under consistent runtime inputs.

- **Clear ownership with mostly centralized mutation**  
  Gameplay state is primarily modeled in `GameStateData`, with simulation orchestration, timers, and RNG streams owned by `Game`. Platform/runtime state (for example window dimensions and renderer handles) is shared separately via `globals` and manager singletons. Most gameplay mutation occurs through `Game` update and input paths, though mutation is not strictly constrained to narrow interfaces.

- **Separation of simulation and presentation**  
  Rendering receives gameplay state through const interfaces and currently treats it as read-only; render-side caches and SDL render state may still mutate.

- **Production-first build and runtime validation**  
  Build, test, and runtime workflows are provided as explicit scripts and presets.

## What this Project Emphasizes

This is a small game used to explore system-level concerns:

- deterministic simulation via fixed-step update, stable ordering, and seeded RNG
- explicit state ownership with mostly centralized mutation
- container and lifetime tradeoffs (e.g., `plf::colony`)
- gameplay/render separation without claiming fully immutable render internals
- preset-driven CMake configuration, PowerShell build/test/analyze scripts, and an optional diagnostics lane (`/analyze`) define a consistent Windows development workflow

## Docs

- [Development](doc/development.md) — build, test, and analysis workflows
- [Architecture](doc/architecture.md) — modules, ownership
- [Entity Model](doc/entity_model.md) — containers, lifetime
- [Game Loop](doc/game_loop.md) — fixed-step, ordering
- [Rendering Pipeline](doc/rendering_pipeline.md) — draw order
- [Notes](doc/notes.md) — dev notes
- [Roadmap](doc/roadmap.md) — pending work

## Quick Start

Requires CMake 4.2+.

### Run Game
```powershell
./build.ps1 -VcpkgRoot "C:\vcpkg\dir"
```

See [Development](doc/development.md) in the docs for more information about running, testing, or performing static analysis.

## Architecture

### Startup
- main → bootstrap (seed, working dir)
- construct `Game`
- hand off to `Platform::run`

### Runtime ownership
- **Platform**
  - SDL init + shutdown
  - main loop
  - input polling
- **Game**
  - simulation orchestration
  - timers + RNG
  - all gameplay mutation
- **GameStateData**
  - entity containers
  - core gameplay state
- **globals**
  - renderer
  - window dimensions
- **managers**
  - texture / font / sound caches

### Mutation rules
- input mutates state
- update mutates state
- collision mutates state (often immediate)
- pruning is distributed (no single owner)

### Rendering rules
- takes `const GameStateData&`
- no gameplay mutation
- mutates render-side state:
  - caches
  - SDL objects

### Entity lifecycle
- no single lifecycle model
- removal is path-dependent:
  - immediate erase **or**
  - deferred prune

## License

This project is licensed under the MIT License.

Third-party libraries, fonts, and audio assets are licensed separately.
See [THIRD_PARTY_LICENSES](THIRD_PARTY_LICENSES.txt) for details.
