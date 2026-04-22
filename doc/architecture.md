## architecture overview

- main.cpp constructs Game, Platform
- app_bootstrap parses seed, sets working dir
- platform owns SDL init, runtime loop
- game.cpp routes state, top-level sim
- game_input_handler maps input → state changes
- game_update_handler runs motion, timers, spawns
- render helpers split: main / hud / screens
- managers cache textures, fonts, audio (mutable runtime singletons)
- gameplay state: GameStateData plus Game-owned timers/RNG streams
- platform/runtime shared state: globals (renderer + window dimensions)
- opponent hierarchy via BaseOpponent inheritance

## helper split

- helpers_platform is for reusable clipping calculations
- helpers_platform_rendering is for render-path helpers (draw plus render-side cache/state utilities)
