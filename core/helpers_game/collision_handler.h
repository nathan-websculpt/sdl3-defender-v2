#pragma once
#include "core/config.h"
#include "core/helpers_game/game_helper.h"
#include "core/high_scores/high_scores.h"
#include "core/random/rng_service.h"
#include "entities/health_item.h"
struct GameStateData;

namespace CollisionHandler {
bool processAllCollisions(GameStateData& state, const GameHelper& helpers,
                          const HighScores& highScores, const SDL_FRect& playerBounds,
                          Random::RngEngine& fxRng);
}
