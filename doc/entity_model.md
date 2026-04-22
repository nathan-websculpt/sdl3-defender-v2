## entity model overview

- Player, Projectile, Particle, HealthItem entities
- BaseOpponent + subtype variants
- player owned via unique_ptr
- dynamic sets use plf::colony
- opponents: colony<unique_ptr<BaseOpponent>>
- projectiles: colony<Projectile>
- health items: colony<unique_ptr<HealthItem>>
- projectiles pruned by bounds, expiry, collision-with-ground
- particles pruned via isAlive bool
- health items updated in ColonyUpdateAndPrune::healthItems
- same-pass erase in opponents + collisions
- Game::update handles simulation mutation (input handling also mutates state)