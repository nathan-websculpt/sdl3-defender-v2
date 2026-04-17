#pragma once
#include <core/helpers_game/game_helper.h>
#include <entities/health_item.h>
#include <entities/particle.h>
#include <entities/projectile.h>
#include <plf/plf_colony.h>
namespace ColonyUpdateAndPrune
{
void projectiles(plf::colony<Projectile>& projectiles, float deltaTime, const GameHelper& helpers);
void particles(plf::colony<Particle>& particles, float deltaTime);
void healthItems(plf::colony<std::unique_ptr<HealthItem>>& healthItems, float deltaTime,
                 const GameHelper& helpers);
} // namespace ColonyUpdateAndPrune
