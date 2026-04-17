#include <algorithm>
#include <entities/health_item.h>
#include <entities/opponents/aggressive_opponent.h>
#include <entities/opponents/basic_opponent.h>
#include <entities/opponents/sniper_opponent.h>
#include <numbers>

#include <core/config.h>
#include <core/game.h>
#include <core/globals.h>
#include <core/helpers_game/colony_update_and_prune.h>

void Game::updatePlayerAndProjectiles(float deltaTime) {
    if (!m_state.player)
        return;

    m_state.player->update(m_state.particles, m_rng.fxEngine);

    // player projectiles
    auto& playerProjectiles = m_state.player->getProjectiles();
    ColonyUpdateAndPrune::projectiles(playerProjectiles, deltaTime, m_gameHelpers);
}

bool Game::updateOpponents(float deltaTime, const SDL_FRect& playerBounds) {
    for (auto opp_iter = m_state.opponents.begin(); opp_iter != m_state.opponents.end();) {
        auto& oppPtr = *opp_iter;
        if (!oppPtr) {
            opp_iter = m_state.opponents.erase(opp_iter);
            continue;
        }

        if (oppPtr->isAlive()) {
            SDL_FPoint playerPos = {playerBounds.x, playerBounds.y};
            oppPtr->update(deltaTime, playerPos, m_state.cameraX, m_rng.simEngine);
            ColonyUpdateAndPrune::projectiles(oppPtr->getProjectiles(), deltaTime, m_gameHelpers);
        }

        // check if opponent hit landscape
        SDL_FRect opplayerBoundsounds = oppPtr->getBounds();
        float oppCenterX = opplayerBoundsounds.x + opplayerBoundsounds.w / 2.0f;
        float groundY = m_gameHelpers.getGroundYAt(oppCenterX);
        if (opplayerBoundsounds.y + opplayerBoundsounds.h >= groundY) {
            if (oppPtr->damagesWorldOnGroundImpact()) { // only basic opponents damage world
                m_state.worldHealth--;
                if (m_state.worldHealth <= 0) {
                    // world health too low; game over
                    return false; // report game-over to top-level update
                }
            }
            // opponent touched ground - explode
            oppPtr->explode(m_state.particles, m_rng.fxEngine);
            opp_iter = m_state.opponents.erase(opp_iter);

            continue;
        }

        if (!oppPtr->isAlive()) {
            opp_iter = m_state.opponents.erase(opp_iter);
            continue;
        }

        ++opp_iter;
    }
    return true;
}

void Game::handleSpawnsAndTimers(SecondsF deltaTime) {
    m_playerHealthItemSpawnTimer += deltaTime;
    while (m_playerHealthItemSpawnTimer >= PLAYER_HEALTH_ITEM_SPAWN_INTERVAL) {
        spawnHealthItem(HealthItemType::PLAYER);
        m_playerHealthItemSpawnTimer -= PLAYER_HEALTH_ITEM_SPAWN_INTERVAL;
    }

    m_worldHealthItemSpawnTimer += deltaTime;
    while (m_worldHealthItemSpawnTimer >= WORLD_HEALTH_ITEM_SPAWN_INTERVAL) {
        spawnHealthItem(HealthItemType::WORLD);
        m_worldHealthItemSpawnTimer -= WORLD_HEALTH_ITEM_SPAWN_INTERVAL;
    }

    m_opponentSpawnTimer += deltaTime;
    while (m_opponentSpawnTimer >= OPPONENT_SPAWN_INTERVAL) {
        spawnOpponent();
        m_opponentSpawnTimer -= OPPONENT_SPAWN_INTERVAL;
    }
}

void Game::spawnOpponent() {
    const int type = Random::randomIntInclusive(m_rng.simEngine, 0, 2);
    const int maxSpawnX = std::max(0, Config::Game::WORLD_WIDTH - 51);
    const float x = static_cast<float>(Random::randomIntInclusive(m_rng.simEngine, 0, maxSpawnX));
    float y = -50.0f;
    switch (type) {
    case 0:
        m_state.opponents.emplace(std::make_unique<BasicOpponent>(x, y, 40.0f, 40.0f));
        break;
    case 1:
        m_state.opponents.emplace(std::make_unique<AggressiveOpponent>(x, y, 45.0f, 45.0f));
        break;
    case 2: {
        const float offset =
            Random::randomFloatRange(m_rng.simEngine, 0.0f, 2.0f * std::numbers::pi_v<float>);
        m_state.opponents.emplace(std::make_unique<SniperOpponent>(x, y, 35.0f, 35.0f, offset));
        break;
    }
    }
}

void Game::spawnHealthItem(HealthItemType type) {
    const int maxSpawnX = std::max(0, Config::Game::WORLD_WIDTH - 51);
    const float x = static_cast<float>(Random::randomIntInclusive(m_rng.simEngine, 0, maxSpawnX));
    float y = -50.0f; // start from top
    float w = 30.0f;
    float h = 30.0f;
    const bool doesStop = Random::randomChance(m_rng.simEngine, 0.25);
    const int randomStopY = doesStop ? Random::randomIntInclusive(m_rng.simEngine, 200, 300) : 0;
    const std::string& textureKey = (type == HealthItemType::PLAYER)
                                        ? Config::Textures::PLAYER_HEALTH_ITEM
                                        : Config::Textures::WORLD_HEALTH_ITEM;
    m_state.healthItems.emplace(
        std::make_unique<HealthItem>(x, y, w, h, type, textureKey, doesStop, randomStopY));
}
