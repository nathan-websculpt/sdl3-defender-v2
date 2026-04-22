## game loop overview

- Platform::run drives loop
- pollInput gathers input each fixed-step iteration
- Game::handleInput applies state logic
- Game::update runs simulation step
- prune runs inside update
- collision runs after prune
- render after the fixed-step catch-up loop
- fixed timestep
- delta clamp: 0.2s
- spawn timers use catch-up while loops
- loop can execute 0..N fixed updates before each render
- deterministic seed available via --seed
- default launch without --seed uses runtime-selected base seed
- deterministic comparisons assume consistent external runtime inputs (for example window resize behavior)
- stable update ordering per tick
- prune runs before collision inside update; no clean-set guarantee
